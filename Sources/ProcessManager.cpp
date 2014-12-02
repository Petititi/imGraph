#include "ProcessManager.h"
#include "Block.h"
#include <vector>

using namespace lsis_org;
using boost::recursive_mutex;
using boost::lock_guard;
using std::vector;

namespace charliesoft
{
  ProcessManager* ProcessManager::_ptr = NULL;
  recursive_mutex ProcessManager::_listBlockMutex;

  ProcessManager::ProcessManager(){
    _ptr = this;
  };

  ProcessManager* ProcessManager::getInstance()
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    if (_ptr == NULL)
      _ptr = new ProcessManager();
    return _ptr;
  }

  void ProcessManager::releaseInstance()
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    if (_ptr != NULL)
      delete _ptr;
    _ptr = NULL;
  }

  Block* ProcessManager::createAlgoInstance(std::string algo_name) const
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    if (algo_factory_.find(algo_name) == algo_factory_.end())
      return NULL;
    Block* b = algo_factory_.at(algo_name)();
    b->initParameters(algorithmInParams_.at(algo_name), algorithmOutParams_.at(algo_name));
    return b;
  }

  AlgoType ProcessManager::getAlgoType(std::string algo_name) const
  {
    for (auto it : listOfAlgorithms_)
    {
      for (auto algo : it.second)
        if (algo.compare(algo_name) == 0)
          return it.first;
    }
    return output;
  }

  std::vector<std::string> ProcessManager::getAlgos(AlgoType type) const
  {
    if (listOfAlgorithms_.find(type) != listOfAlgorithms_.end())
      return listOfAlgorithms_.at(type);
    else
      return std::vector<std::string>();
  }

  const std::vector<ParamDefinition>& ProcessManager::getAlgo_InParams(std::string name) const
  {
    if (algorithmInParams_.find(name) != algorithmInParams_.end())
      return algorithmInParams_.at(name);
    else
      return emptyVector;
  }

  const std::vector<ParamDefinition>& ProcessManager::getAlgo_SubParams(std::string name) const
  {
    if (algorithmSubParams_.find(name) != algorithmSubParams_.end())
      return algorithmSubParams_.at(name);
    else
      return emptyVector;
  }
  const std::vector<ParamDefinition>& ProcessManager::getAlgo_OutParams(std::string name) const
  {
    if (algorithmOutParams_.find(name) != algorithmOutParams_.end())
      return algorithmOutParams_.at(name);
    else
      return emptyVector;
  }
  
  ParamType ProcessManager::getParamType(std::string algo_name, std::string paramName, bool input) const
  {
    if (input)
    {
      const std::vector<ParamDefinition> &params = getAlgo_InParams(algo_name);
      auto it = params.begin();
      while (it != params.end())
      {
        if (it->_name.compare(paramName) == 0)
          return it->_type;
        it++;
      }
    }
    else
    {
      const std::vector<ParamDefinition> &params = getAlgo_OutParams(algo_name);
      auto it = params.begin();
      while (it != params.end())
      {
        if (it->_name.compare(paramName) == 0)
          return it->_type;
        it++;
      }
    }
    return typeError;
  }

  std::string ProcessManager::getParamHelp(std::string algo_name, std::string paramName, bool input) const
  {
    if (input)
    {
      const std::vector<ParamDefinition> &params = getAlgo_InParams(algo_name);
      auto it = params.begin();
      while (it != params.end())
      {
        if (it->_name.compare(paramName) == 0)
          return it->_helper;
        it++;
      }
    }
    else
    {
      const std::vector<ParamDefinition> &params = getAlgo_OutParams(algo_name);
      auto it = params.begin();
      while (it != params.end())
      {
        if (it->_name.compare(paramName) == 0)
          return it->_helper;
        it++;
      }
    }
    return "";
  }
}