
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
  ADD_PARAMETER(true, Matrix, "BLOCK__CROP_IN_IMAGE", "BLOCK__CROP_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_WIDTH", "BLOCK__CROP_WIDTH_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_HEIGHT", "BLOCK__CROP_HEIGHT_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_IN_X", "BLOCK__CROP_IN_X_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_IN_Y", "BLOCK__CROP_IN_Y_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockCrop);
  ADD_PARAMETER(true, Matrix, "BLOCK__CROP_OUT_IMAGE", "BLOCK__CROP_OUT_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_WIDTH", "BLOCK__CROP_WIDTH_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CROP_HEIGHT", "BLOCK__CROP_HEIGHT_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockCrop);
  END_BLOCK_PARAMS();

  BlockCrop::BlockCrop() :Block("BLOCK__CROP_NAME"){
    _myInputs["BLOCK__CROP_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__CROP_WIDTH"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__CROP_HEIGHT"].addValidator({ new ValPositiv(true) });
  };

  bool BlockCrop::run(bool oneShot){
    if (_myInputs["BLOCK__CROP_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__CROP_IN_IMAGE"].get<cv::Mat>(true);
    if (!mat.empty())
    {
      if (!_myInputs["BLOCK__CROP_IN_X"].isDefaultValue())
        filter.set("x",
        _myInputs["BLOCK__CROP_IN_X"].get<int>(true));
      if (!_myInputs["BLOCK__CROP_IN_Y"].isDefaultValue())
        filter.set("y",
        _myInputs["BLOCK__CROP_IN_Y"].get<int>(true));
      if (!_myInputs["BLOCK__CROP_WIDTH"].isDefaultValue())
        filter.set("width",
        _myInputs["BLOCK__CROP_WIDTH"].get<int>(true));
      if (!_myInputs["BLOCK__CROP_HEIGHT"].isDefaultValue())
        filter.set("height",
        _myInputs["BLOCK__CROP_HEIGHT"].get<int>(true));
      cv::Mat imgTmp = filter.process(mat);
      _myOutputs["BLOCK__CROP_OUT_IMAGE"] = imgTmp;
      _myOutputs["BLOCK__CROP_WIDTH"] = imgTmp.cols;
      _myOutputs["BLOCK__CROP_HEIGHT"] = imgTmp.rows;
    }
    return true;
  };
};