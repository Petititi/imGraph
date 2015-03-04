
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
#include "Convertor.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(StringCreationBlock);
  //You can add methods, re implement needed functions... 
  std::map<std::string, ParamValue*> valToReplace;
  virtual void init();
  BLOCK_END_INSTANTIATION(StringCreationBlock, AlgoType::imgProcess, BLOCK__STRING_CREATION_NAME);
  BEGIN_BLOCK_INPUT_PARAMS(StringCreationBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(userConstant, String, "BLOCK__STRING_CREATION_IN_REGEX", "BLOCK__STRING_CREATION_IN_REGEX_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(StringCreationBlock);
  ADD_PARAMETER(true, String, "BLOCK__STRING_CREATION_OUT", "BLOCK__STRING_CREATION_OUT_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(StringCreationBlock);
  END_BLOCK_PARAMS();

  StringCreationBlock::StringCreationBlock() :
    Block("BLOCK__STRING_CREATION_NAME", true, synchrone, true){
    _myInputs["BLOCK__STRING_CREATION_IN_REGEX"].addValidator({ new ValNeeded() });
    valToReplace["%n%"] = new ParamValue(0);
  };

  void StringCreationBlock::init()
  {
    *valToReplace["%n%"] = 0;
  }

  bool StringCreationBlock::run(bool oneShot){
    std::string myVal = _myInputs["BLOCK__STRING_CREATION_IN_REGEX"].get<std::string>();

    (*valToReplace["%n%"]) = valToReplace["%n%"]->get<int>() + 1;

    std::vector<ParamValue*> otherParams;
    for (auto& it = _myInputs.begin();
      it != _myInputs.end(); it++)
      if (it->first.compare("BLOCK__STRING_CREATION_IN_REGEX") != 0)
        otherParams.push_back(&(it->second));

    std::string finalVal = StringConvertor::regExExpend(myVal, otherParams, valToReplace);
    _myOutputs["BLOCK__STRING_CREATION_OUT"] = finalVal;
    return true;
  };
};