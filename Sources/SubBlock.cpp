
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

  SubBlock::SubBlock(string name) :Block(name, false){
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

  ParamValue* SubBlock::addNewInput(ParamDefinition& param)
  {
    _inputParams[param._name] = param;

    ParamValue& t = _myInputs[param._name] = ParamValue(this, param._name, false);
    t.isNeeded(true);//always needed!
    t = param._initVal;
    return &t;
  };

  ParamValue* SubBlock::addNewOutput(ParamDefinition& param)
  {
    _outputParams[param._name] = param;

    ParamValue& t = _myOutputs[param._name] = ParamValue(this, param._name, true);
    t.isNeeded(true);//always needed!
    t = param._initVal;
    return &t;
  };

  vector<ParamDefinition> SubBlock::getInParams() const
  {
    vector<ParamDefinition> tmp = Block::getInParams();
    for (auto it : _inputParams)
      tmp.push_back(it.second);
    return tmp;
  };
  vector<ParamDefinition> SubBlock::getOutParams() const
  {
    vector<ParamDefinition> tmp = Block::getOutParams();
    for (auto it : _outputParams)
      tmp.push_back(it.second);
    return tmp;
  };


  void SubBlock::waitUpdateParams(boost::unique_lock<boost::mutex>& lock)
  {
    _wait_param_update.wait(lock);
  }

  void SubBlock::initFromXML(boost::property_tree::ptree* block,
    std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
    std::map<unsigned int, ParamValue*>& addressesMap,
    std::vector<ConditionOfRendering*>& condToUpdate)
  {
    string pos = block->get("position", "[0.0,0.0]");
    int posSepare = pos.find_first_of(',') + 1;
    string xPos = pos.substr(1, posSepare - 2);
    string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    pos = block->get("size_increment", "[0.0,0.0]");
    posSepare = pos.find_first_of(',') + 1;
    string xInc = pos.substr(1, posSepare - 2);
    string yInc = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    setPosition(lexical_cast<float>(xPos), lexical_cast<float>(yPos),
      lexical_cast<float>(xInc), lexical_cast<float>(yInc));
    for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
    {
      if (it1->first.compare("Input") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        string helper = it1->second.get("Helper", nameIn);
        string val = it1->second.get("Value", "Not initialized...");
        ParamType paramType = static_cast<ParamType>(it1->second.get("ParamType", 0));
        ParamValue& tmpValLoaded = ParamValue::fromString(paramType, val);
        ParamValue* tmpVal=addNewInput(ParamDefinition(true, paramType, nameIn, helper, tmpValLoaded));

        bool link = it1->second.get("Link", false);

        if (!link)
        {
          try
          {
            if (tmpVal != NULL)
              tmpVal->valid_and_set(tmpVal->fromString(tmpVal->getType(), val));
          }
          catch (...)
          {
            tmpVal->setNew(false);
          }
        }
        else
          toUpdate.push_back(std::pair<ParamValue*, unsigned int>(tmpVal, lexical_cast<unsigned int>(val)));
      }
      if (it1->first.compare("Output") == 0)
      {
        string nameOut = it1->second.get("Name", "Error");
        string helper = it1->second.get("Helper", nameOut);
        ParamType paramType = static_cast<ParamType>(it1->second.get("ParamType", 0));
        ParamValue* tmpVal = addNewOutput(ParamDefinition(true, paramType, nameOut, helper));
        tmpVal->setNew(false);
        string val = it1->second.get("ID", "0");
        addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
      }
      if (it1->first.compare("Condition") == 0)
      {
        int cLeft = it1->second.get("category_left", 0);
        int cRight = it1->second.get("category_right", 0);
        int cOperator = it1->second.get("boolean_operator", 0);

        double valLeft = it1->second.get("_valueleft", 0.);
        double valRight = it1->second.get("_valueright", 0.);
        addCondition(ConditionOfRendering(cLeft, valLeft, cRight, valRight, cOperator,
          this));
        if (cLeft == 1 || cRight == 1)//output of block...
          condToUpdate.push_back(&_conditions.back());
      }
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


    ptree tree;
    tree.put("name", _name);
    tree.put("position", _position);
    tree.put("size_increment", _sizeIncrement);

    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      const ParamDefinition pDef = _inputParams.at(it->first);

      paramTree.put("Name", pDef._name);
      paramTree.put("Helper", pDef._helper);
      paramTree.put("ParamType", pDef._type);

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
      const ParamDefinition pDef = _outputParams.at(it->first);

      ptree paramTree;
      paramTree.put("Name", pDef._name);
      paramTree.put("Helper", pDef._helper);
      paramTree.put("ParamType", pDef._type);
      paramTree.put("ID", (unsigned int)&it->second);

      tree.add_child("Output", paramTree);
    }

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

    for (auto it = _conditions.begin();
      it != _conditions.end(); it++)
      tree.add_child("Condition", it->getXML());

    return tree;
  };

  ParamDefinition SubBlock::getDef(std::string name, bool isInput)
  {
    if (isInput)
    {
      auto iter = _inputParams.find(name);
      if (iter == _inputParams.end())
        return ParamDefinition();
      else
        return iter->second;
    }
    else
    {
      auto iter = _outputParams.find(name);
      if (iter == _outputParams.end())
        return ParamDefinition();
      else
        return iter->second;
    }
  }

};