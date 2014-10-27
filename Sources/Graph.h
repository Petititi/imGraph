#ifndef _GRAPH_HEADER_
#define _GRAPH_HEADER_

#include "Block.h"

namespace charliesoft
{
  class GraphOfProcess
  {
    std::vector< boost::thread > _runningThread;
    std::vector<Block*> _vertices;
    //edges are stored into Block (_myInputs[]->isLinked())
  public:
    static bool pauseProcess;
    GraphOfProcess();
    ~GraphOfProcess();

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