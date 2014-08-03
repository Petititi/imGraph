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
    return algo_factory_[algo_name]();
  }

  std::vector<std::string> ProcessManager::getAlgos(AlgoType type)
  {
    return listOfAlgorithms_[type];
  }
}