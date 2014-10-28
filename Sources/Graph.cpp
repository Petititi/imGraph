
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

  void GraphOfProcess::synchronizeTimestamp(Block* processToSynchronize)
  {
    for (auto it : _vertices)
    {
      //each block should have the same timestamp:
      if (it != processToSynchronize && it->isAncestor(processToSynchronize))
      {
        while (!it->validTimestampOrWait(processToSynchronize));
      }
    }
  }
  std::vector<Block*>& GraphOfProcess::getVertices()
  {
    return _vertices;
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
    _current_timestamp++;
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
          string pos = block->get("position", "[0,0]");
          int posSepare = pos.find_first_of(',') + 1;
          string xPos = pos.substr(1, posSepare - 2);
          string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);
          Block* tmp = ProcessManager::getInstance()->createAlgoInstance(name);
          if (tmp != NULL)
          {
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
                    if (tmpVal != NULL)
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
              if (it1->first.compare("Condition") == 0)
              {
                int cLeft = it1->second.get("category_left", 0);
                int cRight = it1->second.get("category_right", 0);
                int cOperator = it1->second.get("boolean_operator", 0);

                double valLeft = it1->second.get("Value_left", 0.);
                double valRight = it1->second.get("Value_right", 0.);
                tmp->addCondition(ConditionOfRendering(cLeft, valLeft, cRight, valRight, cOperator,
                  tmp));
                if (cLeft == 1 || cRight == 1)//output of block...
                  condToUpdate.push_back(&tmp->getConditions().back());
              }
            }
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
          toBlock->createLink(secondVal->getName(), fromBlock, valToUpdate.first->getName());
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
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_left().get<double>() + 0.5);
        cond->setValue(true, addressesMap[addr]);
      }
      if (cond->getCategory_right() == 1)//output of block:
      {
        unsigned int addr = static_cast<unsigned int>(cond->getOpt_value_right().get<double>() + 0.5);
        cond->setValue(false, addressesMap[addr]);
      }
    }
  }
}