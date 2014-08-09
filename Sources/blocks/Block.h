#ifndef _BLOCK_IMGRAPH_HEADER_
#define _BLOCK_IMGRAPH_HEADER_

#ifndef Q_MOC_RUN
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
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
  }; \
  bool className##::addedToList = \
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
  typedef boost::square_topology<>::point_type Point_;//position of vertex
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
    std::string error_msg_;
    std::string name_;
    Point_ position_;

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

    std::string getErrorInfo(){ return error_msg_; };
    bool validateParams(std::string param, const ParamValue val);

    virtual void setParam(std::string nameParam_, ParamValue& value);
    virtual ParamValue* getParam(std::string nameParam_);

    void updateIfNeeded() { if (!isUpToDate()) run(); };

    bool isUpToDate();

    std::vector<BlockLink> getInEdges();

    std::string getErrorMsg();

    const Point_& getPosition() const { return position_; }
    void updatePosition(int x, int y) { position_[0] = x; position_[1] = y; };
    void createLink(std::string paramName, Block* dest, std::string paramNameDest);
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
    std::vector<Block*> vertices_;
    //edges are stored into Block (myInputs_[]->isLinked())
  public:
    GraphOfProcess();

    void addNewProcess(Block* filter);
    void deleteProcess(Block* process);

    bool run(Block* endingVertex = NULL);

    std::vector<Block*>& getVertices();
  };
}

#endif