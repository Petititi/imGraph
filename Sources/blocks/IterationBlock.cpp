
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "IterationBlock.h"

using namespace charliesoft;
using std::vector;
using std::string;
using namespace cv;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  bool ForBlock::addedToList =
    charliesoft::ProcessManager::getInstance()->addNewAlgo<ForBlock>(input, "FOR_BLOCK_");

  bool ForBlock::run(bool oneShot)
  {
    boost::unique_lock<boost::mutex> lock(_mtx_1);
    //first set input values:
    for (auto link : externBlocksInput)
    {
      try
      {
        if (!_myInputs[link._fromParam].isDefaultValue())
        {
          ParamValue* distVal = link._to->getParam(link._toParam, true);
          distVal->setValue(&_myInputs[link._fromParam]);

          _myInputs[link._fromParam].setNew(false);
        }
      }
      catch (...)
      {
        //nothing to do...
      }
    }
    _subGraph->run(true);
    //wait for ouput updates:
    for (auto link : externBlocksOutput)
    {
      link._from->waitProducers(lock);
      ParamValue* value = link._from->getParam(link._fromParam, false);
      _myOutputs[link._toParam].setValue(value);
    }
    _subGraph->waitUntilEnd();
    return true;
  };

  ForBlock::ForBlock() :SubBlock("FOR_BLOCK_")
  {
    ParamDefinition* for_params = new ParamDefinition(
      true, Float, "FOR_BLOCK_INITVAL", "FOR_BLOCK_INITVAL_HELP");
    for_params->_initVal = 0;
    addNewInput(for_params); 
    for_params = new ParamDefinition(
      true, Float, "FOR_BLOCK_ENDVAL", "FOR_BLOCK_ENDVAL_HELP");
    for_params->_initVal = 1;
    addNewInput(for_params);
    for_params = new ParamDefinition(
      true, Float, "FOR_BLOCK_STEPVAL", "FOR_BLOCK_STEPVAL_HELP");
    addNewInput(for_params);
  }

  boost::property_tree::ptree ForBlock::getXML()
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

    for (auto it = _myInputs.begin();
      it != _myInputs.end(); it++)
    {
      ptree paramTree;
      ParamDefinition& pDef = *getParamDefinition(it->first, true);

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
      ParamDefinition& pDef = *getParamDefinition(it->first, false);

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
  }
  void ForBlock::initFromXML(boost::property_tree::ptree* block,
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
        ParamValue* tmpVal = addNewInput(new ParamDefinition(true, paramType, nameIn, helper, tmpValLoaded));

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
        addNewOutput(new ParamDefinition(true, paramType, nameOut, helper));
        string val = it1->second.get("ID", "0");
        ParamValue* tmpVal = getParam(nameOut, false);
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
};