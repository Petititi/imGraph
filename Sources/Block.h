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

#include <iostream>
#include "OpenCV_filter.h"

namespace charliesoft
{
  class GraphOfProcess;
  typedef boost::square_topology<>::point_type Point_;//position of vertex

  class Block{
    GraphOfProcess* graph_;//<Graph who own this block
    Point_* position_;//<adress of VertexProperties_ position!
  public:
    Block(){ position_ = NULL; graph_ = NULL; };
    virtual void execute() = 0;
    virtual std::string getName() = 0;
    virtual std::vector<std::string> getListParams() = 0;
    virtual std::vector<std::string> getListOutputs() = 0;

    Point_* getPosition() const { return position_; }
    void setPosition(Point_* val) { position_ = val; }
    GraphOfProcess* getGraph() const { return graph_; }
    void setGraph(GraphOfProcess* val) { graph_ = val; }
  };

  class BlockLoader :public Block
  {
  public:
    BlockLoader(){};
    virtual void execute(){};
    virtual std::string getName();
    virtual std::vector<std::string> getListParams();
    virtual std::vector<std::string> getListOutputs();
  };

  class GraphOfProcess
  {

    struct VertexProperties_
    {
      Point_ position_;
      Block* block_;
      VertexProperties_(){ block_ = NULL; position_[0] = position_[1] = 0; };
    };

    struct EdgeProperty_
    {
      double weight;        //<not used now... Always equal to 1
      std::string propFrom; //<name of the property the previous filter is connected to
      std::string propTo;   //<name of the property the next filter is connected to
      EdgeProperty_(std::string from, std::string to) :propFrom(from), propTo(to){ weight = 1.; };//each edges have same weight
    };


    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
      VertexProperties_, EdgeProperty_ > Graph_Intern_;
    typedef boost::labeled_graph < Graph_Intern_, Block* > Graph_;

    Graph_ myGraph_;

  public:
    GraphOfProcess();

    void addNewProcess(Block* filter);
    void createNewConnection(std::string propFrom, std::string propTo,
      Block* from, Block* to);
    void deleteConnection(std::string propFrom, std::string propTo,
      Block* from, Block* to);
    void deleteProcess(Block* process);

    std::vector<Block*> getNodes();
  };
}

#endif