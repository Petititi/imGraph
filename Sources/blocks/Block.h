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
    bool show_;
    ParamType type_;
    std::string name_;
    std::string helper_;
    ParamDefinition(bool show, ParamType type, std::string name, std::string helper) :
      show_(show), type_(type), name_(name), helper_(helper){};
  };

  class Block{
    friend class GraphOfProcess;
    friend charliesoft::ProcessManager;

  protected:
    boost::condition_variable cond_;  // parameter upgrade condition
    boost::mutex mtx_;    // explicit mutex declaration
    unsigned int timestamp_;

    std::string error_msg_;
    std::string name_;
    cv::Point2f position_;

    std::map<std::string, ParamValue> myOutputs_;
    std::map<std::string, ParamValue> myInputs_;

    void initParameters(std::vector<ParamDefinition>& inParam, 
      std::vector<ParamDefinition>& outParam);

    bool isUpToDate_;

    virtual bool run() = 0;
  public:
    Block(std::string name);
    std::string getName(){
      return name_;
    };
    void operator()();

    std::string getErrorInfo(){ return error_msg_; };
    bool validateParams(std::string param, const ParamValue val);
    bool isReadyToRun();

    virtual void setParam(std::string nameParam_, ParamValue& value);
    virtual ParamValue* getParam(std::string nameParam_);

    void updateIfNeeded() { if (!isUpToDate()) run(); };

    bool isUpToDate();
    void wakeUp();

    std::vector<BlockLink> getInEdges();

    std::string getErrorMsg();

    const cv::Point2f& getPosition() const { return position_; }
    void updatePosition(float x, float y) { position_.x = x; position_.y = y; };
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
    //edges are stored into Block (myInputs_[]->isLinked())
  public:
    static bool pauseProcess;
    GraphOfProcess();

    void saveGraph(boost::property_tree::ptree& tree) const;
    void fromGraph(boost::property_tree::ptree& tree);

    static unsigned int current_timestamp_;

    void addNewProcess(Block* filter);
    void deleteProcess(Block* process);

    bool run();
    void stop();
    void switchPause();

    std::vector<Block*>& getVertices();
  };
}

#endif