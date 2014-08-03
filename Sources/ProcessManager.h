#ifndef _PROCESS_MANAGER_HEADER_
#define _PROCESS_MANAGER_HEADER_

#include <boost/function.hpp>
#include <boost/functional/factory.hpp>

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

    void addNewAlgo(Algo_factory& factory, AlgoType type,  std::string name);

    Block* createAlgoInstance(std::string algo_name);
  };
}

#endif