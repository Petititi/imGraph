
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
  BLOCK_BEGIN_INSTANTIATION(BlockCrop);
  //You can add methods, re implement needed functions... 
  Crop_Filter filter;
  BLOCK_END_INSTANTIATION(BlockCrop, AlgoType::imgProcess, BLOCK__CROP_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockCrop);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Mat, "BLOCK__CROP_IN_IMAGE", "BLOCK__CROP_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_WIDTH", "BLOCK__CROP_WIDTH_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_HEIGHT", "BLOCK__CROP_HEIGHT_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_IN_X", "BLOCK__CROP_IN_X_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_IN_Y", "BLOCK__CROP_IN_Y_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockCrop);
  ADD_PARAMETER(true, Mat, "BLOCK__CROP_OUT_IMAGE", "BLOCK__CROP_OUT_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_WIDTH", "BLOCK__CROP_WIDTH_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_HEIGHT", "BLOCK__CROP_HEIGHT_HELP");

  END_BLOCK_PARAMS();

  BlockCrop::BlockCrop() :Block("BLOCK__CROP_NAME"){
    myInputs_["BLOCK__CROP_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  bool BlockCrop::run(){
    if (myInputs_["BLOCK__CROP_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = myInputs_["BLOCK__CROP_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      if (!myInputs_["BLOCK__CROP_IN_X"].isDefaultValue())
        filter.set("x",
        myInputs_["BLOCK__CROP_IN_X"].get<int>());
      if (!myInputs_["BLOCK__CROP_IN_Y"].isDefaultValue())
        filter.set("y",
        myInputs_["BLOCK__CROP_IN_Y"].get<int>());
      if (!myInputs_["BLOCK__CROP_WIDTH"].isDefaultValue())
        filter.set("width",
        myInputs_["BLOCK__CROP_WIDTH"].get<int>());
      if (!myInputs_["BLOCK__CROP_HEIGHT"].isDefaultValue())
        filter.set("height",
        myInputs_["BLOCK__CROP_HEIGHT"].get<int>());
      myOutputs_["BLOCK__CROP_OUT_IMAGE"] = filter.process(mat);
      myOutputs_["BLOCK__CROP_WIDTH"] = myInputs_["BLOCK__CROP_WIDTH"].get<int>();
      myOutputs_["BLOCK__CROP_HEIGHT"] = myInputs_["BLOCK__CROP_HEIGHT"].get<int>();
    }
    return true;
  };
};