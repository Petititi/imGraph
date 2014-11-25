
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
#include "view/Window.h"

#include "ProcessManager.h"
#include "blocks/ParamValidator.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  unsigned int GraphOfProcess::_current_timestamp = 0;
  bool GraphOfProcess::pauseProcess = false;

  GraphOfProcess::GraphOfProcess(){
  };

  GraphOfProcess::~GraphOfProcess()
  {
    for (Block* b : _vertices)
      delete b;
    _vertices.clear();
  }

  void GraphOfProcess::addNewProcess(Block* block){
    _vertices.push_back(block);
    block->setGraph(this);
  };

  void GraphOfProcess::deleteProcess(Block* process){
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      if (*it == process)
      {
        _vertices.erase(it);
        delete process;
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

  void GraphOfProcess::waitForFullRendering(Block* father_process, Block* process)
  {
    _waitingForRendering[father_process].insert(process);
    for (auto it = process->_myOutputs.begin(); it != process->_myOutputs.end(); it++)
    {
      //wait the thread to process it!
      std::set<ParamValue*>& listeners = it->second.getListeners();
      for (auto listener : listeners)
      {
        if (listener->isLinked())
          waitForFullRendering(father_process, listener->getBlock());
      }
    }
  }

  void GraphOfProcess::shouldWaitChild(Block* process)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    //should we wait for a process? (depending on the block type)
    if (process->getTypeExec() == Block::asynchrone)
      return;//no need to wait
    if (process->getTypeExec() == Block::oneShot || process->getTypeExec() == Block::producer)
    {
      //wait for every direct linked ancestors (using timestamp verification):
      //Is there old parameters? -> if yes, we wait for update, else we process the block right now:
      for (auto it = process->_myInputs.begin(); it != process->_myInputs.end(); it++)
      {
        if (it->second.isLinked())
        {
          while (!it->second.isNew())
          {
            //std::cout << "---  " << _STR(process->getName()) << " (" << process->_timestamp << ") Wait " << _STR(it->second.getName()) << endl;
            process->waitUpdateTimestamp(lock);//wait for parameter update!
          }
        }
      }

      return;//ok, every ancestor have produced a value!
    }
  }

  void GraphOfProcess::shouldWaitConsumers(Block* process)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    //should we wait for a process? (depending on the block type)
    if (process->getTypeExec() == Block::producer)
    {
      //wait for every childs. They have to be fully rendered before we rerun this block!
      for (auto it = process->_myOutputs.begin(); it != process->_myOutputs.end(); it++)
      {
        //wake up the threads, if any!
        std::set<ParamValue*>& listeners = it->second.getListeners();
        for (auto listener : listeners)
        {
          if (listener->isLinked())
            waitForFullRendering(process, listener->getBlock());
        }
      }
      if (!_waitingForRendering[process].empty())
        process->waitUpdateTimestamp(lock);//wait for parameter update!
      _current_timestamp++;
      return;//ok, every child have processed our value!
    }
  }

  ///TODO:
  /// Verify that parameter update set timestamp, even if not linked!
  /// But don't change timestamp is value set is the same as previous stored value.
  void GraphOfProcess::blockProduced(Block* process, bool fullyRendered)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    std::cout << "   " << _STR(process->getName()) << " (" << _current_timestamp << ") Produced!" << endl;
    if (fullyRendered)
    {
      //remove this block for every waiting thread:
      for (auto& waitThread : _waitingForRendering)
      {
        waitThread.second.erase(process);
        if (waitThread.second.empty())
          waitThread.first->wakeUp();
      }
      //set timestamp of block to current timestamp:
      process->_timestamp = _current_timestamp;
      //wake up any waiting thread (some thread can wait specifically our rendering):
      process->wakeUp();
    }
    //wake up linked output blocks
    process->wakeUpOutputListeners();
  }

  void GraphOfProcess::stop()
  {
    for (size_t i = 0; i < _runningThread.size(); i++)
    {
      _runningThread[i].interrupt();
      _runningThread[i].join();//wait for the end...
    }
    _runningThread.clear();
  }

  bool GraphOfProcess::run()
  {
    stop();//just in case...
    bool res = true;
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
      _runningThread.push_back(boost::thread(boost::ref(**it)));

    return res;
  }

  void GraphOfProcess::switchPause()
  {
    pauseProcess = !pauseProcess;
    if (!pauseProcess)
    {
      //wake up threads:
      for (auto it = _vertices.begin();
        it != _vertices.end(); it++)
        (*it)->wakeUp();
    }
  }

  void GraphOfProcess::saveGraph(boost::property_tree::ptree& tree) const
  {
    for (auto it = _vertices.begin();
      it != _vertices.end(); it++)
    {
      tree.add_child("GraphOfProcess.Block", (*it)->getXML());
    }
  }

  void GraphOfProcess::initChildDatas(Block* block, std::set<Block*>& listOfRenderedBlocks)
  {
    listOfRenderedBlocks.insert(block);
    //take one shot of block to init output:
    block->run(true);
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

  void GraphOfProcess::fromGraph(boost::property_tree::ptree& tree)
  {
    boost::optional<ptree&> vertices = tree.get_child_optional("GraphOfProcess");

    map<unsigned int, ParamValue*> addressesMap;
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
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_left().get<double>(false) + 0.5);
        cond->setValue(true, addressesMap[addr]);
      }
      if (cond->getCategory_right() == 1)//output of block:
      {
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_right().get<double>(false) + 0.5);
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