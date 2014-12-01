
#ifdef _WIN32
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

using namespace lsis_org;
using std::vector;
using std::string;
using namespace cv;
using boost::property_tree::ptree;
using boost::lexical_cast;

namespace charliesoft
{
  bool SubBlock::addedToList = 
    charliesoft::ProcessManager::getInstance()->addNewAlgo<SubBlock>(input, "SUBBLOCK__");

  SubBlock::SubBlock() :Block("SUBBLOCK__"){
    _subGraph = new GraphOfProcess();
  };

  bool SubBlock::run(bool oneShot){
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
      link._from->waitUpdateTimestamp(lock);
      ParamValue* value = link._from->getParam(link._fromParam, false);
      _myOutputs[link._toParam].setValue(value);
    }
    _subGraph->waitUntilEnd();
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


  void SubBlock::initFromXML(boost::property_tree::ptree* block,
    std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
    std::map<unsigned int, ParamValue*>& addressesMap,
    std::vector<ConditionOfRendering*>& condToUpdate)
  {
    string pos = block->get("position", "[0,0]");
    int posSepare = pos.find_first_of(',') + 1;
    string xPos = pos.substr(1, posSepare - 2);
    string yPos = pos.substr(posSepare + 1, pos.size() - posSepare - 2);

    setPosition(lexical_cast<int>(xPos), lexical_cast<int>(yPos));
    for (ptree::iterator it1 = block->begin(); it1 != block->end(); it1++)
    {
      if (it1->first.compare("Input") == 0)
      {
        string nameIn = it1->second.get("Name", "Error");
        string helper = it1->second.get("Helper", nameIn);
        string val = it1->second.get("Value", "Not initialized...");
        ParamType paramType = static_cast<ParamType>(it1->second.get("ParamType", 0));
        ParamValue& tmpValLoaded = ParamValue::fromString(paramType, val);
        addNewInput(ParamDefinition(true, paramType, nameIn, helper, tmpValLoaded));

        bool link = it1->second.get("Link", false);
        ParamValue* tmpVal = getParam(nameIn, true);
        if (!link)
        {
          try
          {
            if (tmpVal != NULL)
              tmpVal->valid_and_set(tmpValLoaded);
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
        addNewOutput(ParamDefinition(true, paramType, nameOut, helper));
        string val = it1->second.get("ID", "0");
        ParamValue* tmpVal = getParam(nameOut, false);
        addressesMap[lexical_cast<unsigned int>(val)] = tmpVal;
      }
      if (it1->first.compare("Condition") == 0)
      {
        int cLeft = it1->second.get("category_left", 0);
        int cRight = it1->second.get("category_right", 0);
        int cOperator = it1->second.get("boolean_operator", 0);

        double valLeft = it1->second.get("Value_left", 0.);
        double valRight = it1->second.get("Value_right", 0.);
        addCondition(ConditionOfRendering(cLeft, valLeft, cRight, valRight, cOperator,
          this));
        if (cLeft == 1 || cRight == 1)//output of block...
          condToUpdate.push_back(&_conditions.back());
      }
    }
  }

  boost::property_tree::ptree SubBlock::getXML() const
  {
    ptree tree;
    tree.put("name", _name);
    tree.put("position", _position);

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
        paramTree.put("Value", (unsigned int)it->second.get<ParamValue*>(false));

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

    for (auto it = _conditions.begin();
      it != _conditions.end(); it++)
      tree.add_child("Condition", it->getXML());

    return tree;
  };
};