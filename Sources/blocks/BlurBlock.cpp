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

using namespace charliesoft;
using std::vector;
using boost::lexical_cast;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlurBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(BlurBlock, AlgoType::imgProcess, BLOCK__BLUR_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlurBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__BLUR_IN_IMG", "BLOCK__BLUR_IN_IMG_HELP");
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__BLUR_IN_METHOD", "BLOCK__BLUR_IN_METHOD_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlurBlock);
  ADD_PARAMETER(toBeLinked, AnyType, "BLOCK__BLUR_OUT_IMAGE", "BLOCK__BLUR_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlurBlock);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Mean.kernel size X", "kernel size X", 3);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Mean.kernel size Y", "kernel size Y", 3);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Mean.anchor point X", "anchor point X", -1);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Mean.anchor point Y", "anchor point Y", -1);

  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Gaussian.kernel size X", "kernel size X", 3);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Gaussian.kernel size Y", "kernel size Y", 3);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__BLUR_IN_METHOD.Gaussian.Sigma X", "Sigma X", 1);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__BLUR_IN_METHOD.Gaussian.Sigma Y", "Sigma Y", 0);

  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Median.kernel size", "kernel size", 3);

  ADD_PARAMETER_FULL(false, Int, "BLOCK__BLUR_IN_METHOD.Bilateral.Diameter", "Diameter", 5);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__BLUR_IN_METHOD.Bilateral.Sigma color", "Sigma color", 10.);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__BLUR_IN_METHOD.Bilateral.Sigma space", "Sigma space", 10.);
  END_BLOCK_PARAMS();

  BlurBlock::BlurBlock() :Block("BLOCK__BLUR_NAME", true){
    _myInputs["BLOCK__BLUR_IN_METHOD"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__BLUR_IN_IMG"].addValidator({ new ValNeeded() });
  };

  bool BlurBlock::run(bool oneShot){
    Mat imgSrc = _myInputs["BLOCK__BLUR_IN_IMG"].get<cv::Mat>(),
      imgOut;

    switch (_myInputs["BLOCK__BLUR_IN_METHOD"].get<int>())
    {
    case 0://Mean
    {
      blur(imgSrc, imgOut, 
        cv::Size(_mySubParams["BLOCK__BLUR_IN_METHOD.Mean.kernel size X"].get<int>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Mean.kernel size Y"].get<int>()),
        cv::Point(_mySubParams["BLOCK__BLUR_IN_METHOD.Mean.anchor point X"].get<int>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Mean.anchor point Y"].get<int>()),
        cv::BORDER_DEFAULT);
      break;
    }
    case 1://Gaussian
    {
      cv::Size ksize(_mySubParams["BLOCK__BLUR_IN_METHOD.Gaussian.kernel size X"].get<int>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Gaussian.kernel size Y"].get<int>());
      if (ksize.width <= 0) ksize.width = 1;
      if (ksize.width % 2 == 0) ksize.width += 1;
      if (ksize.height <= 0) ksize.height = 1;
      if (ksize.height % 2 == 0) ksize.height += 1;

      GaussianBlur(imgSrc, imgOut, ksize,
        _mySubParams["BLOCK__BLUR_IN_METHOD.Gaussian.Sigma X"].get<double>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Gaussian.Sigma Y"].get<double>(),
        cv::BORDER_DEFAULT);
      break;
    }
    case 2://Median
    {
      int medianSize = _mySubParams["BLOCK__BLUR_IN_METHOD.Median.kernel size"].get<int>();
      if (medianSize % 2 != 1)
        medianSize += 1;
      medianBlur(imgSrc, imgOut, medianSize);
      break;
    }
    case 3://Bilateral
    {
      bilateralFilter(imgSrc, imgOut, _mySubParams["BLOCK__BLUR_IN_METHOD.Bilateral.Diameter"].get<int>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Bilateral.Sigma color"].get<double>(),
        _mySubParams["BLOCK__BLUR_IN_METHOD.Bilateral.Sigma space"].get<double>());
      break;
    }
    default:
      return false;//nothing to do as we don't support this type of operation
      break;
    }
    _myOutputs["BLOCK__BLUR_OUT_IMAGE"] = imgOut;

    return true;
  };
};