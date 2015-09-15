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
  BLOCK_BEGIN_INSTANTIATION(ThinningBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(ThinningBlock, AlgoType::mathOperator, BLOCK__THINNING_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(ThinningBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__THINNING_IN_IMAGE", "BLOCK__THINNING_IN_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(ThinningBlock);
  ADD_PARAMETER(true, AnyType, "BLOCK__THINNING_OUT_IMAGE", "BLOCK__THINNING_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(ThinningBlock);
  END_BLOCK_PARAMS();

  ThinningBlock::ThinningBlock() :Block("BLOCK__THINNING_NAME", true){
    _myInputs["BLOCK__THINNING_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  /**
  * Perform one thinning iteration.
  * Normally you wouldn't call this function directly from your code.
  *
  * @param  im    Binary image with range = 0-1
  * @param  iter  0=even, 1=odd
  */
  void thinningIteration(cv::Mat& im, int iter)
  {
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    uchar *line, *prevLine, *nextLine;
    nextLine = im.ptr<uchar>(1);
    line = im.ptr<uchar>(0);
    for (int i = 1; i < im.rows - 1; i++)
    {
      prevLine = line;
      line = nextLine;
      nextLine = im.ptr<uchar>(i + 1);
      for (int j = 1; j < im.cols - 1; j++)
      {
        const uchar& p2 = prevLine[j];
        const uchar& p3 = prevLine[j + 1];
        const uchar& p4 = line[j + 1];
        const uchar& p5 = nextLine[j + 1];
        const uchar& p6 = nextLine[j];
        const uchar& p7 = nextLine[j - 1];
        const uchar& p8 = line[j - 1];
        const uchar& p9 = prevLine[j - 1];

        int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
          (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
          (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
          (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
        int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
        int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
        int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

        if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
          marker.at<uchar>(i, j) = 1;
      }
    }

    im &= ~marker;
  }

  void thinning(cv::Mat& im)
  {
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
      thinningIteration(im, 0);
      thinningIteration(im, 1);
      cv::absdiff(im, prev, diff);
      im.copyTo(prev);
    } while (cv::countNonZero(diff) > 0);

    im *= 255;
  }

  bool ThinningBlock::run(bool oneShot){
    if (_myInputs["BLOCK__THINNING_IN_IMAGE"].isDefaultValue())
      return false;
    
    cv::Mat mat = _myInputs["BLOCK__THINNING_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      cv::Mat output = MatrixConvertor::adjustChannels(mat.clone(), 1);

      bool inverse = mean(output)[0] > 128;
      if (inverse)
        output = 255 - output;

      thinning(output);

      if (inverse)
        output = 255 - output;

      _myOutputs["BLOCK__THINNING_OUT_IMAGE"] = output;
    }
    return !mat.empty();
  };
};