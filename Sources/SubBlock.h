#ifndef _BLOCK_SUB_DEF_H
#define _BLOCK_SUB_DEF_H
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

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
    std::vector<ParamDefinition> _inputParams;
    std::vector<ParamDefinition> _outputParams;
    virtual bool run(bool oneShot = false);
  public:
    SubBlock();

    void addNewInput(ParamDefinition& param);
    void addNewOutput(ParamDefinition& param);
    std::vector<ParamDefinition> getInputs(){ return _inputParams; };
    std::vector<ParamDefinition> getOutputs(){ return _outputParams; };


    virtual std::vector<ParamDefinition> getInParams() const;
    virtual std::vector<ParamDefinition> getOutParams() const;
  };

};

#endif