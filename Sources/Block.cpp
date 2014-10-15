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
    case Color:
      return "Color";
    case Matrix:
      return "Mat";
    case String:
      return "String";
    case FilePath:
      return "FilePath";
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
        _renderingSkiped = false;
        while (GraphOfProcess::pauseProcess)
          _cond_pause.wait(lock);//wait for any parameter update...

        if (_timestamp < _processes->_current_timestamp)
        {
          //are parameters ok?
          for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
          {
            if (it->second.isLinked())
              validTimestampOrWait(it->second.get<ParamValue*>()->getBlock(), _processes->_current_timestamp);
          }

          bool shouldRun = true;
          for (ConditionOfRendering& condition : _conditions)
          {
            if (!condition.canRender(this))
              shouldRun= false;
          }
          //now we can run the process:
          if (shouldRun && !_renderingSkiped)
            run();
          else
            skipRendering();
          //std::string debug = _STR(getName()) + " (" + lexical_cast<string>(_timestamp)+") rendering done\n";
          //std::cout << debug;

          nbRendering++;
          _fullyRendered = true;

          _cond_sync.notify_all();//wake up waiting thread (if needed)
        }
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

  void Block::skipRendering()
  {
    {
      boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
      _timestamp = _processes->_current_timestamp;
      _renderingSkiped = true;
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
      //std::string debug = "  " + _STR(other->getName()) + " (" + lexical_cast<string>(other->_timestamp) + ") blocked at " + _STR(getName()) + " : " + lexical_cast<string>(_timestamp)+"\n";
      //std::cout << debug;
      waiting = true;
      _cond_sync.wait(guard);
      //debug = "   " + _STR(other->getName()) + " unblocked (" + _STR(getName()) + ")\n";
      //std::cout << debug;
    }
    return !waiting;
  }

  bool Block::validTimestampOrWait(Block* other, unsigned int timeGoal)
  {
    bool waiting = false;
    boost::unique_lock<boost::mutex> guard(other->_mtx_timestamp_inc);
    while (other->_timestamp<timeGoal)
    {
      //std::string debug = "  - " + _STR(getName()) + " : " + lexical_cast<string>(_timestamp)+" wait for " + _STR(other->getName()) + " timestamp (" + lexical_cast<string>(timeGoal)+")\n";
      //std::cout << debug;
      waiting = true;
      _cond_sync.wait(guard);
      //debug = "  + " + _STR(getName()) + " unblocked\n";
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
      _myInputs[it->_name].isNeeded(it->_show);
      it++;
    }
    it = outParam.begin();
    while (it != outParam.end())
    {
      _myOutputs[it->_name] = ParamValue(this, it->_name, true);
      _myOutputs[it->_name].isNeeded(it->_show);
      it++;
    }
  }

  void Block::setParamLink(std::string nameParam_, ParamValue* value){
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
    ParamValue& valIn = ParamValue();
    if (dest->_myInputs.find(paramNameDest) == dest->_myInputs.end())
      valIn = dest->_mySubParams[paramNameDest];
    else
      valIn = dest->_myInputs[paramNameDest];

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

  void Block::removeCondition()
  {
    _conditions.pop_back();
  }
}