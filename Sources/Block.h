#ifndef _BLOCK_IMGRAPH_HEADER_
#define _BLOCK_IMGRAPH_HEADER_

/*!
\page BlockSection Block
\section BlockSection_intro Introduction
<p>
!OLD!
A block (or node, or vertex, or process) represents an operation. This can be of several types (mathematical operation, image processing, loading, ...), as well as several rendering types:
- The asynchronous rendering. This block will produce the output independently of other blocks. It's useful when you want realtime processing.
- The "one shot" rendering. This block will produce the output only when input values changes. It's useful to save some CPU.
- The synchronous rendering. It's close to the "one shot" rendering, but the block also wait for the whole graph uses the value it has produced. It's useful when you want to process each frame of a video without skipping a single frame.

</p>
*/

#ifndef Q_MOC_RUN
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4244 4251 4275 4800 4503)
#endif
#include <boost/config.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opencv2/core.hpp>

#include <iostream>

#ifdef _WIN32
#pragma warning(pop)
#endif
#endif

#include "Internationalizator.h"
#include "blocks/ParamValue.h"
#include "ProcessManager.h"

//macro to add algo to list:
#define BLOCK_BEGIN_INSTANTIATION(className) \
  \
  class className## :public Block \
  { \
    friend class charliesoft::ProcessManager; \
    static std::vector<ParamDefinition> getListParams(); \
    static std::vector<ParamDefinition> getListOutputs(); \
    static std::vector<ParamDefinition> getListSubParams(); \
    static bool addedToList; \
  protected: \
    virtual bool run(bool oneShot=false); \
  public: \
    className##();

#define BLOCK_END_INSTANTIATION(className, blockType, keyName) \
  };bool className##::addedToList = \
    charliesoft::ProcessManager::getInstance()->addNewAlgo<##className##>(blockType, #keyName);\
    int className##_InMyLibrary() { return 0; };

#define BEGIN_BLOCK_INPUT_PARAMS(className) \
  std::vector<ParamDefinition> className##::getListParams(){ \
  std::vector<ParamDefinition> output;

#define BEGIN_BLOCK_OUTPUT_PARAMS(className) \
  std::vector<ParamDefinition> className##::getListOutputs(){ \
  std::vector<ParamDefinition> output;

#define BEGIN_BLOCK_SUBPARAMS_DEF(className) \
  std::vector<ParamDefinition> className##::getListSubParams(){ \
  std::vector<ParamDefinition> output;

#define ADD_PARAMETER_FULL(show, type, name, helper, initialValue) output.push_back(ParamDefinition( \
  show, type, name, helper, initialValue));

#define ADD_PARAMETER(show, type, name, helper) output.push_back(ParamDefinition( \
  show, type, name, helper));

#define END_BLOCK_PARAMS() return output; }

#define ADD_SUBPARAM_FROM_OPENCV_ALGO(algo, paramName, paramValName, name) { \
  switch(algo->paramType(name)){\
    case cv::Param::BOOLEAN: \
      ADD_PARAMETER_FULL(false, Boolean, ((std::string)paramName) + "." + paramValName + "." + name, name, algo->get<bool>(name)); break;\
    case cv::Param::UNSIGNED_INT: \
    case cv::Param::UINT64: \
    case cv::Param::UCHAR: \
    case cv::Param::INT: \
      ADD_PARAMETER_FULL(false, Int, ((std::string)paramName) + "." + paramValName + "." + name, name, algo->get<int>(name)); break;\
    case cv::Param::REAL: \
    case cv::Param::FLOAT: \
      ADD_PARAMETER_FULL(false, Float, ((std::string)paramName) + "." + paramValName + "." + name, name, algo->get<double>(name)); break;\
    case cv::Param::STRING: \
      ADD_PARAMETER_FULL(false, String, ((std::string)paramName) + "." + paramValName + "." + name, name, (string)algo->get<cv::String>(name)); break;\
    case cv::Param::MAT: \
      ADD_PARAMETER_FULL(false, Matrix, ((std::string)paramName) + "." + paramValName + "." + name, name, algo->get<cv::Mat>(name)); break;\
    } \
  }


