
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "SubBlock.h"
#include "blocks/ParamValidator.h"

using namespace charliesoft;
using std::vector;
using std::string;
using namespace cv;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  bool SubBlock::addedToList = 
    charliesoft::ProcessManager::getInstance()->addNewAlgo<SubBlock>(input, "SUBBLOCK__");

  SubBlock::SubBlock() :SubBlock("SUBBLOCK__"){
  };

  SubBlock::SubBlock(string name) :Block(name, false, synchrone, true){
    _subGraph = new GraphOfProcess();
  };

  void SubBlock::init(){
    //first set input values:
    for (auto link : externBlocksInput)
    {
      try
      {
        ParamValue& myVal = _myInputs[link._fromParam];
        ParamValue* distVal = link._to->getParam(link._toParam, true);
        distVal->setNew(false);
      }
      catch (...)
      {
        //nothing to do...
      }
    }
    //start graph:
    _subGraph->run(false, false);

  };
  void SubBlock::release(){
    //stop graph:
    _subGraph->stop(false);
  };


  bool SubBlock::run(bool oneShot){
    boost::unique_lock<boost::mutex> lock(_mtx_1);

    //first set input values:
    for (auto link : externBlocksInput)
    {
      try
      {
        ParamValue& myVal = _myInputs[link._fromParam];
        ParamValue* distVal = link._to->getParam(link._toParam, true);
        if (!myVal.isDefaultValue() && myVal.isNew())
          distVal->setValue(&myVal);
        else
          distVal->setNew(false);
      }
      catch (...)
      {
      	//nothing to do...
      }
    }
    //free waiting threads:
    _wait_param_update.notify_all();

    setState(Block::waitingChild);

    //wait for every child updates:
    _subGraph->shouldWaitConsumers();

    bool isFullyProduced = true;
    for (auto link : externBlocksOutput)
    {
      Block *fromParam = link._from;
      ParamValue* value = fromParam->getParam(link._fromParam, false);

      isFullyProduced = isFullyProduced && !(fromParam->getState() == consumingParams);
      _myOutputs[link._toParam].setValue(value);
    }

    for (auto link : externBlocksOutput)
      link._from->getParam(link._fromParam, false)->setNew(false);

    if (isFullyProduced)
      paramsFullyProcessed();
    return true;
  };

  void SubBlock::removeExternLink(const BlockLink& link)
  {
    auto it1 = externBlocksInput.begin();
    while (it1 != externBlocksInput.end())
    {
      if ((*it1) == link)
      {
        externBlocksInput.erase(it1);
        it1 = externBlocksInput.begin();//start back at the begining
      }else
      it1++;
    }
    it1 = externBlocksOutput.begin();
    while (it1 != externBlocksOutput.end())
    {
      if ((*it1) == link)
      {
        externBlocksOutput.erase(it1);
        it1 = externBlocksOutput.begin();//start back at the begining
      }
      else
      it1++;
    }
  }

  void SubBlock::addExternLink(const BlockLink& link, bool isInput)
  {
    if (isInput)
      externBlocksInput.push_back(link);
    else
      externBlocksOutput.push_back(link);
  }
  
  void SubBlock::waitUpdateParams(boost::unique_lock<boost::mutex>& lock)
  {
    _wait_param_update.wait(lock);
  }

  void SubBlock::initFromXML(boost::property_tree::ptree* block,
    std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
    std::map<unsigned int, ParamValue*>& addressesMap,
    std::vector<ConditionOfRendering*>& condToUpdate)
  {
    Block::initFromXML(block, toUpdate, addressesMap, condToUpdate);
    for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
    {
      if (it1->first.compare("SubGraph") == 0)
        _subGraph->fromGraph(it1->second, addressesMap);

      if (it1->first.compare("InputLink") == 0)
      {
        string fromParam = it1->second.get("FromParam", "_");
        string val = it1->second.get("ToParam", "0");
        ParamValue* dist = addressesMap[lexical_cast<unsigned int>(val)];
        ParamValue& local = _myInputs[fromParam];
        addExternLink(BlockLink(local.getBlock(), dist->getBlock(),
          local.getName(), dist->getName()), true);
      }
      if (it1->first.compare("OutputLink") == 0)
      {
        string val = it1->second.get("FromParam", "0");
        string fromParam = it1->second.get("ToParam", "_");
        ParamValue* dist = addressesMap[lexical_cast<unsigned int>(val)];
        ParamValue& local = _myOutputs[fromParam];
        addExternLink(BlockLink(dist->getBlock(), local.getBlock(),
          dist->getName(), local.getName()), false);
      }
    }
  }

  boost::property_tree::ptree SubBlock::getXML()
  {
    //before storing into XML, remove input modified by subparam:
    ParamValue fakeValue = Not_A_Value();
    for (auto link : externBlocksInput)
    {
      try
      {
        link._to->getParam(link._toParam, true)->setValue(&fakeValue);
      }
      catch (...)
      {
        //nothing to do...
      }
    }


    ptree tree = Block::getXML();
    
    //create also the subgraph xml:
    ptree subgraphTree;
    _subGraph->saveGraph(subgraphTree);
    tree.add_child("SubGraph", subgraphTree);

    //and also the links:
    for (auto it = externBlocksInput.begin();
      it != externBlocksInput.end(); it++)
    {
      ptree paramTree;
      paramTree.put("FromParam", it->_fromParam);
      paramTree.put("ToParam", (unsigned int)it->_to->getParam(it->_toParam, true));

      tree.add_child("InputLink", paramTree);
    }
    for (auto it = externBlocksOutput.begin();
      it != externBlocksOutput.end(); it++)
    {
      ptree paramTree;
      paramTree.put("FromParam", (unsigned int)it->_from->getParam(it->_fromParam, false));
      paramTree.put("ToParam", it->_toParam);

      tree.add_child("OutputLink", paramTree);
    }
    return tree;
  };

};