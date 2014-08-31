#include "ProcessManager.h"
#include "blocks/Block.h"
#include <vector>

using namespace lsis_org;
using boost::recursive_mutex;
using boost::lock_guard;
using std::vector;

namespace charliesoft
{
  ProcessManager* ProcessManager::ptr_ = NULL;
  recursive_mutex ProcessManager::_listBlockMutex;

  ProcessManager::ProcessManager(){
    ptr_ = this;
  };

  ProcessManager* ProcessManager::getInstance()
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    if (ptr_ == NULL)
      ptr_ = new ProcessManager();
    return ptr_;
  }

  void ProcessManager::releaseInstance()
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    if (ptr_ != NULL)
      delete ptr_;
    ptr_ = NULL;
  }

  Block* ProcessManager::createAlgoInstance(std::string algo_name) const
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    Block* b = algo_factory_.at(algo_name)();
    b->initParameters(algorithmInParams_.at(algo_name), algorithmOutParams_.at(algo_name));
    return b;
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
        if (it->name_.compare(paramName) == 0)
          return it->type_;
        it++;
      }
    }
    else
    {
      const std::vector<ParamDefinition> &params = getAlgo_OutParams(algo_name);
      auto it = params.begin();
      while (it != params.end())
      {
        if (it->name_.compare(paramName) == 0)
          return it->type_;
        it++;
      }
    }
    return typeError;
  }
}