
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>

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
  BLOCK_BEGIN_INSTANTIATION(LineFinerBlock);
  //You can add methods, re implement needed functions... 
  LineFinder filter;
  BLOCK_END_INSTANTIATION(LineFinerBlock, AlgoType::imgProcess, BLOCK__LINE_FINDER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(LineFinerBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LINE_FINDER_IN_IMAGE", "BLOCK__LINE_FINDER_IN_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(LineFinerBlock);
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LINE_FINDER_OUT_IMAGE", "BLOCK__LINE_FINDER_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(LineFinerBlock);
  END_BLOCK_PARAMS();

  LineFinerBlock::LineFinerBlock() :Block("BLOCK__LINE_FINDER_NAME", true){
    _myInputs["BLOCK__LINE_FINDER_IN_IMAGE"].addValidator({ new ValNeeded() });
  };
  
  bool LineFinerBlock::run(bool oneShot){
    if (_myInputs["BLOCK__LINE_FINDER_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__LINE_FINDER_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
      _myOutputs["BLOCK__LINE_FINDER_OUT_IMAGE"] = filter.process(mat);

    return true;
  };
};