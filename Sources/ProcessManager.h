#ifndef _PROCESS_MANAGER_HEADER_
#define _PROCESS_MANAGER_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/function.hpp>
#include <boost/functional/factory.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "OpenCV_filter.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <iostream>
#include "blocks/ParamValue.h"

namespace charliesoft
{
  class Block;
  struct ParamDefinition;

  enum AlgoType
  {
    input, imgProcess, signalProcess, mathOperator, output
  };

  typedef boost::function< Block*() > Algo_factory;

#define _PROCESS_MANAGER ProcessManager::getInstance()

  class ProcessManager
  {
    std::map< AlgoType, std::vector<std::string> > listOfAlgorithms_;
    std::map< std::string, std::vector<ParamDefinition> > algorithmInParams_;
    std::map< std::string, std::vector<ParamDefinition> > algorithmOutParams_;
    std::map< std::string, Algo_factory > algo_factory_;
    static ProcessManager* ptr_;

    ProcessManager();
    ~ProcessManager(){};
  public:
    static ProcessManager* getInstance();
    static void releaseInstance();
    static boost::recursive_mutex _listBlockMutex;

    std::vector<std::string>& getAlgos(AlgoType type);
    std::vector<ParamDefinition>& getAlgo_InParams(std::string name);
    std::vector<ParamDefinition>& getAlgo_OutParams(std::string name);

    template<typename T>
    bool addNewAlgo(AlgoType type, std::string name)
    {
      boost::lock_guard<boost::recursive_mutex> guard(_listBlockMutex);
      algo_factory_[name] = boost::factory<T*>();
      listOfAlgorithms_[type].push_back(name);
      algorithmInParams_[name] = T::getListParams();
      algorithmOutParams_[name] = T::getListOutputs();
      return true;
    }

    Block* createAlgoInstance(std::string algo_name);
    ParamType getParamType(std::string algo_name, std::string paramName);
  };
}

#endif