#include "Block.h"
#include "Internationalizator.h"
#include <vector>

using namespace lsis_org;
using namespace boost;
using std::vector;

namespace charliesoft
{

  std::string BlockLoader::getName(){
    return _STR("BLOCK_INPUT_NAME");
  };
  std::vector<std::string> BlockLoader::getListParams(){
    std::vector<std::string> output;
    output.push_back(_STR("BLOCK_INPUT_PARAM_INPUT"));
    return output;
  };
  std::vector<std::string> BlockLoader::getListOutputs(){
    std::vector<std::string> output;
    output.push_back(_STR("BLOCK_INPUT_PARAM_FRAMERATE"));
    output.push_back(_STR("BLOCK_INPUT_PARAM_WIDTH"));
    output.push_back(_STR("BLOCK_INPUT_PARAM_HEIGHT"));
    return output;
  };

  GraphOfProcess::GraphOfProcess(){
  };

  void GraphOfProcess::addNewProcess(Block* block){
    block->setGraph(this);

    VertexProperties_& v = myGraph_.graph()[myGraph_.add_vertex(block)];
    block->setPosition(&v.position_);//link the position of block to graph
    v.block_ = block;
  };

  void GraphOfProcess::createNewConnection(std::string propFrom, std::string propTo,
    Block* from, Block* to){
    add_edge_by_label(from, to, EdgeProperty_(propFrom, propTo), myGraph_);
  };

  void GraphOfProcess::deleteConnection(std::string propFrom, std::string propTo,
    Block* from, Block* to){
    //first get edge:
    graph_traits<Graph_>::out_edge_iterator ei, ei_end;
    tie(ei, ei_end) = out_edges(myGraph_.vertex(from), myGraph_);
    Graph_Intern_::vertex_descriptor node2 = myGraph_.vertex(to);
    for (; ei != ei_end; ++ei) {
      if (boost::target(*ei, myGraph_) == node2) {
        if (myGraph_.graph()[*ei].propFrom.compare(propFrom) == 0 &&
          myGraph_.graph()[*ei].propTo.compare(propTo) == 0)
        {
          //we should delete this edge!
          remove_edge(*ei, myGraph_);
          return;//only 1 edge at the time...
        }
      };
    };
  };

  void GraphOfProcess::deleteProcess(Block* process){
    myGraph_.remove_vertex(process);
  };

  std::vector<Block*> GraphOfProcess::getNodes()
  {
    typedef property_map<Graph_Intern_, Block* VertexProperties_::*>::type BlockMap;
    BlockMap blocks = get(&VertexProperties_::block_, myGraph_.graph());

    std::vector<Block*> out;
    Graph_Intern_::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(myGraph_.graph()); vi != vi_end; ++vi)
    {
      Block* block = blocks[*vi];
      if (block!=NULL)
        out.push_back(block);
    }
    return out;
  }
}