namespace charliesoft
{
  class GraphOfProcess;
  struct BlockLink;
  class GraphOfProcess;
  class Block;

  class ConditionOfRendering
  {
    unsigned char category_left;
    ParamValue opt__valueleft;
    unsigned char category_right;
    ParamValue opt__valueright;
    unsigned char boolean_operator;
    Block* _father;
  public:
    ConditionOfRendering();
    ConditionOfRendering(unsigned char category_left,
      ParamValue opt__valueleft,
      unsigned char category_right,
      ParamValue opt__valueright,
      unsigned char boolean_operator,
      Block* father);
    ConditionOfRendering& operator= (const ConditionOfRendering &b)
    {
      category_left = b.category_left;
      opt__valueleft = b.opt__valueleft;
      category_right = b.category_right;
      opt__valueright = b.opt__valueright;
      boolean_operator = b.boolean_operator;
      if (b._father != NULL)
        _father = b._father;
      return *this;
    }
    unsigned char getCondition() const { return boolean_operator; }
    unsigned char getCategory_left() const { return category_left; }
    unsigned char getCategory_right() const { return category_right; }
    ParamValue getOpt__valueleft() const { return opt__valueleft; }
    ParamValue getOpt__valueright() const { return opt__valueright; }
    Block* getFather() const { return _father; }

    std::string toString();

    boost::property_tree::ptree getXML() const;

    bool canRender(Block* blockTested);

    void setValue(bool left, ParamValue* val);
    void setValue(bool left, ParamValue val) {
      if (left)opt__valueleft = val;
      else opt__valueright = val;
    }
    void setFather(Block* val) { _father = val; }
  };

  enum ParamVisibility
  {
    notUsed = 0,
    toBeLinked,
    userConstant
  };

  struct ParamDefinition
  {
    ParamVisibility _show;
    ParamType _type;
    std::string _name;
    std::string _helper;
    ParamValue _initVal;
    ParamDefinition() :
      _show(notUsed), _type(typeError), _name("_"), _helper("_"), _initVal(Not_A_Value()){};
    ParamDefinition(ParamVisibility show, ParamType type, std::string name, std::string helper) :
      _show(show), _type(type), _name(name), _helper(helper), _initVal(Not_A_Value()){};
    ParamDefinition(ParamVisibility show, ParamType type, std::string name, std::string helper, ParamValue initVal) :
      _show(show), _type(type), _name(name), _helper(helper), _initVal(initVal){};

    ParamDefinition(bool show, ParamType type, std::string name, std::string helper) :
      ParamDefinition(show ? toBeLinked : userConstant, type, name, helper){};
    ParamDefinition(bool show, ParamType type, std::string name, std::string helper, ParamValue initVal) :
      ParamDefinition(show ? toBeLinked : userConstant, type, name, helper, initVal){};
  };

  class AlgoPerformance 
  {
    boost::posix_time::time_duration totalTime;
    boost::posix_time::time_duration _maxTime;
    boost::posix_time::time_duration _minTime;
    int nbMeasures;
  public:
    AlgoPerformance();
    int getMeanPerf() const;
    int getMaxPerf() const{ return nbMeasures == 0 ? 0 : static_cast<int>(_maxTime.total_milliseconds()); };
    int getMinPerf() const{ return nbMeasures == 0 ? 0 : static_cast<int>(_minTime.total_milliseconds()); };
    void addNewPerf(boost::posix_time::time_duration newTime);
  };

  class Block{
    friend class GraphOfProcess;
    friend class charliesoft::ProcessManager;
  public:
    /**
    * Two types of blocks:
    * - Classic block. Called "synchrone" they just take some input and create some output. They also wait the output childs to process the data. Ex: line finder.
    * - The "asynchrones" : they produce data continuously without waiting anything.
    */
    enum BlockType
    {
      synchrone = 0,
      asynchrone
    };

