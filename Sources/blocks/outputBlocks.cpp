
#include <vector>

#include "Block.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockShow);
  //You can add methods, reimplement needed functions...
  //Here we need validateParams:
  virtual bool validateParams();
  BLOCK_END_INSTANTIATION(BlockShow, AlgoType::output, BLOCK__OUTPUT_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockShow);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Mat, "BLOCK__OUTPUT_IN_IMAGE", "BLOCK__OUTPUT_IN_IMAGE_HELP");
  ADD_PARAMETER(false, String, "BLOCK__OUTPUT_IN_WIN_NAME", "BLOCK__OUTPUT_IN_WIN_NAME_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockShow);
  END_BLOCK_PARAMS();

  BlockShow::BlockShow() :Block("BLOCK__OUTPUT_NAME"){};
  
  bool BlockShow::run(){
    if (myInputs_["BLOCK__OUTPUT_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = myInputs_["BLOCK__OUTPUT_IN_IMAGE"].get<cv::Mat>(true);
    if (!mat.empty())
      cv::imshow(myInputs_["BLOCK__OUTPUT_IN_WIN_NAME"].get<string>(true), mat);
    return true;
  };

  bool BlockShow::validateParams(){
    bool isOk = true;
    //we need BLOCK__OUTPUT_IN_IMAGE to be set:
    if (myInputs_["BLOCK__OUTPUT_IN_IMAGE"].isDefaultValue())
    {
      isOk = false;
      error_msg_ += (my_format(_STR("ERROR_PARAM_NEEDED")) % _STR("BLOCK__OUTPUT_IN_IMAGE")).str();
    }
    return isOk;
  };


};