#ifndef _PROCESS_MANAGER_HEADER_
#define _PROCESS_MANAGER_HEADER_

#include <boost/function.hpp>
#include <boost/functional/factory.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <iostream>
#include "OpenCV_filter.h"


namespace charliesoft
{
  class Block;

  enum AlgoType
  {
    input, imgProcess, signalProcess, mathOperator, output
  };

  typedef boost::function< Block*() > Algo_factory;

  class ProcessManager
  {
    static std::map< AlgoType, std::vector<std::string> > listOfAlgorithms_;
    static std::map< std::string, Algo_factory > algo_factory_;
    static ProcessManager* ptr_;

    ProcessManager();
    ~ProcessManager(){};
  public:
    static ProcessManager* getInstance();
    static void releaseInstance();
    static boost::recursive_mutex _listBlockMutex;

    template<typename T>
    bool addNewAlgo(AlgoType type, std::string name)
    {
      boost::lock_guard<boost::recursive_mutex> guard(_listBlockMutex);
      algo_factory_[name] = boost::factory<T*>();
      listOfAlgorithms_[type].push_back(name);
      return true;
    }

    Block* createAlgoInstance(std::string algo_name);
  };
}

#endif