#ifndef _GRAPH_HEADER_
#define _GRAPH_HEADER_

#include "Block.h"


/*!
\page ProcessGraph Scheduling and synchronization
\section ProcessGraph_intro Introduction
<p>
As seen in section \ref BlockSection, there is 2 types of rendering allowed : asynchronous and synchronous. The differences between each mode imply that the graph processing should allow "push" and "pull" data flow.<br/>
 - a "Push" event occurs when a block produces a value and wake up a child block which was waiting for a value (this is true only if each input value of this block is new or constant). This is the case when, for example, an input block read a frame from a video file and give this frame to the next block.<br/>
 - a "Pull" event occurs when a block asks for a value instead of waiting the parent to produce. This is the case when, for example, a block want to skip a frame: it will then ask the parent to render again.
</p>
<p>
In order to allow these functionalities, we use a double linked oriented graph. Each node of the graph need to know every child that are connected, as well as its parents.
\ref charliesoft::GraphOfProcess stores a std::map of \ref charliesoft::Block, and each block stores a std::map of parameters (\ref charliesoft::ParamValue) which can be then be an edge of the graph. A parameter can indeed be linked to another parameter.
In this case, the parameter knows its parent (the block which owns him), and if it's an input parameter, its value will be the address of the output parameter.
If it's an output parameter, the children can be recovered from \ref charliesoft::ParamValue::_distantListeners.
</p>
*/

namespace charliesoft
{
  class SubBlock;
  class GraphOfProcess
  {
    static bool _pauseProcess;
    GraphOfProcess* _parent;
    SubBlock *_subBlock;
    std::map< Block*, std::set<Block*> > _waitingForRendering;
    std::map< Block*, boost::thread > _runningThread;
    std::vector<Block*> _vertices;
    boost::mutex _mtx;
    //edges are stored into Block (_myInputs[]->isLinked())

  public:
    GraphOfProcess();
    ~GraphOfProcess();

    void saveGraph(boost::property_tree::ptree& tree) const;
    void fromGraph(boost::property_tree::ptree& tree,
      std::map<unsigned int, ParamValue*>& = std::map<unsigned int, ParamValue*>());
    GraphOfProcess* getParent() const { return _parent; }
    void setParent(GraphOfProcess* val, SubBlock* block) { _parent = val; _subBlock = block; }

    void initChildDatas(Block*, std::set<Block*>& listOfRenderedBlocks);


    void addNewProcess(Block* filter);
    void deleteProcess(Block* process);
    void extractProcess(Block* process);

    void createLink(Block* src, std::string paramName, Block* dest, std::string paramNameDest);
    
    bool run(bool singleShot = false, bool delegateParent = true);
    void stop(bool delegateParent = true, bool waitEnd = true);
    void waitUntilEnd(size_t max_ms_time = 0);
    bool switchPause(bool delegateParent = true);
    bool isPause(){
      return _pauseProcess;
    };

    /**
    * Will wake up waiting childs if needed... 
    */
    void blockProduced(Block* process, bool fullyRendered = true);

    /**
    * Will wait end of rendering for every block ancestor
    */
    void shouldWaitAncestors(Block* process);
    /**
    * Ask an update for each block ancestor
    */
    void updateAncestors(Block* process);
    /**
    * Will wait for childs to process the data we just produce
    * If process is NULL, will wait for every process in the graph
    */
    void shouldWaitConsumers(Block* process=NULL);

    /**
    * Reset the waiting list of block of the parameter
    */
    void clearWaitingList(Block* process);

    std::vector<Block*>& getVertices();
    void removeLink(const BlockLink& l);
  };
}


#endif