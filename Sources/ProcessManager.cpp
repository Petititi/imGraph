#include "ProcessManager.h"
#include "blocks/Block.h"
#include <vector>

using namespace lsis_org;
using boost::recursive_mutex;
using boost::lock_guard;
using std::vector;

namespace charliesoft
{
  std::map< AlgoType, std::vector<std::string> > ProcessManager::listOfAlgorithms_;
  std::map< std::string, std::vector<ParamDefinition> > ProcessManager::algorithmInParams_;
  std::map< std::string, std::vector<ParamDefinition> > ProcessManager::algorithmOutParams_;
  std::map< std::string, Algo_factory > ProcessManager::algo_factory_;

  ProcessManager* ProcessManager::ptr_ = NULL;
  recursive_mutex ProcessManager::_listBlockMutex;

  /*
  vector<cv::String> algorithms;
  Algorithm::getList(algorithms);
  cout << "Algorithms: " << algorithms.size() << endl;
  for (size_t i = 0; i < algorithms.size(); i++)
  cout << algorithms[i] << endl;
  system("pause");
  */

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

  Block* ProcessManager::createAlgoInstance(std::string algo_name)
  {
    lock_guard<recursive_mutex> guard(_listBlockMutex);
    Block* b = algo_factory_[algo_name]();
    b->initParameters(algorithmInParams_[algo_name], algorithmOutParams_[algo_name]);
    return b;
  }

  std::vector<std::string>& ProcessManager::getAlgos(AlgoType type)
  {
    return listOfAlgorithms_[type];
  }

  std::vector<ParamDefinition>& ProcessManager::getAlgo_InParams(std::string name)
  {
    return algorithmInParams_[name];
  }
  std::vector<ParamDefinition>& ProcessManager::getAlgo_OutParams(std::string name)
  {
    return algorithmOutParams_[name];
  }

  ParamType ProcessManager::getParamType(std::string algo_name, std::string paramName)
  {
    std::vector<ParamDefinition> &params = getAlgo_InParams(algo_name);
    auto it = params.begin();
    while (it != params.end())
    {
      if (it->name_.compare(paramName) == 0)
        return it->type_;
      it++;
    }
    params = getAlgo_OutParams(algo_name);
    it = params.begin();
    while (it != params.end())
    {
      if (it->name_.compare(paramName) == 0)
        return it->type_;
      it++;
    }
    return typeError;
  }
}