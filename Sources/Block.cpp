#include "Block.h"
#include "Internationalizator.h"
#include <vector>

using namespace lsis_org;
using namespace boost;
using std::vector;

namespace charliesoft
{

  std::string BlockLoader::getName(){
    return _STR("BLOCK__INPUT_NAME");
  };
  std::vector<std::string> BlockLoader::getListParams(){
    std::vector<std::string> output;
    output.push_back(_STR("BLOCK__INPUT_PARAM_IN_FILE"));
    return output;
  };
  std::vector<std::string> BlockLoader::getListOutputs(){
    std::vector<std::string> output;
    output.push_back(_STR("BLOCK__INPUT_PARAM_OUT_FRAMERATE"));
    output.push_back(_STR("BLOCK__INPUT_PARAM_OUT_WIDTH"));
    output.push_back(_STR("BLOCK__INPUT_PARAM_OUT_HEIGHT"));
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
    auto g = myGraph_.graph();
    //first get edge:
    graph_traits<Graph_>::out_edge_iterator ei, ei_end;
    tie(ei, ei_end) = out_edges(myGraph_.vertex(from), g);
    Graph_Intern_::vertex_descriptor node2 = myGraph_.vertex(to);
    for (; ei != ei_end; ++ei) {
      if (boost::target(*ei, myGraph_) == node2) {
        if (g[*ei].propFrom.compare(propFrom) == 0 &&
          g[*ei].propTo.compare(propTo) == 0)
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
    auto g = myGraph_.graph();
    auto blocks = get(&VertexProperties_::block_, g);

    std::vector<Block*> out;
    Graph_Intern_::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    {
      Block* block = blocks[*vi];
      if (block!=NULL)
        out.push_back(block);
    }
    return out;
  }

  std::vector<BlockLink> GraphOfProcess::getLinks()
  {
    vector<BlockLink> output;

    auto g = myGraph_.graph();
    for (auto ep = edges(g); ep.first != ep.second; ++ep.first)
    {
      // Get the two vertices that are joined by this edge...
      auto u = source(*ep.first, g), v = target(*ep.first, g);

      output.push_back(BlockLink(g[u].block_, g[v].block_,
        get(&EdgeProperty_::propFrom, g, *ep.first),
        get(&EdgeProperty_::propTo, g, *ep.first)));
    }
    return output;
  }

  void Block::createLink(std::string paramName, Block* dest, std::string paramNameDest)
  {
    graph_->createNewConnection(paramName, paramNameDest, this, dest);
  }

}