#include "Block.h"
#include <vector>

using namespace lsis_org;
using boost::graph_traits;
using std::vector;

namespace charliesoft
{
  Block::Block(std::string name){
    name_ = name;
    isUpToDate_ = false;
  };

  bool Block::isUpToDate()
  {
    auto it = myInputs_.begin();
    while (it != myInputs_.end())
    {
      if (it->second.isNew())
        return false;
      it++;
    }
    return true;
  }

  void Block::initParameters(std::vector<ParamDefinition>& inParam,
    std::vector<ParamDefinition>& outParam)
  {
    //add empty parameters:
    auto it = inParam.begin();
    while (it != inParam.end())
    {
      myInputs_[it->name_] = ParamValue(this, it->name_, false);
      it++;
    }
    it = outParam.begin();
    while (it != outParam.end())
    {
      myOutputs_[it->name_] = ParamValue(this, it->name_, true);
      it++;
    }
  }

  void Block::setParam(std::string nameParam_, ParamValue& value){
    if (myInputs_.find(nameParam_) != myInputs_.end())
      myInputs_[nameParam_] = &value;
    else
      myOutputs_[nameParam_] = value;
  };
  ParamValue* Block::getParam(std::string nameParam_){
    if (myInputs_.find(nameParam_) != myInputs_.end())
      return &myInputs_[nameParam_];
    else
      return &myOutputs_[nameParam_];
  };

  std::vector<BlockLink> Block::getInEdges()
  {
    vector<BlockLink> out;
    auto it = myInputs_.begin();
    while (it != myInputs_.end())
    {
      if (it->second.isLinked())
        out.push_back(BlockLink(it->second.toBlockLink()));
      it++;
    }
    return out;
  }

  GraphOfProcess::GraphOfProcess(){
  };

  void GraphOfProcess::addNewProcess(Block* block){
    vertices_.push_back(block);
  };

  void GraphOfProcess::deleteProcess(Block* process){
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
    {
      if (*it == process)
      {
        vertices_.erase(it);
        return;
      }
    }
  };

  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return vertices_;
  }

  void Block::createLink(std::string paramName, Block* dest, std::string paramNameDest)
  {
    dest->setParam(paramNameDest, myOutputs_[paramName]);
  }

  bool GraphOfProcess::run(Block* endingVertex)
  {
    if (endingVertex != NULL)
      return endingVertex->run();
    bool res = true;
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
      res &= (*it)->run();//order is not important: input parameters are automatically computed

    return res;
  }
}