#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <opencv2/imgproc.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
#include "Convertor.h"
using namespace charliesoft;
using std::vector;
using boost::lexical_cast;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(DistanceTransformBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(DistanceTransformBlock, AlgoType::mathOperator, BLOCK__DISTANCE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(DistanceTransformBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__DISTANCE_IN_IMAGE", "BLOCK__DISTANCE_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(userConstant, ListBox, "BLOCK__DISTANCE_IN_DISTANCETYPE", "BLOCK__DISTANCE_IN_DISTANCETYPE_HELP", 2);
  ADD_PARAMETER_FULL(userConstant, Boolean, "BLOCK__BINARIZE_IN_INVERSE", "BLOCK__BINARIZE_IN_INVERSE_HELP", false);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(DistanceTransformBlock);
  ADD_PARAMETER(toBeLinked, AnyType, "BLOCK__DISTANCE_OUT_IMAGE", "BLOCK__DISTANCE_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(DistanceTransformBlock);
  END_BLOCK_PARAMS();

  DistanceTransformBlock::DistanceTransformBlock() :Block("BLOCK__DISTANCE_NAME", true){
    _myInputs["BLOCK__DISTANCE_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  bool DistanceTransformBlock::run(bool oneShot){
    if (_myInputs["BLOCK__DISTANCE_IN_IMAGE"].isDefaultValue())
      return false;
    
    cv::Mat mat = _myInputs["BLOCK__DISTANCE_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      int choice = _myInputs["BLOCK__DISTANCE_IN_DISTANCETYPE"].get<int>();
      cv::Mat output = MatrixConvertor::adjustChannels(mat.clone(), 1);
      normalize(output, output, 0, 255, cv::NORM_MINMAX);
      if (_myInputs["BLOCK__BINARIZE_IN_INVERSE"].get<bool>())
        output = 255 - output;
      int distType = cv::DIST_L2;
      int mask = cv::DIST_MASK_3;
      if (choice == 0)
        distType = cv::DIST_C;
      else if (choice == 1)
        distType = cv::DIST_L1;
      else if (choice == 3)
      {
        distType = cv::DIST_L2;
        mask = cv::DIST_MASK_5;
      }
      cv::distanceTransform(output, output, distType, mask);

      _myOutputs["BLOCK__DISTANCE_OUT_IMAGE"] = output;
    }
    return !mat.empty();
  };
};