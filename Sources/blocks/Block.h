#ifndef _BLOCK_IMGRAPH_HEADER_
#define _BLOCK_IMGRAPH_HEADER_

#ifndef Q_MOC_RUN
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <boost/config.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/property_tree/ptree.hpp>
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

namespace charliesoft
{
  class GraphOfProcess;
  struct BlockLink;
  class GraphOfProcess;
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
    GraphOfProcess* _processes;///<list of process currently in use
    boost::condition_variable _cond;  // parameter upgrade condition
    boost::mutex _mtx;    // explicit mutex declaration
    boost::condition_variable _cond_sync;  // global sync condition
    boost::mutex _mtx_timestamp_inc;    // Timestamps update
    unsigned int _timestamp;///<timestamp of produced values
    unsigned int _work_timestamp;///<timestamp of values we are working on
    bool _fullyRendered;

    std::string _error_msg;
    std::string _name;
    cv::Point2f _position;

    std::map<std::string, ParamValue> _myOutputs;
    std::map<std::string, ParamValue> _myInputs;

    void initParameters(const std::vector<ParamDefinition>& inParam, 
      const std::vector<ParamDefinition>& outParam);

    void renderingDone(bool fullyRendered=true);

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

    unsigned int getTimestamp(){ return _timestamp; };
    std::string getErrorInfo(){ return _error_msg; };
    bool validateParams(std::string param, const ParamValue val);
    bool isReadyToRun();

    virtual void setParam(std::string nameParam_, ParamValue& value);
    virtual ParamValue* getParam(std::string nameParam_, bool input);

    bool isStartingBlock();
    bool isAncestor(Block* other);
    bool validTimestampOrWait(unsigned int timestamp);
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
  };


  struct BlockLink
  {
    Block* from_;
    Block* to_;
    std::string fromParam_;
    std::string toParam_;
    BlockLink(){ from_ = to_ = NULL; };
    BlockLink(Block* from,Block* to,std::string fromParam,std::string toParam)
    {
      from_ = from; to_ = to; fromParam_ = fromParam; toParam_ = toParam;
    }
    bool operator==(const BlockLink &b) const{
      return (from_ == b.from_) && (to_ == b.to_) &&
        (fromParam_.compare(b.fromParam_) == 0) && (toParam_.compare(b.toParam_) == 0);
    }
    bool operator<(const BlockLink & record2) const{
      if (from_ < record2.from_) return true;
      if (from_ > record2.from_) return false;
      if (to_ < record2.to_) return true;
      if (to_ > record2.to_) return false;
      if (fromParam_ < record2.fromParam_) return true;
      if (fromParam_ > record2.fromParam_) return false;
      return toParam_ < record2.toParam_;
    }
    bool operator<(BlockLink & record2){
      if (from_ < record2.from_) return true;
      if (from_ > record2.from_) return false;
      if (to_ < record2.to_) return true;
      if (to_ > record2.to_) return false;
      if (fromParam_ < record2.fromParam_) return true;
      if (fromParam_ > record2.fromParam_) return false;
      return toParam_ < record2.toParam_;
    }
  };

  class GraphOfProcess
  {
    std::vector< boost::thread > runningThread_;
    std::vector<Block*> vertices_;
    //edges are stored into Block (_myInputs[]->isLinked())
  public:
    static bool pauseProcess;
    GraphOfProcess();

    void saveGraph(boost::property_tree::ptree& tree) const;
    void fromGraph(boost::property_tree::ptree& tree);

    static unsigned int _current_timestamp;

    void addNewProcess(Block* filter);
    void deleteProcess(Block* process);

    void synchronizeTimestamp(Block* processToSynchronize);

    bool run();
    void stop();
    void switchPause();

    std::vector<Block*>& getVertices();
  };
}

#endif