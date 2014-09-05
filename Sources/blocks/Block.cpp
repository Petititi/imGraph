#include "Block.h"
#include <vector>
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/thread/thread.hpp>
#include <boost/parameter.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/lock_guard.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif
#include "ParamValidator.h"

using namespace lsis_org;
using std::string;
using std::map;
using std::vector;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  std::string typeName(ParamType type)
  {
    switch (type)
    {
    case Boolean:
      return "Boolean";
    case Int:
      return "Int";
    case Float:
      return "Float";
    case Vector:
      return "Vector";
    case Mat:
      return "Mat";
    case String:
      return "String";
    case FilePath:
      return "FilePath";
    default:
      return "typeError";
    }
  }

  unsigned int GraphOfProcess::_current_timestamp = 0;
  bool GraphOfProcess::pauseProcess = false;
  boost::mutex _mtx_synchro;

  Block::Block(std::string name){
    _name = name;
    _timestamp = 0;
    _fullyRendered = true;
  };

  void Block::operator()()
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    try
    {
      while (true)//this will stop when user stop the process...
      {
        while (GraphOfProcess::pauseProcess)
          _cond.wait(lock);//wait for any parameter update...

        _work_timestamp = GraphOfProcess::_current_timestamp;
        if (_timestamp < _work_timestamp)
        {
          //are parameters ok?
          bool parametersOK = false;
          while (!parametersOK)
          {
            parametersOK = true;
            for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
            {
              if (it->second.isLinked() &&
                it->second.getTimestamp() < _work_timestamp)
              {
                parametersOK = false;
                //not OK! we must wait here until producer update value!
                it->second.waitForUpdate(lock);
              }
            }
          }

          //now we can run the process:
          run();
          _cond.wait(lock);//wait for any parameter update...
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(100.));
      }
    }
    catch (boost::thread_interrupted const&)
    {
      //end of thread (requested by interrupt())!
    }
  }


  bool Block::isAncestor(Block* other)
  {
    //add empty parameters:
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
      {
        ParamValue* otherParam = it->second.get<ParamValue*>();
        if (otherParam->getBlock() == other)
          return true;
        if (otherParam->getBlock()->isAncestor(other))
          return true;
      }
      it++;
    }
    return false;
  }

  bool Block::isStartingBlock()
  {
    if (ProcessManager::getInstance()->getAlgoType(_name) != input)
      return false;
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
        return false;
      it++;
    }
    return true;
  }
  void Block::renderingDone(bool fullyRendered)
  {
    _fullyRendered = fullyRendered;
    {
      boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
      _timestamp = _work_timestamp;
      if (_fullyRendered)
        _cond_sync.notify_all();//wake up waiting thread (if needed)

      while (GraphOfProcess::pauseProcess)
        _cond.wait(guard);//wait for any parameter update...
    }
    //test if we need to wait!
    _processes->synchronizeTimestamp(this);
    if (!_fullyRendered)
      _processes->_current_timestamp++;//next frame?
    _work_timestamp = _processes->_current_timestamp;
  }

  void Block::wakeUp()
  {
    _cond.notify_all();//wake up waiting thread (if needed)
  }
  void Block::waitUpdate(boost::unique_lock<boost::mutex>& lock)
  {
    _cond_sync.wait(lock);
  }

  bool Block::isReadyToRun()
  {
    _error_msg = "";
    for (auto it = _myInputs.begin(); it != _myInputs.end(); it++)
    {
      try
      {
        if (it->second.isLinked())
        {
          ParamValue* other = it->second.get<ParamValue*>();
          if (!other->getBlock()->isReadyToRun())
            throw ErrorValidator(other->getBlock()->_error_msg);
        }
        else
          it->second.validate(it->second);
      }
      catch (ErrorValidator& e)
      {
        _error_msg += e.errorMsg + "<br/>";
      }
    }
    return _error_msg.empty();
  }

  bool Block::validTimestampOrWait(unsigned int timestamp)
  {
    bool waiting = false;
    boost::unique_lock<boost::mutex> guard(_mtx_timestamp_inc);
    while (!_fullyRendered || timestamp > _timestamp)
    {
      waiting = true;
      waitUpdate(guard);
    }
    return !waiting;
  }

  bool Block::validateParams(std::string param, const ParamValue val){
    try
    {
      _myInputs[param].validate(val);
    }
    catch (ErrorValidator& e)
    {
      _error_msg += e.errorMsg + "<br/>";
      return false;
    }
    return true;
  }

  std::string Block::getErrorMsg() {
    std::string tmp = _error_msg;
    _error_msg = "";
    return tmp;
  }

  void Block::initParameters(const std::vector<ParamDefinition>& inParam,
    const std::vector<ParamDefinition>& outParam)
  {
    //add empty parameters:
    auto it = inParam.begin();
    while (it != inParam.end())
    {
      _myInputs[it->_name] = ParamValue(this, it->_name, false);
      it++;
    }
    it = outParam.begin();
    while (it != outParam.end())
    {
      _myOutputs[it->_name] = ParamValue(this, it->_name, true);
      it++;
    }
  }

  void Block::setParam(std::string nameParam_, ParamValue& value){
    if (_myInputs.find(nameParam_) != _myInputs.end())
      _myInputs[nameParam_] = &value;
    else
      _myOutputs[nameParam_] = value;
  };
  ParamValue* Block::getParam(std::string nameParam_, bool input){
    if (input && _myInputs.find(nameParam_) != _myInputs.end())
      return &_myInputs[nameParam_];
    else
      if (!input && _myOutputs.find(nameParam_) != _myOutputs.end())
        return &_myOutputs[nameParam_];
      
    return NULL;
  };

  std::vector<BlockLink> Block::getInEdges()
  {
    vector<BlockLink> out;
    auto it = _myInputs.begin();
    while (it != _myInputs.end())
    {
      if (it->second.isLinked())
        out.push_back(BlockLink(it->second.toBlockLink()));
      it++;
    }
    return out;
  }

  boost::property_tree::ptree Block::getXML() const
  {
    ptree tree;
    tree.put("name", _name);
    tree.put("position", _position);

    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("Link", it->second.isLinked());
      if (!it->second.isLinked())
        paramTree.put("Value", it->second.toString());
      else
        paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>());

      tree.add_child("Input", paramTree);
    }

    for (auto it = _myOutputs.begin();
      it != _myOutputs.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("ID", (unsigned int)&it->second);

      tree.add_child("Output", paramTree);
    }
    return tree;
  };

  void Block::createLink(std::string paramName, Block* dest, std::string paramNameDest)
  {
    ParamValue& valIn = dest->_myInputs[paramNameDest];
    ParamValue& valOut = _myOutputs[paramName];
    //first test type of input:
    if (valIn.getType() != valOut.getType())
    {
      throw (ErrorValidator((my_format(_STR("ERROR_TYPE")) % _STR(getName()) % _STR(valIn.getName()) % typeName(valIn.getType()) %
        _STR(dest->getName()) % _STR(valOut.getName()) % typeName(valOut.getType())).str()));
    }
    dest->setParam(paramNameDest, _myOutputs[paramName]);
  }

  void Block::setPosition(int x, int y)
  {
    _position.x = x;
    _position.y = y;
  }

  GraphOfProcess::GraphOfProcess(){
  };

  void GraphOfProcess::addNewProcess(Block* block){
    vertices_.push_back(block);
    block->setGraph(this);
  };

  void GraphOfProcess::deleteProcess(Block* process){
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
    {
      if (*it == process)
      {
        vertices_.erase(it);
        delete process;
        return;
      }
    }
  };

  void GraphOfProcess::synchronizeTimestamp(Block* processToSynchronize)
  {
    bool isFullyRendered = true;
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
    {
      //each block should have the same timestamp:
      if ((*it)->isAncestor(processToSynchronize))
      {
        while (!(*it)->validTimestampOrWait(processToSynchronize->getTimestamp()))
          isFullyRendered = false;
      }
    }
  }
  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return vertices_;
  }

  void GraphOfProcess::stop()
  {
    for (size_t i = 0; i < runningThread_.size(); i++)
      runningThread_[i].interrupt();
    runningThread_.clear();
  }
  
  bool GraphOfProcess::run()
  {
    _current_timestamp++;
    bool res = true;
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
      runningThread_.push_back(boost::thread(boost::ref(**it)));

    return res;
  }
  void GraphOfProcess::switchPause()
  {
    GraphOfProcess::pauseProcess = !GraphOfProcess::pauseProcess;
    if (!pauseProcess)
    {
      //wake up threads:
      for (auto it = vertices_.begin();
        it != vertices_.end(); it++)
        (*it)->wakeUp();
    }
  }

  void GraphOfProcess::saveGraph(boost::property_tree::ptree& tree) const
  {
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
    {
      tree.add_child("GraphOfProcess.Block", (*it)->getXML());
    }
  }

  void GraphOfProcess::fromGraph(boost::property_tree::ptree& tree)
  {
    boost::optional<ptree&> vertices = tree.get_child_optional("GraphOfProcess");

    map<unsigned int, ParamValue*> addressesMap;
    vector < std::pair<ParamValue*, unsigned int> > toUpdate;

    if (vertices)
    {
      for (ptree::iterator it = vertices->begin(); it != vertices->end(); it++)
      {
        if (it->first.compare("Block") == 0)
        {
          ptree *block = &it->second;
          string name = block->get("name", "Error");
          string pos = block->get("position", "[0,0]");
          int posSepare = pos.find_first_of(',') + 1;
          string xPos = pos.substr(1, posSepare - 2);
          string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);
          Block* tmp = ProcessManager::getInstance()->createAlgoInstance(name);
          addNewProcess(tmp);
          tmp->setPosition(lexical_cast<float>(xPos), lexical_cast<float>(yPos));
          for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
          {
            if (it1->first.compare("Input") == 0)
            {
              string nameIn = it1->second.get("Name", "Error");
              bool link = it1->second.get("Link", false);
              string val = it1->second.get("Value", "Not initialized...");
              ParamValue* tmpVal = tmp->getParam(nameIn, true);
              if (!link)
              {
                try
                {
                  tmpVal->valid_and_set(tmpVal->fromString(tmpVal->getType(), val));
                }
                catch (...)
                {
                }
              }
              else
                toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
            }
            if (it1->first.compare("Output") == 0)
            {
              string nameOut = it1->second.get("Name", "Error");
              string val = it1->second.get("ID", "0");
              ParamValue* tmpVal = tmp->getParam(nameOut, false);
              addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
            }
          }
        }
      }
    }
    //now make links:
    for (auto valToUpdate : toUpdate)
    {
      Block* fromBlock = valToUpdate.first!=NULL?valToUpdate.first->getBlock():NULL;
      ParamValue* secondVal = addressesMap[valToUpdate.second];
      Block* toBlock = secondVal != NULL ? secondVal->getBlock() : NULL;
      if (fromBlock != NULL && toBlock != NULL)
      {
        try
        {
          toBlock->createLink(secondVal->getName(), fromBlock, valToUpdate.first->getName());
        }
        catch (ErrorValidator& e)
        {//algo doesn't accept this value!
          //QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
          std::cout << e.errorMsg << std::endl;
        }
      }
    }
  }
}