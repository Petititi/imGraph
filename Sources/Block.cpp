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
#include "Graph.h"
#include "blocks/ParamValidator.h"
#include "ProcessManager.h"

using namespace charliesoft;
using std::string;
using std::map;
using std::vector;
using boost::property_tree::ptree;
using boost::lexical_cast;
using namespace boost::posix_time;

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
    case Color:
      return "Color";
    case Matrix:
      return "Mat";
    case String:
      return "String";
    case FilePath:
      return "FilePath";
    case AnyType:
      return "AnyType";
    default:
      return "typeError";
    }
  }

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


  AlgoPerformance::AlgoPerformance()
  {
    nbMeasures = 0;
  }
  int AlgoPerformance::getMeanPerf() const
  {
    if (nbMeasures == 0)
      return 0;
    return (totalTime / nbMeasures).total_milliseconds();
  };
  void AlgoPerformance::addNewPerf(time_duration newTime)
  {
    totalTime += newTime;
    nbMeasures++;
  };

  Block::~Block()
  {
    _processes->extractProcess(this);
    wakeUpFromPause();
    wakeUpFromConsumers();
    _wait_processed.notify_all();
  }

  Block::Block(std::string name, bool isOneShot, BlockType typeExec){
    _isOneShot = isOneShot;
    _state = stopped;
    _threadID = boost::thread::id();
    _exec_type = typeExec;
    _name = name;
    _timestamp = 0;
    nbRendering = 0;
    _executeOnlyOnce = false;
  };

  void Block::operator()()
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    if (_threadID == boost::thread::id())//not a thread...
      _threadID = boost::this_thread::get_id();
    else
      return;//this thread is already running! This should not be possible...
    nbRendering = 0;
    try
    {
      bool isInit = false;
      while (true)//this will stop when user stop the process...
      {
        while (_processes->isPause())
          _cond_pause.wait(lock);//wait for play

        _processes->shouldWaitAncestors(this);//ask to scheduler if we have to wait...
        _state = consumingParams;
        bool shouldRun = true;
        for (ConditionOfRendering& condition : _conditions)
        {
          if (!condition.canRender(this))
            shouldRun= false;
        }
        //now we can run the process:
        if (!isInit)
          init();
        isInit = true;
        if (shouldRun)
        {
          do
          {
            _time_start = microsec_clock::local_time();
            run(_executeOnlyOnce);
            if (_isOneShot)
              paramsFullyProcessed();
            newProducedData();//tell to scheduler we produced some datas...
          } while (_state == consumingParams && !_executeOnlyOnce);
        }

        nbRendering++;

        boost::this_thread::sleep(boost::posix_time::milliseconds(10.));
        if (_executeOnlyOnce)
          break;
      }
    }
    catch (boost::thread_interrupted const&)
    {
      //end of thread (requested by interrupt())!
    }
    release();
    _state = stopped;
    _threadID = boost::thread::id();//reset thread ID!
  }
  
  void Block::update()
  {
    //first of all, ask ancestors to render themselves!
    _processes->updateAncestors(this);
    if (_threadID == boost::thread::id())//thread not running
    {
      //in this case, we have to run him by hand!
      bool wasExecOnce = _executeOnlyOnce;
      _executeOnlyOnce = true;
      (*this)();//run
      _executeOnlyOnce = wasExecOnce;
    }
    else
    {
      boost::unique_lock<boost::mutex> lock(_mtx_timestamp_inc);
      //there is a running thread! Just have to wake it up, and wait for rendering:
      _processes->clearWaitingList(this);
      _wait_consume.notify_all();//now we are awake!
      //and we should be processing the value...
      _wait_processed.wait(lock);//wait the update!
    }
  }

  void Block::paramsFullyProcessed()
  {
    boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
    //each params is marked as not new...
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      if (it->second.isLinked())
        it->second.markAsUsed();
    }
    _state = consumedParams;
  }

  void Block::newProducedData()
  {
    ptime time_end(microsec_clock::local_time());
    time_duration duration(time_end - _time_start);
    _perfCounter.addNewPerf(duration);

    {
      boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
      _processes->blockProduced(this, _state == consumedParams);//tell to scheduler we produced some datas...
    }
    if (_exec_type == asynchrone) return;//no need to wait
    //we have to wait entire chain of rendering to process our data:
    _processes->shouldWaitConsumers(this);//ask to scheduler if we have to wait...

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

  std::vector<cv::String> Block::getSubParams(std::string paramName)
  {
    vector<cv::String> out;
    const std::vector<ParamDefinition>& subParams = _PROCESS_MANAGER->getAlgo_SubParams(_name);
    //test if param exist
    for (auto val : subParams)
    {
      auto pos = val._name.find(paramName);
      if (pos != string::npos && paramName.length() > pos + 1)
        out.push_back(val._helper);
    }
    return out;
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

  void Block::wakeUpFromPause()
  {
    _cond_pause.notify_all();
  }

  void Block::wakeUpFromConsumers()
  {
    _wait_consume.notify_all();//wake up waiting thread (if needed)
  }

  void Block::notifyProduction()
  {
    _wait_processed.notify_all();
  }

  bool Block::isReadyToRun(bool realCheck)
  {
    _error_msg = "";
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      try
      {
        if (it->second.isLinked())
        {
          ParamValue* other = it->second.get<ParamValue*>();
          if (realCheck)
          {
            it->second.validate(*other);
          }
          else
          {
            if (!other->getBlock()->isReadyToRun())
              throw ErrorValidator(other->getBlock()->_error_msg);
          }
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

  bool Block::hasNewParameters()
  {
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      if (it->second.isNew())
        return true;
    }
    return false;
  }

  void Block::waitConsumers(boost::unique_lock<boost::mutex>& lock)
  {
    _wait_consume.wait(lock);
  }

  void Block::waitProducers(boost::unique_lock<boost::mutex>& lock)
  {
    _wait_processed.wait(lock);
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
    for (const auto& it : inParam)
    {
      ParamValue& t = _myInputs[it._name] = ParamValue(this, &it, false);
      t.isNeeded(it._show);
      t = it._initVal;
    }
    for (const auto& it : outParam)
    {
      ParamValue& t = _myOutputs[it._name] = ParamValue(this, &it, true);
      t.isNeeded(it._show);
      t = it._initVal;
    }
    const std::vector<ParamDefinition>& subParams = _PROCESS_MANAGER->getAlgo_SubParams(_name);
    //test if param is an algo:
    for (const auto& val : subParams)
    {
      ParamValue& t = _mySubParams[val._name] = ParamValue(this, &val, false);
      t = val._initVal;
      t.isNeeded(false);
    }
  }

  void Block::setParamValue(std::string nameParam_, ParamValue* value){
    if (_myInputs.find(nameParam_) != _myInputs.end())
      _myInputs[nameParam_] = value;
  };
  ParamValue* Block::getParam(std::string nameParam_, bool input){
    if (input)
    {
      if (_myInputs.find(nameParam_) != _myInputs.end())
        return &_myInputs[nameParam_];
      //maybe a subparam?
      if (_mySubParams.find(nameParam_) != _mySubParams.end())
        return &_mySubParams[nameParam_];
    }
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

  void Block::initFromXML(boost::property_tree::ptree* block,
    std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
    std::map<unsigned int, ParamValue*>& addressesMap,
    std::vector<ConditionOfRendering*>& condToUpdate)
  {
    string pos = block->get("position", "[0,0]");
    int posSepare = pos.find_first_of(',') + 1;
    string xPos = pos.substr(1, posSepare - 2);
    string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    setPosition(lexical_cast<float>(xPos), lexical_cast<float>(yPos));
    for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
    {
      if (it1->first.compare("Input") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        bool link = it1->second.get("Link", false);
        string val = it1->second.get("Value", "Not initialized...");
        ParamValue* tmpVal = getParam(nameIn, true);
        string valID = it1->second.get("ID", "0");
        addressesMap[lexical_cast<unsigned int>(valID)] = tmpVal;

        if (!link)
        {
          try
          {
            if (tmpVal != NULL)
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
        ParamValue* tmpVal = getParam(nameOut, false);
        addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
      }
      if (it1->first.compare("Condition") == 0)
      {
        int cLeft = it1->second.get("category_left", 0);
        int cRight = it1->second.get("category_right", 0);
        int cOperator = it1->second.get("boolean_operator", 0);

        double valLeft = it1->second.get("Value_left", 0.);
        double valRight = it1->second.get("Value_right", 0.);
        addCondition(ConditionOfRendering(cLeft, valLeft, cRight, valRight, cOperator,
          this));
        if (cLeft == 1 || cRight == 1)//output of block...
          condToUpdate.push_back(&_conditions.back());
      }
      if (it1->first.compare("SubParam") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        bool link = it1->second.get("Link", false);
        string val = it1->second.get("Value", "Not initialized...");
        ParamValue* tmpVal = getParam(nameIn, true);
        string valID = it1->second.get("ID", "0");
        addressesMap[lexical_cast<unsigned int>(valID)] = tmpVal;

        if (!link)
        {
          try
          {
            if (tmpVal != NULL)
              tmpVal->valid_and_set(tmpVal->fromString(tmpVal->getType(), val));
          }
          catch (...)
          {
          }
        }
        else
          toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
      }
    }
  }

  boost::property_tree::ptree Block::getXML()
  {
    ptree tree;
    tree.put("name", _name);
    tree.put("position", _position);
    vector<string> inputWithSubparams;

    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("ID", (unsigned int)&it->second);
      paramTree.put("Link", it->second.isLinked());
      if (!it->second.isLinked())
        paramTree.put("Value", it->second.toString());
      else
        paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

      tree.add_child("Input", paramTree);

      if (it->second.getType()==ListBox)
      {
        inputWithSubparams.push_back(it->second.getName() + "." + it->second.getValFromList());
      }
    }

    for (auto it = _myOutputs.begin();
      it != _myOutputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("ID", (unsigned int)&it->second);

      tree.add_child("Output", paramTree);
    }

    for (auto it = _mySubParams.begin();
      it != _mySubParams.end(); it++)
    {
      bool shouldAdd = false;
      string paramName = it->first;
      for (auto it1 = inputWithSubparams.begin();
        it1 != inputWithSubparams.end() && !shouldAdd; it1++)
      {
        if (paramName.find(*it1) != string::npos)
          shouldAdd = true;
      }
      if (shouldAdd)
      {
        ptree paramTree;
        paramTree.put("Name", it->first);
        paramTree.put("ID", (unsigned int)&it->second);
        paramTree.put("Link", it->second.isLinked());
        if (!it->second.isLinked())
          paramTree.put("Value", it->second.toString());
        else
          paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

        tree.add_child("SubParam", paramTree);
      }
    }

    for (auto it = _conditions.begin();
      it != _conditions.end(); it++)
      tree.add_child("Condition", it->getXML());

    return tree;
  };

  void Block::linkParam(std::string paramName, Block* dest, std::string paramNameDest)
  {
    ParamValue& valIn = ParamValue();
    if (dest->_myInputs.find(paramNameDest) == dest->_myInputs.end())
      valIn = dest->_mySubParams[paramNameDest];
    else
      valIn = dest->_myInputs[paramNameDest];

    ParamValue& valOut = _myOutputs[paramName];
    //first test type of input:
    if (valIn.getType() != AnyType && valIn.getType() != valOut.getType())
    {
      throw (ErrorValidator((my_format(_STR("ERROR_TYPE")) %
        _STR(getName()) % _STR(valOut.getName()) % typeName(valOut.getType()) %
        _STR(dest->getName()) % _STR(valIn.getName()) % typeName(valIn.getType())).str()));
    }
    dest->setParamValue(paramNameDest, &_myOutputs[paramName]);
  }

  void Block::setPosition(int x, int y)
  {
    _position.x = x;
    _position.y = y;
  }

  void Block::removeCondition()
  {
    _conditions.pop_back();
  }

  vector<ParamDefinition> Block::getInParams() const
  {
    return ProcessManager::getInstance()->getAlgo_InParams(_name);
  };
  vector<ParamDefinition> Block::getOutParams() const
  {
    return ProcessManager::getInstance()->getAlgo_OutParams(_name);
  };
}