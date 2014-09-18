#include "Block.h"
#include <vector>
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/thread/thread.hpp>
#include <boost/parameter.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/lock_guard.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif
#include "ParamValidator.h"

using namespace lsis_org;
using std::string;
using std::map;
using std::vector;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  std::string typeName(ParamType type)
  {
    switch (type)
    {
    case Boolean:
      return "Boolean";
    case Int:
      return "Int";
    case Float:
      return "Float";
    case Vector:
      return "Vector";
    case Mat:
      return "Mat";
    case String:
      return "String";
    case FilePath:
      return "FilePath";
    default:
      return "typeError";
    }
  }

  unsigned int GraphOfProcess::_current_timestamp = 0;
  bool GraphOfProcess::pauseProcess = false;
  boost::mutex _mtx_synchro;

  ConditionOfRendering::ConditionOfRendering()
  {
    category_left = category_right = boolean_operator = 0;
    _father = 0;
  }
  ConditionOfRendering::ConditionOfRendering(unsigned char category_l,
    ParamValue opt_value_l,
    unsigned char category_r, ParamValue opt_value_r,
    unsigned char boolean_op, Block* father) :
    category_left(category_l), opt_value_left(opt_value_l), category_right(category_r),
    opt_value_right(opt_value_r), boolean_operator(boolean_op), _father(father)
  {
    opt_value_left.setBlock(father);
    opt_value_right.setBlock(father);
  }

  boost::property_tree::ptree ConditionOfRendering::getXML() const
  {
    ptree tree;
    tree.put("category_left", category_left);
    tree.put("category_right", category_right);
    tree.put("boolean_operator", boolean_operator);

    if (!opt_value_left.isLinked())
      tree.put("Value_left", opt_value_left.toString());
    else
      tree.put("Value_left", (unsigned int)opt_value_left.get<ParamValue*>());
    if (!opt_value_right.isLinked())
      tree.put("Value_right", opt_value_right.toString());
    else
      tree.put("Value_right", (unsigned int)opt_value_right.get<ParamValue*>());
    return tree;
  }


  std::string ConditionOfRendering::toString()
  {
    if (category_left > 3 || category_right > 3 || boolean_operator > 5 || category_left == 0 || category_right == 0)
      return "";
    string left, right;
    switch (category_left)
    {
    case 1://Output of block
      left = "XXX";
      break;
    case 3://cardinal of block rendering
      left = _STR("CONDITION_CARDINAL");
      break;
    default://nothing
      left = opt_value_left.toString();
      break;
    }
    switch (category_right)
    {
    case 1://Output of block
      right = "XXX";
      break;
    case 3://Is empty
      right = _STR("CONDITION_IS_EMPTY");
    default://nothing
      right = opt_value_right.toString();
      break;
    }

    switch (boolean_operator)
    {
    case 0://==
      return left + " == " + right;
    case 1://!=
      return left + " != " + right;
    case 2://<
      return left + " < " + right;
    case 3://>
      return left + " > " + right;
    case 4://<=
      return left + " <= " + right;
    case 5://>=
      return left + " >= " + right;
    default://nothing
      break;
    }

    return "";
  }

  bool ConditionOfRendering::canRender(Block* blockTested)
  {
    if (category_left > 3 || category_right > 3 || boolean_operator>5 || category_left == 0 || category_right == 0)
      return true;
    ParamValue left, right;
    switch (category_left)
    {
    case 1://Output of block
    {
      ParamValue* tmp = opt_value_left.get<ParamValue*>();
      if (tmp != NULL)
        left = *tmp;
      break;
    }
    case 2://Constante value
      left = opt_value_left;
      break;
    case 3://cardinal of block rendering
      left = static_cast<double>(blockTested->getNbRendering());
      break;
    default://nothing
      break;
  }
    switch (category_right)
    {
    case 1://Output of block
    {
      ParamValue* tmp = opt_value_right.get<ParamValue*>();
      if (tmp != NULL)
        right = *tmp;
      break;
    }
    case 2://Constante value
      right = opt_value_right;
      break;
    case 3://Is empty
      if (boolean_operator == 0)
        return left.isDefaultValue();
      else
        return !left.isDefaultValue();
    default://nothing
      break;
    }

    switch (boolean_operator)
    {
    case 0://==
      return left == right;
    case 1://!=
      return left != right;
    case 2://<
      return left < right;
    case 3://>
      return left > right;
    case 4://<=
      return left <= right;
    case 5://>=
      return left >= right;
    default://nothing
      return true;
    }

    return true;
  }

  void ConditionOfRendering::setValue(bool left, ParamValue* param2)
  {
    if (left)
    {
      switch (category_left)
      {
      case 1://Output of block
      {
        opt_value_left = param2;
        break;
      }
      default://nothing
        opt_value_left = *param2;
      }
    }
    else
    {
      switch (category_right)
      {
      case 1://Output of block
      {
        opt_value_right = param2;
        break;
      }
      default://nothing
        opt_value_right = *param2;
      }
    }
  }

  Block::Block(std::string name){
    _name = name;
    _timestamp = 0;
    _fullyRendered = true;
    nbRendering = 0;
  };

  void Block::operator()()
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    nbRendering = 0;
    try
    {
      while (true)//this will stop when user stop the process...
      {
        while (GraphOfProcess::pauseProcess)
          _cond_pause.wait(lock);//wait for any parameter update...

        if (_timestamp < _processes->_current_timestamp)
        {
          //are parameters ok?
          bool parametersOK = false;
          while (!parametersOK)
          {
            parametersOK = true;
            for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
            {
              while (it->second.isLinked() &&
                it->second.getTimestamp() < _processes->_current_timestamp)
              {
                ParamValue* t = it->second.get<ParamValue*>();
                parametersOK = false;

                string debug = "   " + _STR(getName()) + " blocked (" + _STR(it->second.getName()) + ")\n";
                //std::cout << debug;

                //not OK! we must wait here until producer update value!
                _cond_sync.wait(lock);//wait for any parameter update...
              }
            }
          }
          bool shouldRun = true;
          for (ConditionOfRendering& condition : _conditions)
          {
            if (!condition.canRender(this))
              shouldRun= false;
          }
          //now we can run the process:
          if (shouldRun)
            run();
          else
            skipRendering();
          nbRendering++;
          _fullyRendered = true;
          _cond_sync.notify_all();//wake up waiting thread (if needed)
        }
        _cond_sync.wait(lock);//wait for any parameter update...
      }
    }
    catch (boost::thread_interrupted const&)
    {
      //end of thread (requested by interrupt())!
    }
  }


  bool Block::isAncestor(Block* other)
  {
    //add empty parameters:
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
      {
        ParamValue* otherParam = it->second.get<ParamValue*>();
        if (otherParam->getBlock() == other)
          return true;
        if (otherParam->getBlock()->isAncestor(other))
          return true;
      }
      it++;
    }
    return false;
  }

  bool Block::isStartingBlock()
  {
    if (ProcessManager::getInstance()->getAlgoType(_name) != input)
      return false;
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
        return false;
      it++;
    }
    return true;
  }

  void Block::renderingDone(bool fullyRendered)
  {
    _fullyRendered = fullyRendered;
    {
      boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
      _timestamp = _processes->_current_timestamp;

      //wake other blocks:
      for (auto output_val = _myOutputs.begin();
        output_val != _myOutputs.end(); output_val++)//should use iterator to not create temp ParamValue
      {
        std::set<ParamValue*>& listeners = output_val->second.getListeners();
        for (auto listener : listeners)
        {
          if (listener != NULL)
            listener->getBlock()->wakeUp();
        }
      }

      while (GraphOfProcess::pauseProcess)
        _cond_pause.wait(guard);//wait for any parameter update...
    }
    //test if we need to wait!
    _processes->synchronizeTimestamp(this);
    if (!fullyRendered)
    {
      _processes->_current_timestamp++;//next frame
    }
  }

  void Block::wakeUp()
  {
    _cond_sync.notify_all();//wake up waiting thread (if needed)
    _cond_pause.notify_all();
  }
  void Block::waitUpdate(boost::unique_lock<boost::mutex>& lock)
  {
    _cond_sync.wait(lock);
  }

  void Block::skipRendering()
  {
    {
      boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
      _timestamp = _processes->_current_timestamp;
    }
    //go through outputs and skip it:
    auto it = _myOutputs.begin();
    while (it != _myOutputs.end())
    {
      std::set<ParamValue*>& listeners = it->second.getListeners();
      for (auto listener : listeners)
      {
        if (listener != NULL)
          listener->getBlock()->skipRendering();
      }
      it++;
    }
    _cond_sync.notify_all();//wake up waiting thread (if needed)
  }

  bool Block::isReadyToRun()
  {
    _error_msg = "";
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      try
      {
        if (it->second.isLinked())
        {
          ParamValue* other = it->second.get<ParamValue*>();
          if (!other->getBlock()->isReadyToRun())
            throw ErrorValidator(other->getBlock()->_error_msg);
        }
        else
          it->second.validate(it->second);
      }
      catch (ErrorValidator& e)
      {
        _error_msg += e.errorMsg + "<br/>";
      }
    }
    return _error_msg.empty();
  }

  bool Block::validTimestampOrWait(Block* other)
  {
    bool waiting = false;
    boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
    while (!_fullyRendered || other->_timestamp > _timestamp)
    {
      std::string debug = "  " + _STR(other->getName()) + " blocked at " + _STR(getName()) + " : " + lexical_cast<string>(_timestamp) + "\n";
      //std::cout << debug;
      waiting = true;
      waitUpdate(guard);
      debug = "   " + _STR(other->getName()) + " unblocked (" + _STR(getName()) + ")\n";
      //std::cout << debug;
    }
    return !waiting;
  }

  bool Block::validateParams(std::string param, const ParamValue val){
    try
    {
      _myInputs[param].validate(val);
    }
    catch (ErrorValidator& e)
    {
      _error_msg += e.errorMsg + "<br/>";
      return false;
    }
    return true;
  }

  std::string Block::getErrorMsg() {
    std::string tmp = _error_msg;
    _error_msg = "";
    return tmp;
  }

  void Block::initParameters(const std::vector<ParamDefinition>& inParam,
    const std::vector<ParamDefinition>& outParam)
  {
    //add empty parameters:
    auto it = inParam.begin();
    while (it != inParam.end())
    {
      _myInputs[it->_name] = ParamValue(this, it->_name, false);
      it++;
    }
    it = outParam.begin();
    while (it != outParam.end())
    {
      _myOutputs[it->_name] = ParamValue(this, it->_name, true);
      it++;
    }
  }

  void Block::setParamLink(std::string nameParam_, ParamValue* value){
    if (_myInputs.find(nameParam_) != _myInputs.end())
      _myInputs[nameParam_] = value;
  };
  ParamValue* Block::getParam(std::string nameParam_, bool input){
    if (input && _myInputs.find(nameParam_) != _myInputs.end())
      return &_myInputs[nameParam_];
    else
      if (!input && _myOutputs.find(nameParam_) != _myOutputs.end())
        return &_myOutputs[nameParam_];
      
    return NULL;
  };

  std::vector<BlockLink> Block::getInEdges()
  {
    vector<BlockLink> out;
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
        out.push_back(BlockLink(it->second.toBlockLink()));
      it++;
    }
    return out;
  }

  boost::property_tree::ptree Block::getXML() const
  {
    ptree tree;
    tree.put("name", _name);
    tree.put("position", _position);

    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("Link", it->second.isLinked());
      if (!it->second.isLinked())
        paramTree.put("Value", it->second.toString());
      else
        paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

      tree.add_child("Input", paramTree);
    }

    for (auto it = _myOutputs.begin();
      it != _myOutputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("ID", (unsigned int)&it->second);

      tree.add_child("Output", paramTree);
    }

    for (auto it = _conditions.begin();
      it != _conditions.end(); it++)
      tree.add_child("Condition", it->getXML());

    return tree;
  };

  void Block::createLink(std::string paramName, Block* dest, std::string paramNameDest)
  {
    ParamValue& valIn = dest->_myInputs[paramNameDest];
    ParamValue& valOut = _myOutputs[paramName];
    //first test type of input:
    if (valIn.getType() != valOut.getType())
    {
      throw (ErrorValidator((my_format(_STR("ERROR_TYPE")) %
        _STR(getName()) % _STR(valOut.getName()) % typeName(valOut.getType()) %
        _STR(dest->getName()) % _STR(valIn.getName()) % typeName(valIn.getType())).str()));
    }
    dest->setParamLink(paramNameDest, &_myOutputs[paramName]);
  }

  void Block::setPosition(int x, int y)
  {
    _position.x = x;
    _position.y = y;
  }

  GraphOfProcess::GraphOfProcess(){
  };

  void GraphOfProcess::addNewProcess(Block* block){
    _vertices.push_back(block);
    block->setGraph(this);
  };

  void GraphOfProcess::deleteProcess(Block* process){
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      if (*it == process)
      {
        _vertices.erase(it);
        delete process;
        return;
      }
    }
  };

  void GraphOfProcess::synchronizeTimestamp(Block* processToSynchronize)
  {
    for (auto it : _vertices)
    {
      //each block should have the same timestamp:
      if (it != processToSynchronize && it->isAncestor(processToSynchronize))
      {
        while (!it->validTimestampOrWait(processToSynchronize));
      }
    }
  }
  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return _vertices;
  }

  void GraphOfProcess::stop()
  {
    for (size_t i = 0; i < _runningThread.size(); i++)
      _runningThread[i].interrupt();
    _runningThread.clear();
  }
  
  bool GraphOfProcess::run()
  {
    stop();//just in case...
    _current_timestamp++;
    bool res = true;
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
      _runningThread.push_back(boost::thread(boost::ref(**it)));

    return res;
  }
  void GraphOfProcess::switchPause()
  {
    GraphOfProcess::pauseProcess = !GraphOfProcess::pauseProcess;
    if (!pauseProcess)
    {
      //wake up threads:
      for (auto it = _vertices.begin();
        it != _vertices.end(); it++)
        (*it)->wakeUp();
    }
  }

  void GraphOfProcess::saveGraph(boost::property_tree::ptree& tree) const
  {
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      tree.add_child("GraphOfProcess.Block", (*it)->getXML());
    }
  }

  void GraphOfProcess::fromGraph(boost::property_tree::ptree& tree)
  {
    boost::optional<ptree&> vertices = tree.get_child_optional("GraphOfProcess");

    map<unsigned int, ParamValue*> addressesMap;
    vector < std::pair<ParamValue*, unsigned int> > toUpdate;
    vector<ConditionOfRendering*> condToUpdate;

    if (vertices)
    {
      for (ptree::iterator it = vertices->begin(); it != vertices->end(); it++)
      {
        if (it->first.compare("Block") == 0)
        {
          ptree *block = &it->second;
          string name = block->get("name", "Error");
          string pos = block->get("position", "[0,0]");
          int posSepare = pos.find_first_of(',') + 1;
          string xPos = pos.substr(1, posSepare - 2);
          string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);
          Block* tmp = ProcessManager::getInstance()->createAlgoInstance(name);
          if (tmp != NULL)
          {
            addNewProcess(tmp);
            tmp->setPosition(lexical_cast<float>(xPos), lexical_cast<float>(yPos));
            for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
            {
              if (it1->first.compare("Input") == 0)
              {
                string nameIn = it1->second.get("Name", "Error");
                bool link = it1->second.get("Link", false);
                string val = it1->second.get("Value", "Not initialized...");
                ParamValue* tmpVal = tmp->getParam(nameIn, true);
                if (!link)
                {
                  try
                  {
                    tmpVal->valid_and_set(tmpVal->fromString(tmpVal->getType(), val));
                  }
                  catch (...)
                  {
                  }
                }
                else
                  toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
              }
              if (it1->first.compare("Output") == 0)
              {
                string nameOut = it1->second.get("Name", "Error");
                string val = it1->second.get("ID", "0");
                ParamValue* tmpVal = tmp->getParam(nameOut, false);
                addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
              }
              if (it1->first.compare("Condition") == 0)
              {
                int cLeft = it1->second.get("category_left", 0);
                int cRight = it1->second.get("category_right", 0);
                int cOperator = it1->second.get("boolean_operator", 0);

                double valLeft = it1->second.get("Value_left", 0.);
                double valRight = it1->second.get("Value_right", 0.);
                tmp->addCondition(ConditionOfRendering(cLeft, valLeft, cRight, valRight, cOperator, 
                  tmp));
                if (cLeft==1||cRight==1)//output of block...
                  condToUpdate.push_back(&tmp->getConditions().back());
              }
            }
          }
        }
      }
    }
    //now make links:
    for (auto valToUpdate : toUpdate)
    {
      Block* fromBlock = valToUpdate.first!=NULL?valToUpdate.first->getBlock():NULL;
      ParamValue* secondVal = addressesMap[valToUpdate.second];
      Block* toBlock = secondVal != NULL ? secondVal->getBlock() : NULL;
      if (fromBlock != NULL && toBlock != NULL)
      {
        try
        {
          toBlock->createLink(secondVal->getName(), fromBlock, valToUpdate.first->getName());
        }
        catch (ErrorValidator& e)
        {//algo doesn't accept this value!
          //QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
          std::cout << e.errorMsg << std::endl;
        }
      }
    }
    //and set correct values of conditions:
    for (auto cond : condToUpdate)
    {
      if (cond->getCategory_left() == 1)//output of block:
      {
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_left().get<double>() + 0.5);
        cond->setValue(true, addressesMap[addr]);
      }
      if (cond->getCategory_right() == 1)//output of block:
      {
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_right().get<double>() + 0.5);
        cond->setValue(false, addressesMap[addr]);
      }
    }
  }
}