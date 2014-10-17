
#include <queue>

#include "Block.h"
#include "ParamValidator.h"
using namespace lsis_org;
using namespace cv;
using std::vector;
using std::string;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(DelayBlock);
  //You can add methods, re implement needed functions...
  std::queue<Mat> imgs;
  BLOCK_END_INSTANTIATION(DelayBlock, AlgoType::videoProcess, BLOCK__DELAY_VIDEO_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(DelayBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__DELAY_VIDEO_IN_IMAGE", "BLOCK__DELAY_VIDEO_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__DELAY_VIDEO_IN_DELAY", "BLOCK__DELAY_VIDEO_IN_DELAY_HELP", 1);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(DelayBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__DELAY_VIDEO_OUT_IMAGE", "BLOCK__DELAY_VIDEO_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(DelayBlock);
  END_BLOCK_PARAMS();

  DelayBlock::DelayBlock() :Block("BLOCK__DELAY_VIDEO_NAME"){
    _myInputs["BLOCK__DELAY_VIDEO_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__DELAY_VIDEO_IN_DELAY"].addValidator({ new ValNeeded(), new ValPositiv(true) });
  };
  
  bool DelayBlock::run(){
    cv::Mat src = _myInputs["BLOCK__DELAY_VIDEO_IN_IMAGE"].get<cv::Mat>();
    int historySize = _myInputs["BLOCK__DELAY_VIDEO_IN_DELAY"].get<int>();
    imgs.push(src);
    Mat out = imgs.front();
    if ((int)imgs.size() > historySize)
      imgs.pop();
    _myOutputs["BLOCK__DELAY_VIDEO_OUT_IMAGE"] = out;
    renderingDone();
    return true;
  };
};