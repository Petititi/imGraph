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
    friend class charliesoft::ProcessManager;

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
  protected:
    std::map<std::string, ParamDefinition> _inputParams;
    std::map<std::string, ParamDefinition> _outputParams;

    std::vector<BlockLink> externBlocksInput;
    std::vector<BlockLink> externBlocksOutput;

    GraphOfProcess* _subGraph;
    virtual bool run(bool oneShot = false);
  public:
    SubBlock();

    ParamValue* addNewInput(ParamDefinition& param);
    ParamValue* addNewOutput(ParamDefinition& param);

    virtual std::vector<ParamDefinition> getInParams() const;
    virtual std::vector<ParamDefinition> getOutParams() const;

    virtual boost::property_tree::ptree getXML() const;
    virtual void initFromXML(boost::property_tree::ptree* tree,
      std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
      std::map<unsigned int, ParamValue*>& addressesMap,
      std::vector<ConditionOfRendering*>& condToUpdate);

    GraphOfProcess* getSubGraph() const { return _subGraph; };
    void addExternLink(BlockLink link, bool isInput);
  };

};

#endif