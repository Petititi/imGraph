
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include "opencv2\imgproc.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

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
  ADD_PARAMETER_FULL(false, Int, "BLOCK__CROP_WIDTH", "BLOCK__CROP_WIDTH_HELP", -1);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__CROP_HEIGHT", "BLOCK__CROP_HEIGHT_HELP", -1);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockCrop);
  END_BLOCK_PARAMS();

  BlockCrop::BlockCrop() :Block("BLOCK__CROP_NAME", true){
    _myInputs["BLOCK__CROP_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  bool BlockCrop::run(bool oneShot){
    cv::Mat mat = _myInputs["BLOCK__CROP_IN_IMAGE"].get<cv::Mat>();
    if (mat.empty())
      return false;

    if (!_myInputs["BLOCK__CROP_IN_X"].isDefaultValue())
      filter.setRegionX(_myInputs["BLOCK__CROP_IN_X"].get<int>());
    if (!_myInputs["BLOCK__CROP_IN_Y"].isDefaultValue())
      filter.setRegionY(_myInputs["BLOCK__CROP_IN_Y"].get<int>());
    if (!_myInputs["BLOCK__CROP_WIDTH"].isDefaultValue())
      filter.setRegionWidth(_myInputs["BLOCK__CROP_WIDTH"].get<int>());
    if (!_myInputs["BLOCK__CROP_HEIGHT"].isDefaultValue())
      filter.setRegionHeight(_myInputs["BLOCK__CROP_HEIGHT"].get<int>());

    cv::Mat imgTmp = filter.process(mat);
    _myOutputs["BLOCK__CROP_OUT_IMAGE"] = imgTmp;
    _myOutputs["BLOCK__CROP_WIDTH"] = imgTmp.cols;
    _myOutputs["BLOCK__CROP_HEIGHT"] = imgTmp.rows;

    return true;
  };



  BLOCK_BEGIN_INSTANTIATION(ResizeMatrix);
  //You can add methods, re implement needed functions...
public:
  BLOCK_END_INSTANTIATION(ResizeMatrix, AlgoType::imgProcess, BLOCK__RESIZEMATRIX_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(ResizeMatrix);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__RESIZEMATRIX_IN_IMG", "BLOCK__RESIZEMATRIX_IN_IMG_HELP");
  ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__RESIZEMATRIX_IN_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP", 640);
  ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__RESIZEMATRIX_IN_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP", 480);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(ResizeMatrix);
  ADD_PARAMETER(true, Matrix, "BLOCK__RESIZEMATRIX_OUT_IMAGE", "BLOCK__RESIZEMATRIX_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(ResizeMatrix);
  END_BLOCK_PARAMS();

  ResizeMatrix::ResizeMatrix() :Block("BLOCK__RESIZEMATRIX_NAME", true){
    _myInputs["BLOCK__RESIZEMATRIX_IN_IMG"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__RESIZEMATRIX_IN_WIDTH"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__RESIZEMATRIX_IN_HEIGHT"].addValidator({ new ValNeeded(), new ValPositiv(true) });
  };

  bool ResizeMatrix::run(bool oneShot){
    //todo: verify that type index correspond to constant!
    cv::Mat imgSrc = _myInputs["BLOCK__RESIZEMATRIX_IN_IMG"].get<cv::Mat>();
    int wantedRow, wantedCol;
    if (_myInputs["BLOCK__RESIZEMATRIX_IN_HEIGHT"].isDefaultValue())
      wantedRow = imgSrc.rows;
    else
      wantedRow = _myInputs["BLOCK__RESIZEMATRIX_IN_HEIGHT"].get<int>();
    if (_myInputs["BLOCK__RESIZEMATRIX_IN_WIDTH"].isDefaultValue())
      wantedCol = imgSrc.cols;
    else
      wantedCol = _myInputs["BLOCK__RESIZEMATRIX_IN_WIDTH"].get<int>();

    cv::Mat output;
    cv::resize(imgSrc, output, cv::Size(wantedCol, wantedRow));
    _myOutputs["BLOCK__RESIZEMATRIX_OUT_IMAGE"] = output;

    return true;
  };
};