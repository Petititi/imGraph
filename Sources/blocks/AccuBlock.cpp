
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(AccuBlock);
  //You can add methods, re implement needed functions... 
  Accumul_Filter filter;
  BLOCK_END_INSTANTIATION(AccuBlock, AlgoType::imgProcess, BLOCK__ACCUMULATOR_NAME);
  BEGIN_BLOCK_INPUT_PARAMS(AccuBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Mat, "BLOCK__ACCUMULATOR_IN_IMAGE", "BLOCK__ACCUMULATOR_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__ACCUMULATOR_IN_NB_HISTORY", "BLOCK__ACCUMULATOR_IN_NB_HISTORY_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(AccuBlock);
  ADD_PARAMETER(true, Mat, "BLOCK__ACCUMULATOR_OUT_IMAGE", "BLOCK__ACCUMULATOR_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  AccuBlock::AccuBlock() :Block("BLOCK__ACCUMULATOR_NAME"), filter(15){
    _myInputs["BLOCK__ACCUMULATOR_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__ACCUMULATOR_IN_NB_HISTORY"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__ACCUMULATOR_IN_NB_HISTORY"] = 15;
  };
  
  bool AccuBlock::run(){
    if (_myInputs["BLOCK__ACCUMULATOR_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__ACCUMULATOR_IN_IMAGE"].get<cv::Mat>();
    int nbAccu = _myInputs["BLOCK__ACCUMULATOR_IN_NB_HISTORY"].get<int>();
    if (nbAccu>0)
      filter.set("nbBufferMax", nbAccu);
    if (!mat.empty())
      _myOutputs["BLOCK__ACCUMULATOR_OUT_IMAGE"] = filter.process(mat);
    renderingDone();
    return true;
  };
};