#ifndef _BLOCK_IMGRAPH_HEADER_
#define _BLOCK_IMGRAPH_HEADER_

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
#ifdef _WIN32
#pragma warning(pop)
#endif
#endif

#include "Internationalizator.h"
#include "blocks/ParamValue.h"
#include <iostream>
#include "OpenCV_filter.h"
#include "ProcessManager.h"

//macro to add algo to list:
#define BLOCK_BEGIN_INSTANTIATION(className) \
  \
  class className## :public Block \
        { \
      friend charliesoft::ProcessManager; \
      static std::vector<ParamDefinition> getListParams(); \
      static std::vector<ParamDefinition> getListOutputs(); \
      static bool addedToList; \
  protected: \
    virtual bool run(); \
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

#define ADD_PARAMETER(show, type, name, helper) output.push_back(ParamDefinition( \
  show, type, name, helper));

#define END_BLOCK_PARAMS() return output; }

#define ADD_SUB_PARAMETER(nameSub, defaultVal) {_mySubParams[nameSub] = ParamValue(this, nameSub, false); _mySubParams[nameSub] = defaultVal;}

#define ADD_SUBPARAM_FROM_OPENCV_ALGO(algo, paramName, name) { \
  switch(algo->paramType(name)){\
    case cv::Param::BOOLEAN: \
      ADD_SUB_PARAMETER(paramName + "." + name, algo->get<bool>(name)); break;\
    case cv::Param::UNSIGNED_INT: \
    case cv::Param::UINT64: \
    case cv::Param::UCHAR: \
    case cv::Param::INT: \
      ADD_SUB_PARAMETER(paramName + "." + name, algo->get<int>(name)); break;\
    case cv::Param::REAL: \
    case cv::Param::FLOAT: \
      ADD_SUB_PARAMETER(paramName + "." + name, algo->get<double>(name)); break;\
    case cv::Param::STRING: \
      ADD_SUB_PARAMETER(paramName + "." + name, (string)algo->get<cv::String>(name)); break;\
    case cv::Param::MAT: \
      ADD_SUB_PARAMETER(paramName + "." + name, algo->get<cv::Mat>(name)); break;\
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
    ParamDefinition(bool show, ParamType type, std::string name, std::string helper) :
      _show(show), _type(type), _name(name), _helper(helper){};
  };

  class Block{
    friend class GraphOfProcess;
    friend charliesoft::ProcessManager;

  protected:
    bool _renderingSkiped;
    unsigned int nbRendering;
    GraphOfProcess* _processes;///<list of process currently in use
    boost::condition_variable _cond_pause;  // pause condition
    boost::mutex _mtx;    // explicit mutex declaration
    boost::condition_variable _cond_sync;  // global sync condition
    boost::mutex _mtx_timestamp_inc;    // Timestamps update
    unsigned int _timestamp;///<timestamp of produced values
    bool _fullyRendered;

    std::string _error_msg;
    std::string _name;
    cv::Point2f _position;

    std::map<std::string, ParamValue> _myOutputs;
    std::map<std::string, ParamValue> _myInputs;
    std::map<std::string, ParamValue> _mySubParams;
    std::vector<ConditionOfRendering> _conditions;

    void initParameters(const std::vector<ParamDefinition>& inParam, 
      const std::vector<ParamDefinition>& outParam);

    void renderingDone(bool fullyRendered = true);

    virtual bool run() = 0;
  public:
    Block(std::string name);
    std::string getName(){
      return _name;
    };
    void operator()();

    void setGraph(GraphOfProcess* processes){
      _processes = processes;
    };

    unsigned int getNbRendering() const { return nbRendering; }
    unsigned int getTimestamp(){ return _timestamp; };
    std::string getErrorInfo(){ return _error_msg; };
    bool validateParams(std::string param, const ParamValue val);
    bool isReadyToRun();
    void skipRendering();

    void addCondition(ConditionOfRendering& c){
      if (_conditions.empty())
        _conditions.push_back(c);
      else
        _conditions[0] = c;
    };
    std::vector<ConditionOfRendering>& getConditions(){
      return _conditions;
    };

    virtual void setParamLink(std::string nameParam_, ParamValue* value);
    virtual ParamValue* getParam(std::string nameParam_, bool input);

    virtual std::vector<cv::String> getSubParams(std::string param, int val){ return std::vector<cv::String>(); };

    bool isStartingBlock();
    bool isAncestor(Block* other);
    bool validTimestampOrWait(Block* other);
    bool validTimestampOrWait(Block* other, unsigned int timeGoal);
    void wakeUp();
    void waitUpdate(boost::unique_lock<boost::mutex>& lock);
    boost::mutex& getMutex(){
      return _mtx;
    };

    std::vector<BlockLink> getInEdges();

    std::string getErrorMsg();

    const cv::Point2f& getPosition() const { return _position; }
    void updatePosition(float x, float y) { _position.x = x; _position.y = y; };
    void createLink(std::string paramName, Block* dest, std::string paramNameDest);

    boost::property_tree::ptree getXML() const;
    void setPosition(int x, int y);
    void removeCondition();
  };

  struct BlockLink
  {
    Block* _from;
    Block* _to;
    std::string _fromParam;
    std::string _toParam;
    BlockLink(){ _from = _to = NULL; };
    BlockLink(Block* from,Block* to,std::string fromParam,std::string toParam)
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