
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include "OpenCV_filter.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockNormalize);
  //You can add methods, re implement needed functions... 
  NormalizeFilter filter;
  BLOCK_END_INSTANTIATION(BlockNormalize, AlgoType::imgProcess, BLOCK__NORMALIZ_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockNormalize);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__NORMALIZ_IN_IMAGE", "BLOCK__NORMALIZ_IN_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockNormalize);
  ADD_PARAMETER(true, Matrix, "BLOCK__NORMALIZ_OUT_IMAGE", "BLOCK__NORMALIZ_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockNormalize);
  END_BLOCK_PARAMS();

  BlockNormalize::BlockNormalize() :Block("BLOCK__NORMALIZ_NAME", true){
    _myInputs["BLOCK__NORMALIZ_IN_IMAGE"].addValidator({ new ValNeeded() });
  };
  
  bool BlockNormalize::run(bool oneShot){
    if (_myInputs["BLOCK__NORMALIZ_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__NORMALIZ_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
      _myOutputs["BLOCK__NORMALIZ_OUT_IMAGE"] = filter.process(mat);

    return true;
  };
};