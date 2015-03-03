#ifndef _BLOCK_SUB_DEF_H
#define _BLOCK_SUB_DEF_H
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "Graph.h"

namespace charliesoft
{

  class SubBlock :public Block
  {
  protected:
    friend class charliesoft::ProcessManager;
    friend class charliesoft::GraphOfProcess;

    //////////////////////////////////////////////////////////////////////////
    //The parameters are only known after instanciation...
    static std::vector<ParamDefinition> getListParams()
      {return std::vector <ParamDefinition>();}
    static std::vector<ParamDefinition> getListOutputs()
      {return std::vector <ParamDefinition>();}
    static std::vector<ParamDefinition> getListSubParams()
      {return std::vector <ParamDefinition>();}
    //////////////////////////////////////////////////////////////////////////
    static bool addedToList;
    boost::mutex _mtx_1;    // explicit mutex declaration


    std::vector<BlockLink> externBlocksInput;///<input parameters the user had added to block
    std::vector<BlockLink> externBlocksOutput;///<output parameters the user had added to block

    boost::condition_variable _wait_param_update;  ///< parameter update from subgraph sync condition

    GraphOfProcess* _subGraph;
    virtual bool run(bool oneShot = false);
  public:
    SubBlock();
    SubBlock(std::string name);

    virtual void init();
    virtual void release();

    virtual void setGraph(GraphOfProcess* processes){
      _processes = processes;
      _subGraph->setParent(processes, this);
    };

    virtual boost::property_tree::ptree getXML();
    virtual void initFromXML(boost::property_tree::ptree* tree,
      std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
      std::map<unsigned int, ParamValue*>& addressesMap,
      std::vector<ConditionOfRendering*>& condToUpdate);

    GraphOfProcess* getSubGraph() const { return _subGraph; };
    void addExternLink(const BlockLink& link, bool isInput);
    void removeExternLink(const BlockLink& link);

    void waitUpdateParams(boost::unique_lock<boost::mutex>& lock);

    const std::vector<BlockLink>& getExternBlocksInput() const { return externBlocksInput; }
    const std::vector<BlockLink>& getExternBlocksOutput() const { return externBlocksOutput; }
  };

};

#endif