    /**
    * Five states of blocks:
    * - waitingChild: Block wait for new input values... Only synchrone block can be in this state.
    * - consumingParams: Block is active and produce new datas
    * - consumedParams: Block produced new datas
    * - waitingConsumers: Block wait until all its output are consumed.
    * - stopped: Block is not yet started
    * - paused: Block wait until user start again the graph
    */
    enum BlockState
    {
      waitingChild = 0,
      consumingParams,
      consumedParams,
      waitingConsumers,
      stopped,
      paused
    };
  protected:
    boost::thread::id _threadID;
    boost::posix_time::ptime _time_start;///<used to measure processing time...
    AlgoPerformance _perfCounter;///<Used to record algo performance.

    BlockState _state;///<Current state of the block
    BlockType _exec_type;
    unsigned int nbRendering;
    GraphOfProcess* _processes;///<list of process currently in use
    boost::condition_variable _cond_pause;  ///< pause condition
    boost::mutex _mtx;    ///< explicit mutex declaration
    boost::condition_variable _wait_consume;  ///< parameter consumption sync condition
    boost::condition_variable _wait_processed;  ///<Block has processed the input datas
    boost::mutex _mtx_timestamp_inc;    ///< Timestamps update
    unsigned int _timestamp;///<timestamp of produced values

    std::string _error_msg;
    std::string _name;
    cv::Point2f _position;
    cv::Point2f _sizeIncrement;

    std::string _currentPreview;

    std::vector<ParamDefinition*> _algorithmInParams;///<definition of each input parameters
    std::vector<ParamDefinition*> _algorithmSubParams;///<definition of each input sub-parameters
    std::vector<ParamDefinition*> _algorithmOutParams;///<definition of each output parameters


    std::map<std::string, ParamValue> _myOutputs;///<output value and name of parameters
    std::map<std::string, ParamValue> _myInputs;///<input value and name of parameters
    std::map<std::string, ParamValue> _mySubParams;///<sub-params value and name of parameters
    std::vector<ConditionOfRendering> _conditions;///<Conditions needed for block rendering

    /**
    * This function is used by ProcessManager to init block with default values...
    */
    void initParameters(const std::vector<ParamDefinition>& inParam,
      const std::vector<ParamDefinition>& outParam);

    void newProducedData();
    void paramsFullyProcessed();

    bool _executeOnlyOnce;
    bool _newData;
    bool _isOneShot;
    bool _addInputParam;
  public:
    /**
    Used to create a block...
    @param name block name, can be a label from Internationalizator
    @param isOneShot if the block is simple (that is, the block is state-free), set this value to true.
    @param typeExec used to know the type of execution this bloc should have...
    */
    Block(std::string name, bool isOneShot, BlockType typeExec = synchrone, bool addInputParam = false);
    ///The derived block may want a personalized destructor...
    virtual ~Block();
    std::string getName(){
      return _name;
    };
    BlockType getTypeExec(){
      return _exec_type;
    };

    void operator()();
    virtual bool run(bool oneShot = false) = 0;

    bool hasDynamicParams() const { return _addInputParam; };
    /**
    Used to add a new parameter...
    @param param definition of the new input parameter
    */
    ParamValue* addNewInput(ParamDefinition* param);
    /**
    Used to add a new output...
    @param param definition of the new output value
    */
    ParamValue* addNewOutput(ParamDefinition* param);

    virtual void init(){};
    virtual void release(){};

    ///Use this function to reset state of process (mark every input/output as old)
    void markAsUnprocessed();

    ParamDefinition* getParamDefinition(std::string nameOfParam, bool isInput);
    const ParamDefinition* getParamDefinition(std::string nameOfParam, bool isInput) const;
    bool paramDefinitionExist(std::string nameOfParam, bool isInput);

    std::string getCurrentPreview() const { return _currentPreview; }
    void setCurrentPreview(std::string val) { _currentPreview = val; }

    const AlgoPerformance& getPerf() const { return _perfCounter; }

    virtual void setGraph(GraphOfProcess* processes){
      _processes = processes;
    };
    GraphOfProcess* getGraph() const { return _processes; };

    BlockState getState() const { return _state; };
    void setState(BlockState val) { _state = val; };

    BlockType getExecType() const { return _exec_type; }
    void setExecType(BlockType val) { _exec_type = val; }

