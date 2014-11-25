
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
  };

  bool SubBlock::run(bool oneShot){
    return true;
  };

  void SubBlock::addNewInput(ParamDefinition& param)
  {
    _inputParams.push_back(param);

    ParamValue& t = _myInputs[param._name] = ParamValue(this, param._name, false);
    t.isNeeded(true);//always needed!
    t = param._initVal;
  };
  void SubBlock::addNewOutput(ParamDefinition& param)
  {
    _outputParams.push_back(param);

    ParamValue& t = _myOutputs[param._name] = ParamValue(this, param._name, true);
    t.isNeeded(true);//always needed!
    t = param._initVal;
  };

  vector<ParamDefinition> SubBlock::getInParams() const
  {
    vector<ParamDefinition> tmp = Block::getInParams();
    for (auto it : _inputParams)
      tmp.push_back(it);
    return tmp;
  };
  vector<ParamDefinition> SubBlock::getOutParams() const
  {
    vector<ParamDefinition> tmp = Block::getOutParams();
    for (auto it : _outputParams)
      tmp.push_back(it);
    return tmp;
  };
};