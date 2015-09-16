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
    ParamValue opt__valuel,
    unsigned char category_r, ParamValue opt__valuer,
    unsigned char boolean_op, Block* father) :
    category_left(category_l), opt__valueleft(opt__valuel), category_right(category_r),
    opt__valueright(opt__valuer), boolean_operator(boolean_op), _father(father)
  {
    opt__valueleft.setBlock(father);
    opt__valueright.setBlock(father);
  }

  boost::property_tree::ptree ConditionOfRendering::getXML() const
  {
    ptree tree;
    tree.put("category_left", category_left);
    tree.put("category_right", category_right);
    tree.put("boolean_operator", boolean_operator);

    if (!opt__valueleft.isLinked())
      tree.put("_valueleft", opt__valueleft.toString());
    else
      tree.put("_valueleft", (unsigned int)opt__valueleft.get<ParamValue*>());
    if (!opt__valueright.isLinked())
      tree.put("_valueright", opt__valueright.toString());
    else
      tree.put("_valueright", (unsigned int)opt__valueright.get<ParamValue*>());
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
      left = opt__valueleft.toString();
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
      right = opt__valueright.toString();
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
      ParamValue* tmp = opt__valueleft.get<ParamValue*>();
      if (tmp != NULL)
        left = *tmp;
      break;
    }
    case 2://Constante value
      left = opt__valueleft;
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
      ParamValue* tmp = opt__valueright.get<ParamValue*>();
      if (tmp != NULL)
        right = *tmp;
      break;
    }
    case 2://Constante value
      right = opt__valueright;
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
        opt__valueleft = param2;
        break;
      }
      default://nothing
        opt__valueleft = *param2;
      }
    }
    else
    {
      switch (category_right)
      {
      case 1://Output of block
      {
        opt__valueright = param2;
        break;
      }
      default://nothing
        opt__valueright = *param2;
      }
    }
  }


  AlgoPerformance::AlgoPerformance()
  {
    nbMeasures = 0;
    _maxTime = boost::posix_time::min_date_time;
    _minTime = boost::posix_time::max_date_time;
  }
  int AlgoPerformance::getMeanPerf() const
  {
    if (nbMeasures == 0)
      return 0;
    return static_cast<int>((totalTime / nbMeasures).total_milliseconds());
  };
  void AlgoPerformance::addNewPerf(time_duration newTime)
  {
    if (newTime > _maxTime)
      _maxTime = newTime;
    if (newTime < _minTime)
      _minTime = newTime;
    totalTime += newTime;
    nbMeasures++;
  };

  Block::~Block()
  {
    if (_processes != NULL)
      _processes->extractProcess(this);
    wakeUpFromPause();
    wakeUpFromConsumers();
    _wait_processed.notify_all();

    //delete definitions:

    for (ParamDefinition* def : _algorithmInParams)
      delete def;
    _algorithmInParams.clear();
    for (ParamDefinition* def : _algorithmSubParams)
      delete def;
    _algorithmSubParams.clear();
    for (ParamDefinition* def : _algorithmOutParams)
      delete def;
    _algorithmOutParams.clear();

  }

  Block::Block(std::string name, bool isOneShot, BlockType typeExec, bool addInputParam){
    _addInputParam = addInputParam;
    _currentPreview = "None";
    _isOneShot = isOneShot;
    _state = stopped;
    _threadID = boost::thread::id();
    _exec_type = typeExec;
    _name = name;
    _timestamp = 0;
    nbRendering = 0;
    _executeOnlyOnce = false;
    _processes = NULL;
  };


  void Block::operator()()
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    if (_threadID == boost::thread::id())//not a thread...
      _threadID = boost::this_thread::get_id();
    else
      return;//this thread is already running! This should not be possible...
    //get mandatories the input params:
    std::vector<ParamValue*> vals;
    for (auto& param : _myInputs)
      if (param.second.containValidator<ValNeeded>())
        vals.push_back(&param.second);

    bool errors = false;
    nbRendering = 0;
    try
    {
      bool isInit = false;
      while (true)//this will stop when user stop the process...
      {
        //for each input needed, wait for a value:
        for (auto param : vals)
        {
          if (param->isDefaultValue())
            param->waitForUpdate(lock);
        }

        while (_processes!=NULL && _processes->isPause())
        {
          BlockState oldState = _state;
          _state = paused;
          _cond_pause.wait(lock);//wait for play
          _state = oldState;
        }

        if (_processes != NULL)
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

            try
            {
              if (run(_executeOnlyOnce))
                _error_msg = "";//no errors...
              else
                paramsFullyProcessed();//something goes wrong, set parameters as processed to get new ones
            }
            catch (cv::Exception& e)
            {
              errors = true;
              _error_msg += e.what();
              std::cout << "exception caught: " << e.what() << std::endl;
              throw boost::thread_interrupted();
            }
            catch (...)
            {
              errors = true;
              throw;
            }

            if (_isOneShot )//&& !_executeOnlyOnce)
              paramsFullyProcessed();
            newProducedData();//tell to scheduler we produced some datas...
          } while (_state == consumingParams && !_executeOnlyOnce);
        }

        nbRendering++;

        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
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
    if (_processes!=NULL)
      _processes->stop(false, false, errors);
  }
  
  void Block::update()
  {
    //first of all, ask ancestors to render themselves!
    if (_processes!=NULL)
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

  void Block::markAsUnprocessed()
  {
    boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
    //each params is marked as not new...
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
      if (it->second.isLinked())
        it->second.setNew(false);
    for (auto it = _myOutputs.begin(); it != _myOutputs.end(); it++)
      it->second.setNew(false);

    if (_state!=stopped)
      _state = waitingChild;
  }

  void Block::paramsFullyProcessed()
  {
    boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
    //each params is marked as not new...
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      if (it->second.isLinked())
        it->second.setNew(false);
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
      if (_processes != NULL)
        _processes->blockProduced(this, _state == consumedParams);//tell to scheduler we produced some datas...
    }
    if (_exec_type == asynchrone) return;//no need to wait
    //we have to wait entire chain of rendering to process our data:
    if (_processes != NULL)
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

  ParamValue* Block::addNewInput(ParamDefinition* param)
  {
    _algorithmInParams.push_back(param);

    ParamValue& t = _myInputs[param->_name] = ParamValue(this, param->_name, false);
    t.isNeeded(true);//always needed!
    t = param->_initVal;
    return &t;
  };

  ParamValue* Block::addNewOutput(ParamDefinition* param)
  {
    _algorithmOutParams.push_back(param);

    ParamValue& t = _myOutputs[param->_name] = ParamValue(this, param->_name, true);
    t.isNeeded(true);//always needed!
    t = param->_initVal;
    return &t;
  };

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
    return _error_msg;
  }

  void Block::initParameters(const std::vector<ParamDefinition>& inParam,
    const std::vector<ParamDefinition>& outParam)
  {
    //copy of param definitions:
    for (auto& it : inParam)
      _algorithmInParams.push_back(new ParamDefinition(it));
    for (auto& it : outParam)
      _algorithmOutParams.push_back(new ParamDefinition(it));

    //add empty parameters:
    for (auto& it : _algorithmInParams)
    {
      ParamValue& t = _myInputs[it->_name] = ParamValue(this, it, false);
      t.isNeeded(it->_show == toBeLinked);
      t = it->_initVal;
    }

    for (auto& it : _algorithmOutParams)
    {
      ParamValue& t = _myOutputs[it->_name] = ParamValue(this, it, true);
      t.isNeeded(it->_show == toBeLinked);
      t = it->_initVal;
    }
    std::vector<ParamDefinition> subParams = _PROCESS_MANAGER->getAlgo_SubParams(_name);
    //test if param is an algo:
    for (auto& val : subParams)
    {
      ParamDefinition* def = new ParamDefinition(val);
      _algorithmSubParams.push_back(def);
      ParamValue& t = _mySubParams[val._name] = ParamValue(this, def, false);
      t = val._initVal;
      t.isNeeded(false);
    }
  }

  void Block::setParamValue(std::string nameParam_, ParamValue* value){
    if (_myInputs.find(nameParam_) != _myInputs.end())
      _myInputs[nameParam_] = value;
    else if (_mySubParams.find(nameParam_) != _mySubParams.end())
      _mySubParams[nameParam_] = value;
  };

  void Block::setParam(std::string nameParam_, ParamValue value){
    if (_myInputs.find(nameParam_) != _myInputs.end())
      _myInputs[nameParam_] = value;
    else if (_mySubParams.find(nameParam_) != _mySubParams.end())
      _mySubParams[nameParam_] = value;
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

  bool Block::paramDefinitionExist(std::string nameOfParam, bool isInput)
  {
    vector<ParamDefinition*>& vectOfDef = _algorithmInParams;
    if (!isInput)
      vectOfDef = _algorithmOutParams;
    for (ParamDefinition* def : vectOfDef)
    {
      if (def->_name.compare(nameOfParam) == 0)
        return true;
    }
    return false;
  }

  const ParamDefinition* Block::getParamDefinition(std::string nameOfParam, bool isInput) const
  {
    const vector<ParamDefinition*>* vectOfDef = &_algorithmInParams;
    if (!isInput)
      vectOfDef = &_algorithmOutParams;
    for (const ParamDefinition* def : *vectOfDef)
    {
      if (def->_name.compare(nameOfParam) == 0)
        return def;
    }
    return NULL;
  }

  ParamDefinition* Block::getParamDefinition(std::string nameOfParam, bool isInput)
  {
    vector<ParamDefinition*>& vectOfDef = _algorithmInParams;
    if (!isInput)
      vectOfDef = _algorithmOutParams;
    for (ParamDefinition* def : vectOfDef)
    {
      if (def->_name.compare(nameOfParam) == 0)
        return def;
    }
    return NULL;
  }

  void Block::initFromXML(boost::property_tree::ptree* block,
    std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
    std::map<unsigned int, ParamValue*>& addressesMap,
    std::vector<ConditionOfRendering*>& condToUpdate)
  {
    string pos = block->get("position", "[0.0,0.0]");
    int posSepare = pos.find_first_of(',') + 1;
    string xPos = pos.substr(1, posSepare - 2);
    string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    pos = block->get("size_increment", "[0.0,0.0]");
    posSepare = pos.find_first_of(',') + 1;
    string xInc = pos.substr(1, posSepare - 2);
    string yInc = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    setPosition((int)lexical_cast<float>(xPos), (int)lexical_cast<float>(yPos),
      lexical_cast<float>(xInc), lexical_cast<float>(yInc));

    _currentPreview = block->get("preview_active", "None");

    for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
    {
      if (it1->first.compare("Input") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        bool link = it1->second.get("Link", false);
        string val = it1->second.get("Value", "Not initialized...");
        ParamValue* tmpVal = getParam(nameIn, true);
        if (tmpVal != NULL)
        {
          string valID = it1->second.get("ID", "0");
          addressesMap[lexical_cast<unsigned int>(valID)] = tmpVal;

          ParamDefinition* def = getParamDefinition(nameIn, true);
          if (def != NULL)
          {
            def->_show = ParamVisibility(it1->second.get("IsVisible", (int)def->_show));
          }

          ParamType typeOfVal = ParamType(it1->second.get("SubType", (int)tmpVal->getType()));

          if (!link)
          {
            try
            {
              if (tmpVal != NULL)
                tmpVal->valid_and_set(tmpVal->fromString(typeOfVal, val));
            }
            catch (...)
            {
              tmpVal->setNew(false);
            }
          }
          else
            toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
        }
      }
      if (it1->first.compare("Output") == 0)
      {
        string nameOut = it1->second.get("Name", "Error");
        string val = it1->second.get("ID", "0");
        ParamValue* tmpVal = getParam(nameOut, false);
        if (tmpVal != NULL)
        {
          tmpVal->setNew(false);
          tmpVal->isNeeded(true);
        }
        addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
      }

      if (it1->first.compare("Input_to_create") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        string helper = it1->second.get("Helper", nameIn);
        bool link = it1->second.get("Link", false);
        string val = it1->second.get("Value", "Not initialized...");
        ParamType paramType = static_cast<ParamType>(it1->second.get("ParamType", 0));
        ParamValue& tmpValLoaded = ParamValue::fromString(paramType, val);
        ParamVisibility show = ParamVisibility(it1->second.get("IsVisible", 1));
        ParamValue* tmpVal = addNewInput(new ParamDefinition(show, paramType, nameIn, helper, tmpValLoaded));

        if (!link)
        {
          try
          {
            if (tmpVal != NULL)
              tmpVal->valid_and_set(tmpVal->fromString(tmpVal->getType(), val));
          }
          catch (...)
          {
            tmpVal->setNew(false);
          }
        }
        else
          toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
      }
      if (it1->first.compare("Output_to_create") == 0)
      {
        string nameOut = it1->second.get("Name", "Error");
        string helper = it1->second.get("Helper", nameOut);
        ParamType paramType = static_cast<ParamType>(it1->second.get("ParamType", 0));
        ParamValue* tmpVal = addNewOutput( new ParamDefinition(true, paramType, nameOut, helper));
        tmpVal->setNew(false);
        string val = it1->second.get("ID", "0");
        addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
      }
      
      if (it1->first.compare("Condition") == 0)
      {
        int cLeft = it1->second.get("category_left", 0);
        int cRight = it1->second.get("category_right", 0);
        int cOperator = it1->second.get("boolean_operator", 0);

        double valLeft = it1->second.get("_valueleft", 0.);
        double valRight = it1->second.get("_valueright", 0.);
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
    tree.put("size_increment", _sizeIncrement);
    tree.put("preview_active", _currentPreview);
    vector<string> inputWithSubparams;

    ProcessManager* pm = ProcessManager::getInstance();
    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      //if the parameter is known by ProcessManager, it's a classic parameter
      if (pm->getParamType(_name, it->first, true) != typeError)
      {
        paramTree.put("Name", it->first);
        paramTree.put("ID", (unsigned int)&it->second);
        paramTree.put("Link", it->second.isLinked());
        if (!it->second.isLinked())
          paramTree.put("Value", it->second.toString());
        else
          paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

        if (it->second.getType() == AnyType)
          paramTree.put("SubType", it->second.getType(false));

        ParamDefinition* def = getParamDefinition(it->first, true);
        paramTree.put("IsVisible", (int)def->_show);

        tree.add_child("Input", paramTree);

        if (it->second.getType() == ListBox)
          inputWithSubparams.push_back(it->second.getName() + "." + it->second.getValFromList());
      }
      else
      {
        ParamDefinition* pDef = getParamDefinition(it->first, true);
        if (pDef != NULL)
        {
          paramTree.put("Name", pDef->_name);
          paramTree.put("Helper", pDef->_helper);
          paramTree.put("ParamType", pDef->_type);
          paramTree.put("IsVisible", (int)pDef->_show);

          paramTree.put("Link", it->second.isLinked());
          if (!it->second.isLinked())
            paramTree.put("Value", it->second.toString());
          else
            paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

          tree.add_child("Input_to_create", paramTree);
        }
      }
    }

    for (auto it = _myOutputs.begin();
      it != _myOutputs.end(); it++)
    {
      ptree paramTree;
      if (pm->getParamType(_name, it->first, false) != typeError)
      {
        paramTree.put("Name", it->first);
        paramTree.put("ID", (unsigned int)&it->second);

        tree.add_child("Output", paramTree);
      }
      else
      {
        ParamDefinition& pDef = *getParamDefinition(it->first, false);

        ptree paramTree;
        paramTree.put("Name", pDef._name);
        paramTree.put("Helper", pDef._helper);
        paramTree.put("ParamType", pDef._type);
        paramTree.put("ID", (unsigned int)&it->second);

        tree.add_child("Output_to_create", paramTree);
      }
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

  ParamDefinition Block::getDef(std::string name, bool isInput) const
  {
    const ParamDefinition* def = getParamDefinition(name, isInput);
    return *def;
  }

  void Block::linkParam(std::string paramName, Block* dest, std::string paramNameDest)
  {
    ParamValue& valIn = ParamValue();
    if (dest->_myInputs.find(paramNameDest) == dest->_myInputs.end())
      valIn = dest->_mySubParams[paramNameDest];
    else
      valIn = dest->_myInputs[paramNameDest];

    valIn.getDefinition()->_show = toBeLinked;

    ParamValue& valOut = _myOutputs[paramName];
    valOut.getDefinition()->_show = toBeLinked;

    //first test type of input:
    ParamType type1 = valIn.getType(false);
    ParamType type2 = valOut.getType(false);
    if (type1 == FilePath)
      type1 = String;
    if (type2 == FilePath)
      type2 = String;
    if (type1 != AnyType && type2 != AnyType && type1 != type2)
    {
      throw (ErrorValidator((my_format(_STR("ERROR_TYPE")) %
        _STR(getName()) % _STR(valOut.getName()) % typeName(valOut.getType()) %
        _STR(dest->getName()) % _STR(valIn.getName()) % typeName(valIn.getType())).str()));
    }
    dest->setParamValue(paramNameDest, &valOut);
  }

  void Block::setIncrSize(float incX, float incY)
  {
    _sizeIncrement.x = incX;
    _sizeIncrement.y = incY;
  }

  void Block::setPosition(int x, int y, float incX, float incY)
  {
    _position.x = static_cast<float>(x);
    _position.y = static_cast<float>(y);
    _sizeIncrement.x = incX;
    _sizeIncrement.y = incY;
  }

  void Block::removeCondition()
  {
    _conditions.pop_back();
  }

  vector<ParamDefinition*>& Block::getInParams()
  {
    return _algorithmInParams;
  };
  vector<ParamDefinition*>& Block::getOutParams()
  {
    return _algorithmOutParams;
  };
}