    unsigned int getNbRendering() const { return nbRendering; }
    unsigned int getTimestamp(){ return _timestamp; };
    std::string getErrorInfo(){ return _error_msg; };
    bool validateParams(std::string param, const ParamValue val);
    ///if realCheck is true, we look for links being ready to consume
    bool isReadyToRun(bool realCheck=false);

    void addCondition(ConditionOfRendering& c){
      if (_conditions.empty())
        _conditions.push_back(c);
      else
        _conditions[0] = c;
    };
    std::vector<ConditionOfRendering>& getConditions(){
      return _conditions;
    };

    virtual void setParamValue(std::string nameParam_, ParamValue* value);
    virtual void setParam(std::string nameParam_, ParamValue value);
    virtual ParamValue* getParam(std::string nameParam_, bool input);

    virtual std::vector<cv::String> getSubParams(std::string paramName);

    bool isStartingBlock();
    bool isAncestor(Block* other);
    bool hasNewParameters();//true if at least one parameter has timestamp>block timestamp
    void wakeUpFromConsumers();
    void notifyProduction();
    void wakeUpFromPause();
    void waitConsumers(boost::unique_lock<boost::mutex>& lock);
    void waitProducers(boost::unique_lock<boost::mutex>& lock);
    boost::mutex& getMutex(){
      return _mtx;
    };
    boost::mutex& getMutexTimestamp(){
      return _mtx_timestamp_inc;
    };

    std::vector<BlockLink> getInEdges();

    std::string getErrorMsg();

    const cv::Point2f& getPosition() const { return _position; }
    const cv::Point2f& getSizeIncrement() const { return _sizeIncrement; }
    void updatePosition(float x, float y) { _position.x = x; _position.y = y; };
    void linkParam(std::string paramName, Block* dest, std::string paramNameDest);

    virtual boost::property_tree::ptree getXML();
    virtual void initFromXML(boost::property_tree::ptree* tree,
      std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
      std::map<unsigned int, ParamValue*>& addressesMap,
      std::vector<ConditionOfRendering*>& condToUpdate);

    void setPosition(int x, int y, float incX, float incY);
    void setIncrSize(float incX, float incY);
    void removeCondition();

    ///Re-run this block. If links exist, this will also render every ancestors.
    void update();
    bool shouldExecuteOnlyOnce() const { return _executeOnlyOnce; }
    void setExecuteOnlyOnce(bool val) { _executeOnlyOnce = val; }

    std::vector<ParamDefinition*>& getInParams();
    std::vector<ParamDefinition*>& getOutParams();
    std::map<std::string, ParamValue>const & getInputsVals() const { return _myInputs; }
    std::map<std::string, ParamValue>const & getOutputsVals() const { return _myOutputs; }

    ParamDefinition getDef(std::string name, bool isInput) const;
  };

  struct BlockLink
  {
    Block* _from;
    Block* _to;
    std::string _fromParam;
    std::string _toParam;
    BlockLink(){ _from = _to = NULL; };
    BlockLink(Block* from, Block* to, std::string fromParam, std::string toParam)
    {
      _from = from; _to = to; _fromParam = fromParam; _toParam = toParam;
    }
    bool operator==(const BlockLink &b) const{
      return (_from == b._from) && (_to == b._to) &&
        (_fromParam.compare(b._fromParam) == 0) && (_toParam.compare(b._toParam) == 0);
    }
    bool operator<(const BlockLink & record2) const{
      if (_from < record2._from) return true;
      if (_from > record2._from) return false;
      if (_to < record2._to) return true;
      if (_to > record2._to) return false;
      if (_fromParam < record2._fromParam) return true;
      if (_fromParam > record2._fromParam) return false;
      return _toParam < record2._toParam;
    }
    bool operator<(BlockLink & record2){
      if (_from < record2._from) return true;
      if (_from > record2._from) return false;
      if (_to < record2._to) return true;
      if (_to > record2._to) return false;
      if (_fromParam < record2._fromParam) return true;
      if (_fromParam > record2._fromParam) return false;
      return _toParam < record2._toParam;
    }
  };

}

#endif