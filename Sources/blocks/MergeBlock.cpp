#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <opencv2/imgproc.hpp>
#include <vector>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(MergingBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(MergingBlock, AlgoType::mathOperator, BLOCK__MERGING_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(MergingBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, AnyType, "BLOCK__MERGING_IN_IMAGE1", "BLOCK__MERGING_IN_IMAGE1_HELP");
  ADD_PARAMETER(true, AnyType, "BLOCK__MERGING_IN_IMAGE2", "BLOCK__MERGING_IN_IMAGE2_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(MergingBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__MERGING_OUT_IMAGE", "BLOCK__MERGING_OUT_IMAGE");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(MergingBlock);
  END_BLOCK_PARAMS();

  MergingBlock::MergingBlock() :Block("BLOCK__MERGING_NAME"){
    _myInputs["BLOCK__MERGING_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__MERGING_IN_IMAGE2"].addValidator({ new ValNeeded() });
  };
  
  bool MergingBlock::run(bool oneShot){
    if (_myInputs["BLOCK__MERGING_IN_IMAGE1"].isDefaultValue())
      return false;
    if (_myInputs["BLOCK__MERGING_IN_IMAGE2"].isDefaultValue())
      return false;
    cv::Mat mat1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].get<cv::Mat>(true) / 2.;
    cv::Mat mat2 = _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Mat>(true) / 2.;

    if (!mat1.empty() && !mat2.empty())
    {
      if (mat1.channels() != mat2.channels())
      {
        if (mat1.channels() == 1 &&
          mat2.channels() == 3)
          cv::cvtColor(mat1, mat1, cv::COLOR_GRAY2BGR);
        if (mat2.channels() == 1 &&
          mat1.channels() == 3)
          cv::cvtColor(mat2, mat2, cv::COLOR_GRAY2BGR);
      }
      _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = mat1 + mat2;
    }
    return true;
  };
};