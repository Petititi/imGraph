#ifndef _GRAPH_HEADER_
#define _GRAPH_HEADER_

#include "Block.h"

namespace charliesoft
{
  class GraphOfProcess
  {
    GraphOfProcess* _parent;
    std::map< Block*, std::set<Block*> > _waitingForRendering;
    std::map< Block*, boost::thread > _runningThread;
    std::vector<Block*> _vertices;
    boost::mutex _mtx;
    //edges are stored into Block (_myInputs[]->isLinked())

    void waitForFullRendering(Block* main_process, Block* process);
  public:
    static bool pauseProcess;
    GraphOfProcess(GraphOfProcess* parent=NULL);
    ~GraphOfProcess();

    void saveGraph(boost::property_tree::ptree& tree) const;
    void fromGraph(boost::property_tree::ptree& tree,
      std::map<unsigned int, ParamValue*>& = std::map<unsigned int, ParamValue*>());
    GraphOfProcess* getParent() const { return _parent; }
    void setParent(GraphOfProcess* val) { _parent = val; }

    static unsigned int _current_timestamp;
    void initChildDatas(Block*, std::set<Block*>& listOfRenderedBlocks);


    void addNewProcess(Block* filter);
    void deleteProcess(Block* process);
    void extractProcess(Block* process);

    void createLink(Block* src, std::string paramName, Block* dest, std::string paramNameDest);
    
    bool run(bool singleShot = false, bool delegateParent = true);
    void stop(bool delegateParent = true);
    void waitUntilEnd();
    void switchPause(bool delegateParent = true);

    /**
    * Will wake up waiting childs if needed... 
    */
    void blockProduced(Block* process, bool fullyRendered = true);

    void blockWantToSkip(Block* process);

    /**
    * Will wait for rendering block ancestor
    */
    void shouldWaitChild(Block* process);
    /**
    * Will wait for childs to process the data we just produce
    */
    void shouldWaitConsumers(Block* process);

    std::vector<Block*>& getVertices();
    void removeLink(const BlockLink& l);
  };
}


#endif