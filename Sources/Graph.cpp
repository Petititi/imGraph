
#include "Graph.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "ProcessManager.h"
#include "blocks/ParamValidator.h"
#include "SubBlock.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  bool GraphOfProcess::_pauseProcess = false;

  GraphOfProcess::GraphOfProcess()
  {
    _pauseProcess = false;
    _parent = NULL;
    _subBlock = NULL;
  };

  GraphOfProcess::~GraphOfProcess()
  {
    stop();
    waitUntilEnd(5000);//15s
    for (size_t i = 0; i < _vertices.size(); i++)
    {
      Block* b = _vertices[i];
      delete b;
    }
    _vertices.clear();
  }

  void GraphOfProcess::addNewProcess(Block* block){
    _vertices.push_back(block);
    block->setGraph(this);
    //if graph is running, start it:
    if (!_runningThread.empty())
    {
      block->markAsUnprocessed();
      _runningThread[block] = boost::thread(boost::ref(*block));
    }
  };

  void GraphOfProcess::deleteProcess(Block* process){
    extractProcess(process);//remove the process from the graph
    delete process;//free memory
  };

  void GraphOfProcess::extractProcess(Block* process){
    //remove every links:
    for (auto& outParam : process->_myOutputs)
      outParam.second = Not_A_Value();
    for (auto& inParam : process->_myInputs)
      inParam.second = Not_A_Value();

    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      if (*it == process)
      {
        _vertices.erase(it);

        for (auto& waitThread : _waitingForRendering)
        {
          if (waitThread.first != process)
            waitThread.second.erase(process);
        }

        auto& waitThreadDelete = _waitingForRendering.find(process);
        if (waitThreadDelete != _waitingForRendering.end())
        {
          waitThreadDelete->second.clear();
          _waitingForRendering.erase(waitThreadDelete);
        }

        auto& it = _runningThread.find(process);
        if (it != _runningThread.end())
        {
          it->second.interrupt();
          it->second.join();//wait for the end...
          _runningThread.erase(it);
        }

        return;
      }
    }
  };

  void GraphOfProcess::removeLink(const BlockLink& l)
  {
    *(l._to->getParam(l._toParam, true)) = Not_A_Value();
  }

  void GraphOfProcess::createLink(Block* src, std::string paramName, Block* dest, std::string paramNameDest)
  {
    src->linkParam(paramName, dest, paramNameDest);
  }

  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return _vertices;
  }

  void GraphOfProcess::updateAncestors(Block* process)
  {
    //wait for every direct linked ancestors:
    //Is there old parameters? -> if yes, we wait for update, else we process the block right now:
    for (auto it = process->_myInputs.begin(); it != process->_myInputs.end(); it++)
    {
      ParamValue* linkedBlock = it->second.get<ParamValue*>();
      if (linkedBlock!=NULL)
      {
        //we have an ancestor! We ask for an update:
        linkedBlock->update();
      }
    }
  }

  void GraphOfProcess::shouldWaitAncestors(Block* process)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    //should we wait for a process? (depending on the block type)
    if (process->getTypeExec() == Block::asynchrone)
      return;//no need to wait
    //wait for every direct linked ancestors:
    //Is there old parameters? -> if yes, we wait for update, else we process the block right now:
    for (auto it = process->_myInputs.begin(); it != process->_myInputs.end(); it++)
    {
      if (it->second.isLinked())
      {
        ParamValue* other = it->second.get<ParamValue*>();
        while (!it->second.isNew())
        {//we have to wait for any update!
          //std::cout << "---  " << _STR(process->getName()) << " -> Wait " << _STR(it->second.getName()) << endl;
          process->setState(Block::waitingChild);
          other->getBlock()->waitProducers(lock);//wait for parameter update!
          //std::cout << "---  " << _STR(process->getName()) << " <- unblock " << _STR(it->second.getName()) << endl;
        }
      }
    }//ok, every ancestor have produced a value!
    if (NULL != _subBlock)
    {
      for (auto link : _subBlock->externBlocksInput)
      {
        try
        {
          if (link._to == process)
          {
            ParamValue* distVal = link._to->getParam(link._toParam, true);
            if (distVal->isDefaultValue() || !distVal->isNew())
            {
              //std::cout << "---  subBlock_" << _STR(link._to->getName()) << " -> Wait " << _STR(link._fromParam) << endl;
              process->setState(Block::waitingChild);
              _subBlock->waitUpdateParams(lock);//wait for parameter update!
              //std::cout << "---  subBlock_" << _STR(link._to->getName()) << " <- unblock " << _STR(link._fromParam) << endl;
            }
          }
        }
        catch (...)
        {
          //nothing to do...
        }
      }
    }
  }

  void GraphOfProcess::clearWaitingList(Block* process)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    _waitingForRendering[process].clear();
    cout << "___clearWaitingList___" << endl;
  }

  void GraphOfProcess::shouldWaitConsumers(Block* process)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    //should we wait for a process? (depending on the block type)
    if (process!=NULL && process->getTypeExec() == Block::synchrone)
    {
      //wait for the childs of this process. They have to be fully rendered before we rerun this block!
      for (auto it = process->_myOutputs.begin(); it != process->_myOutputs.end(); it++)
      {
        //wait for threads consumption
        std::set<ParamValue*>& listeners = it->second.getListeners();
        for (auto listener : listeners)
        {
          if (listener->isLinked() && listener->isNew())//this param is still not consumed...
          {
            _waitingForRendering[process].insert(listener->getBlock());
          }
        }
      }
    }
    if (process == NULL && _subBlock != NULL)
    {
      process = _subBlock;
      for (auto& threads : _runningThread)
      {
        if (threads.first->getState() != Block::waitingChild &&
          threads.first->getState() != Block::waitingConsumers &&
          threads.first->getState() != Block::stopped)
          _waitingForRendering[process].insert(threads.first);
      }
    }
    if (process != NULL && !_waitingForRendering[process].empty())
    {
      process->setState(Block::waitingConsumers);
      process->waitConsumers(lock);//wait for parameter update! Will be waked up when _waitingForRendering is empty!
    }

  }


  void GraphOfProcess::blockProduced(Block* process, bool fullyRendered)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);/*
    if (fullyRendered)
      std::cout << " \t\t\t  " << _STR(process->getName()) << " Produced!" << endl;
    else
      std::cout << " \t\t\t  " << _STR(process->getName()) << " partially rendered!" << endl;*/
    //wake up linked output blocks
    process->notifyProduction();

    if (fullyRendered)
    {
      //remove this block for every waiting thread:
      for (auto& waitThread : _waitingForRendering)
      {
        if (waitThread.first != process)
        {
          waitThread.second.erase(process);
          if (waitThread.second.empty())
            waitThread.first->wakeUpFromConsumers();
        }
      }
    }
  }

  void GraphOfProcess::stop(bool delegateParent, bool waitEnd, bool stopAll)
  {
    if (_parent != NULL && delegateParent)
      return _parent->stop(delegateParent, waitEnd);
    if (stopAll)
    {
      for (auto& it = _runningThread.begin(); it != _runningThread.end(); it++)
        it->second.interrupt();
    }

    if (waitEnd)
      waitUntilEnd(15000);//15s
  }

  void GraphOfProcess::waitUntilEnd(size_t max_ms_time)
  {
    auto _time_start = boost::posix_time::microsec_clock::local_time();
    for (auto& it = _runningThread.begin(); it != _runningThread.end(); it++)
    {
      if (it->first->getState() != Block::stopped && it->second.joinable())
      {
        if (max_ms_time == 0)
          it->second.join();
        else
        {
          if (!it->second.timed_join(boost::posix_time::milliseconds(max_ms_time)))
            return;//quit because we can't stop the process

          boost::posix_time::ptime time_end(boost::posix_time::microsec_clock::local_time());
          boost::posix_time::time_duration duration(time_end - _time_start);
          if (duration.total_milliseconds() >= max_ms_time)
            return;
          max_ms_time -= static_cast<size_t>(duration.total_milliseconds());
        }
      }
    }
    _runningThread.clear();
  }

  bool GraphOfProcess::run(bool singleShot, bool delegateParent)
  {
    if (_parent != NULL && delegateParent)
      return _parent->run(singleShot);
    stop(delegateParent);//just in case...
    _pauseProcess = false;
    bool res = true;
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      (*it)->markAsUnprocessed();
      (*it)->setExecuteOnlyOnce(singleShot);
      _runningThread[*it] = boost::thread(boost::ref(**it));
    }

    return res;
  }

  bool GraphOfProcess::switchPause(bool delegateParent)
  {
    if (_parent != NULL && delegateParent)
      return _parent->switchPause();
    _pauseProcess = !_pauseProcess;
    if (!_pauseProcess)
    {
      //wake up threads:
      for (auto it = _vertices.begin();
        it != _vertices.end(); it++)
        (*it)->wakeUpFromPause();
    }
    return _pauseProcess;
  }

  void GraphOfProcess::saveGraph(boost::property_tree::ptree& tree) const
  {
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
      tree.add_child("GraphOfProcess.Block", (*it)->getXML());
  }

  bool GraphOfProcess::initChildDatas(Block* block, std::set<Block*>& listOfRenderedBlocks)
  {
    if (listOfRenderedBlocks.find(block) != listOfRenderedBlocks.end())
      return false;//nothing to do...

    if (block->getState() != Block::stopped && block->getState() != Block::paused)
      return true;//block is running, the value will be updated automatically!

    listOfRenderedBlocks.insert(block);
    //take one shot of block to init output:
    try
    {
      block->init();
      block->run(true);
      block->release();

      //now render every childs.
      set<Block*> renderBlocks;
      for (auto it = block->_myOutputs.begin(); it != block->_myOutputs.end(); it++)
      {
        //wake up the threads, if any!
        std::set<ParamValue*>& listeners = it->second.getListeners();
        for (auto listener : listeners)
        {
          if (listener->isLinked())
          {
            Block* consumer = listener->getBlock();
            if (consumer != NULL && consumer->isReadyToRun(true))
              renderBlocks.insert(consumer);
          }
        }

      }
      for (Block* consumer : renderBlocks)
      {
        if (listOfRenderedBlocks.find(consumer) == listOfRenderedBlocks.end())
          initChildDatas(consumer, listOfRenderedBlocks);
      }
    }
    catch (cv::Exception& e)
    {
      block->addErrorMsg(e.what());
      std::cout << "exception caught: " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  void GraphOfProcess::fromGraph(boost::property_tree::ptree& tree,
    std::map<unsigned int, ParamValue*>& addressesMap)
  {
    boost::optional<ptree&> vertices = tree.get_child_optional("GraphOfProcess");

    vector < std::pair<ParamValue*, unsigned int> > toUpdate;
    vector<ConditionOfRendering*> condToUpdate;

    if (vertices)
    {
      for (ptree::iterator it = vertices->begin(); it != vertices->end(); it++)
      {
        if (it->first.compare("Block") == 0)
        {
          ptree *block = &it->second;
          string name = block->get("name", "Error");
          Block* tmp = ProcessManager::getInstance()->createAlgoInstance(name);
          if (tmp != NULL)
          {
            addNewProcess(tmp);
            tmp->initFromXML(block, toUpdate, addressesMap, condToUpdate);
          }
        }
      }
      //now make links:
      for (auto valToUpdate : toUpdate)
      {
        Block* fromBlock = valToUpdate.first != NULL ? valToUpdate.first->getBlock() : NULL;
        ParamValue* secondVal = addressesMap[valToUpdate.second];
        Block* toBlock = secondVal != NULL ? secondVal->getBlock() : NULL;
        if (fromBlock != NULL && toBlock != NULL)
        {
          try
          {
            createLink(toBlock, secondVal->getName(), fromBlock, valToUpdate.first->getName());
          }
          catch (ErrorValidator& e)
          {//algo doesn't accept this value!
            //QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
            std::cout << e.errorMsg << std::endl;
          }
        }
      }
      //and set correct values of conditions:
      for (auto cond : condToUpdate)
      {
        if (cond->getCategory_left() == 1)//output of block:
        {
          unsigned int addr = static_cast<unsigned int>(cond->getOpt__valueleft().get<double>() + 0.5);
          cond->setValue(true, addressesMap[addr]);
        }
        if (cond->getCategory_right() == 1)//output of block:
        {
          unsigned int addr = static_cast<unsigned int>(cond->getOpt__valueright().get<double>() + 0.5);
          cond->setValue(false, addressesMap[addr]);
        }
      }

      //first look for edges who are ready to run:
      set<Block*> renderedBlocks;
      for (auto it = _vertices.begin();
        it != _vertices.end(); it++)
      {
        if ((*it)->isReadyToRun(true))
          initChildDatas(*it, renderedBlocks);//and init him and output childs
      }
    }
  }

}