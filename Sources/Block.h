#ifndef _BLOCK_IMGRAPH_HEADER_
#define _BLOCK_IMGRAPH_HEADER_

/*!
\page BlockSection Block
\section BlockSection_intro Introduction
<p>
A block (or node, or vertex, or process) represents an operation. This can be of several types (mathematical operation, image processing, loading, ...), as well as several rendering types:
- The asynchronous rendering. This block will produce the output independently of other blocks. It's useful when you want realtime processing.
- The "one shot" rendering. This block will produce the output only when input values changes. It's useful to save some CPU.
- The synchronous rendering. It's close to the "one shot" rendering, but the block also wait for the whole graph uses the value it has produced. It's useful when you want to process each frame of a video without skipping a single frame.

</p>
*/

#ifndef Q_MOC_RUN
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <boost/config.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
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
    charliesoft::ProcessManager::getInstance()->addNewAlgo<##className##>(blockType, #keyName);

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
    ParamValue opt_value_left;
    unsigned char category_right;
    ParamValue opt_value_right;
    unsigned char boolean_operator;
    Block* _father;
  public:
    ConditionOfRendering();
    ConditionOfRendering(unsigned char category_left,
      ParamValue opt_value_left,
      unsigned char category_right,
      ParamValue opt_value_right,
      unsigned char boolean_operator,
      Block* father);
    ConditionOfRendering& operator= (const ConditionOfRendering &b)
    {
      category_left = b.category_left;
      opt_value_left = b.opt_value_left;
      category_right = b.category_right;
      opt_value_right = b.opt_value_right;
      boolean_operator = b.boolean_operator;
      if (b._father != NULL)
        _father = b._father;
      return *this;
    }
    unsigned char getCondition() const { return boolean_operator; }
    unsigned char getCategory_left() const { return category_left; }
    unsigned char getCategory_right() const { return category_right; }
    ParamValue getOpt_value_left() const { return opt_value_left; }
    ParamValue getOpt_value_right() const { return opt_value_right; }
    Block* getFather() const { return _father; }

    std::string toString();

    boost::property_tree::ptree getXML() const;

    bool canRender(Block* blockTested);

    void setValue(bool left, ParamValue* val);
    void setValue(bool left, ParamValue val) {
      if (left)opt_value_left = val;
      else opt_value_right = val;
    }
    void setFather(Block* val) { _father = val; }
  };

  struct ParamDefinition
  {
    bool _show;
    ParamType _type;
    std::string _name;
    std::string _helper;
    ParamValue _initVal;
    ParamDefinition() :
      _show(false), _type(typeError), _name("_"), _helper("_"), _initVal(Not_A_Value()){};
    ParamDefinition(bool show, ParamType type, std::string name, std::string helper) :
      _show(show), _type(type), _name(name), _helper(helper), _initVal(Not_A_Value()){};
    ParamDefinition(bool show, ParamType type, std::string name, std::string helper, ParamValue initVal) :
      _show(show), _type(type), _name(name), _helper(helper), _initVal(initVal){};
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
    * Four states of blocks:
    * - Block wait for new input values... Only synchrone block can be in this state.
    * - Block is active and produce new datas
    * - Block wait until all its output are consumed.
    * - Block is not yet started
    */
    enum BlockState
    {
      waitingChild = 0,
      running,
      waitingConsumers,
      stopped
    };
  protected:
    boost::thread::id _threadID;

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

    std::map<std::string, ParamValue> _myOutputs;
    std::map<std::string, ParamValue> _myInputs;
    std::map<std::string, ParamValue> _mySubParams;
    std::vector<ConditionOfRendering> _conditions;

    void initParameters(const std::vector<ParamDefinition>& inParam,
      const std::vector<ParamDefinition>& outParam);

    void newProducedData(bool fullyRendered);

    virtual bool run(bool oneShot = false) = 0;
    bool _executeOnlyOnce;
    bool _newData;
  public:
    Block(std::string name, BlockType typeExec = synchrone);
    ~Block();
    std::string getName(){
      return _name;
    };
    BlockType getTypeExec(){
      return _exec_type;
    };
    void operator()();

    virtual void init(){};
    virtual void release(){};

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
    void updatePosition(float x, float y) { _position.x = x; _position.y = y; };
    void linkParam(std::string paramName, Block* dest, std::string paramNameDest);

    virtual boost::property_tree::ptree getXML();
    virtual void initFromXML(boost::property_tree::ptree* tree,
      std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
      std::map<unsigned int, ParamValue*>& addressesMap,
      std::vector<ConditionOfRendering*>& condToUpdate);

    void setPosition(int x, int y);
    void removeCondition();

    ///Re-run this block. If links exist, this will also render every ancestors.
    void update();
    bool shouldExecuteOnlyOnce() const { return _executeOnlyOnce; }
    void setExecuteOnlyOnce(bool val) { _executeOnlyOnce = val; }

    virtual std::vector<ParamDefinition> getInParams() const;
    virtual std::vector<ParamDefinition> getOutParams() const;
    std::map<std::string, ParamValue>const & getInputsVals() const { return _myInputs; }
    std::map<std::string, ParamValue>const & getOutputsVals() const { return _myOutputs; }
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