
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
using namespace lsis_org;
using std::vector;
using std::string;
using namespace cv;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(PointMatcherBlock);
  //You can add methods, re implement needed functions... 
public:
  BLOCK_END_INSTANTIATION(PointMatcherBlock, AlgoType::imgProcess, BLOCK__POINT_MATCHER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(PointMatcherBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_MATCHER_IN_PT_DESC1", "BLOCK__POINT_MATCHER_IN_PT_DESC1_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_MATCHER_IN_PT_DESC2", "BLOCK__POINT_MATCHER_IN_PT_DESC2_HELP");
  ADD_PARAMETER(false, ListBox, "BLOCK__POINT_MATCHER_IN_ALGO", "BLOCK__POINT_MATCHER_IN_ALGO_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(PointMatcherBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_MATCHER_OUT_MATCHES", "BLOCK__POINT_MATCHER_OUT_MATCHES_HELP");
  ADD_PARAMETER(false, Matrix, "BLOCK__POINT_MATCHER_OUT_MATCHES_MASK", "BLOCK__POINT_MATCHER_OUT_MATCHES_MASK_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(PointMatcherBlock);
  END_BLOCK_PARAMS();


  PointMatcherBlock::PointMatcherBlock() :Block("BLOCK__POINT_MATCHER_NAME"){
    _myInputs["BLOCK__POINT_MATCHER_IN_PT_DESC1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_MATCHER_IN_PT_DESC2"].addValidator({ new ValNeeded() });
  };

  bool PointMatcherBlock::run(bool oneShot){
    cv::Mat pt1 = _myInputs["BLOCK__POINT_MATCHER_IN_PT_DESC1"].get<cv::Mat>(true);
    cv::Mat pt2 = _myInputs["BLOCK__POINT_MATCHER_IN_PT_DESC2"].get<cv::Mat>(true);
    throw "todo!";
    return true;
  };
};