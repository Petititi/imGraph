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
#include "ParamValidator.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace lsis_org;
using std::string;
using std::map;
using std::vector;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  unsigned int GraphOfProcess::current_timestamp_ = 0;

  Block::Block(std::string name){
    name_ = name;
    isUpToDate_ = false;
  };

  void Block::operator()()
  {
    boost::unique_lock<boost::mutex> lock(mtx_);
    unsigned int current_timestamp = GraphOfProcess::current_timestamp_;
    //are parameters ok?
    bool parametersOK = false;
    while (!parametersOK)
    {
      parametersOK = true;
      for (auto it = myInputs_.begin(); it != myInputs_.end(); it++)
      {
        if (it->second.isLinked() &&
          it->second.getTimestamp() < current_timestamp)
        {
          parametersOK = false;
          //not OK! we must wait here until producer update value!
          it->second.waitForUpdate(lock);
        }
      }
    }

    //now we can run process:
    run();
  }

  void Block::notifySchedulerNewData()
  {

  }

  bool Block::isUpToDate()
  {
    return isUpToDate_;
  }

  bool Block::isReadyToRun()
  {
    error_msg_ = "";
    for (auto it = myInputs_.begin(); it != myInputs_.end(); it++)
    {
      try
      {
        it->second.validate(it->second);
      }
      catch (ErrorValidator& e)
      {
        error_msg_ += e.errorMsg + "<br/>";
      }
    }
    return error_msg_.empty();
  }

  bool Block::validateParams(std::string param, const ParamValue val){
    try
    {
      myInputs_[param].validate(val);
    }
    catch (ErrorValidator& e)
    {
      error_msg_ += e.errorMsg + "<br/>";
      return false;
    }
    return true;
  }

  std::string Block::getErrorMsg() {
    std::string tmp = error_msg_;
    error_msg_ = "";
    return tmp;
  }

  void Block::initParameters(std::vector<ParamDefinition>& inParam,
    std::vector<ParamDefinition>& outParam)
  {
    //add empty parameters:
    auto it = inParam.begin();
    while (it != inParam.end())
    {
      myInputs_[it->name_] = ParamValue(this, it->name_, false);
      it++;
    }
    it = outParam.begin();
    while (it != outParam.end())
    {
      myOutputs_[it->name_] = ParamValue(this, it->name_, true);
      it++;
    }
  }

  void Block::setParam(std::string nameParam_, ParamValue& value){
    if (myInputs_.find(nameParam_) != myInputs_.end())
      myInputs_[nameParam_] = &value;
    else
      myOutputs_[nameParam_] = value;
  };
  ParamValue* Block::getParam(std::string nameParam_){
    if (myInputs_.find(nameParam_) != myInputs_.end())
      return &myInputs_[nameParam_];
    else
      return &myOutputs_[nameParam_];
  };

  std::vector<BlockLink> Block::getInEdges()
  {
    vector<BlockLink> out;
    auto it = myInputs_.begin();
    while (it != myInputs_.end())
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
    tree.put("name", name_);

    for (auto it = myInputs_.begin();
      it != myInputs_.end(); it++)
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

    for (auto it = myOutputs_.begin();
      it != myOutputs_.end(); it++)
    {
      ptree paramTree;
      paramTree.put("Name", it->first);
      paramTree.put("ID", (unsigned int)&it->second);

      tree.add_child("Output", paramTree);
    }
    return tree;
  };

  GraphOfProcess::GraphOfProcess(){
  };

  void GraphOfProcess::addNewProcess(Block* block){
    vertices_.push_back(block);
  };

  void GraphOfProcess::deleteProcess(Block* process){
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
    {
      if (*it == process)
      {
        vertices_.erase(it);
        return;
      }
    }
  };

  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return vertices_;
  }

  void Block::createLink(std::string paramName, Block* dest, std::string paramNameDest)
  {
    dest->setParam(paramNameDest, myOutputs_[paramName]);
  }

  bool GraphOfProcess::run(Block* endingVertex)
  {
    current_timestamp_++;
    if (endingVertex != NULL)
      return endingVertex->run();
    bool res = true;
    for (auto it = vertices_.begin();
      it != vertices_.end(); it++)
      boost::thread(boost::ref(**it));

    return res;
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
          Block* tmp = ProcessManager::getInstance()->createAlgoInstance(name);
          addNewProcess(tmp);
          for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
          {
            if (it1->first.compare("Input") == 0)
            {
              string nameIn = it1->second.get("Name", "Error");
              bool link = it1->second.get("Link", false);
              string val = it1->second.get("Value", "Not initialized...");
              ParamValue* tmpVal = tmp->getParam(nameIn);
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
              ParamValue* tmpVal = tmp->getParam(nameOut);
              addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
            }
          }
        }
      }
    }
    //now make links:
    for (auto valToUpdate : toUpdate)
    {
      (*valToUpdate.first) = addressesMap[valToUpdate.second];
    }
  }
}