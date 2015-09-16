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


  using namespace std;
  using namespace cv;

  void nonMaximaSuppression(const Mat& src, const int sz, Mat& dst, const Mat mask = Mat()) {

    // initialise the block mask and destination
    const int M = src.rows;
    const int N = src.cols;
    const bool masked = !mask.empty();
    Mat block = 255 * Mat_<uint8_t>::ones(Size(2 * sz + 1, 2 * sz + 1));
    dst = Mat_<uint8_t>::zeros(src.size());

    // iterate over image blocks
    for (int m = 0; m < M; m += sz + 1) {
      for (int n = 0; n < N; n += sz + 1) {
        Point  ijmax;
        double vcmax, vnmax;

        // get the maximal candidate within the block
        Range ic(m, min(m + sz + 1, M));
        Range jc(n, min(n + sz + 1, N));
        minMaxLoc(src(ic, jc), NULL, &vcmax, NULL, &ijmax, masked ? mask(ic, jc) : noArray());
        Point cc = ijmax + Point(jc.start, ic.start);

        // search the neighbours centered around the candidate for the true maxima
        Range in(max(cc.y - sz, 0), min(cc.y + sz + 1, M));
        Range jn(max(cc.x - sz, 0), min(cc.x + sz + 1, N));

        // mask out the block whose maxima we already know
        Mat_<uint8_t> blockmask;
        block(Range(0, in.size()), Range(0, jn.size())).copyTo(blockmask);
        Range iis(ic.start - in.start, min(ic.start - in.start + sz + 1, in.size()));
        Range jis(jc.start - jn.start, min(jc.start - jn.start + sz + 1, jn.size()));
        blockmask(iis, jis) = Mat_<uint8_t>::zeros(Size(jis.size(), iis.size()));

        minMaxLoc(src(in, jn), NULL, &vnmax, NULL, &ijmax, masked ? mask(in, jn).mul(blockmask) : blockmask);
        Point cn = ijmax + Point(jn.start, in.start);

        // if the block centre is also the neighbour centre, then it's a local maxima
        if (vcmax > vnmax) {
          dst.at<uint8_t>(cc.y, cc.x) = 255;
        }
      }
    }
  }

  //from a binary image, find pen size
  cv::Mat findBiggestPenSize(cv::Mat gray_image)
  {
    cv::Mat out = Mat::zeros(gray_image.size(), CV_8UC1);
    int image_width = gray_image.cols;
    int image_height = gray_image.rows;

    // Calculate the integral image, and integral of the squared image
    cv::Mat integral_image(gray_image.size(), CV_64FC1);

    int xmin, ymin, xmax, ymax;
    double diagsum, idiagsum, diff, area;
    double nbBackgroundPixels;

    /*
    cv::integral(gray_image, integral_image, CV_64FC1);
    //Calculate the mean and standard deviation using the integral image

    for (int i = 0; i < image_width; i++){
      for (int j = 0; j < image_height; j++){

        if (gray_image.at<uchar>(j, i) == 0)
          out.at<uchar>(j, i) = 0;//not a pen stroke, skip this pixel...
        else
        {
          //increase a subWindow around pixel until too much background pixel is taken:
          uchar pixelSize = 1;
          bool stop = false;

          while (pixelSize < 200 && !stop)
          {
            xmin = cv::max(0, i - pixelSize);
            ymin = cv::max(0, j - pixelSize);
            xmax = cv::min(image_width - 1, i + pixelSize);
            ymax = cv::min(image_height - 1, j + pixelSize);
            xmin++; ymin++; xmax++; ymax++;//first column/row is empty...
            area = (xmax - xmin + 1)*(ymax - ymin + 1) * 255;
            if (xmin <= 1 && ymin <= 1){ // Point at origin
              diff = integral_image.at<double>(ymax, xmax);
            }
            else if (xmin <= 1 && ymin > 1){ // first column
              diff = integral_image.at<double>(ymax, xmax) - integral_image.at<double>(ymin - 1, xmax);
            }
            else if (xmin > 1 && ymin <= 1){ // first row
              diff = integral_image.at<double>(ymax, xmax) - integral_image.at<double>(ymax, xmin - 1);
            }
            else{ // rest of the image
              diagsum = integral_image.at<double>(ymax, xmax) + integral_image.at<double>(ymin - 1, xmin - 1);
              idiagsum = integral_image.at<double>(ymin - 1, xmax) + integral_image.at<double>(ymax, xmin - 1);
              diff = diagsum - idiagsum;
            }

            nbBackgroundPixels = (area - diff) / 255;

            stop = (nbBackgroundPixels >= pixelSize * 3);

            pixelSize++;
          }

          out.at<uchar>(j, i) = pixelSize;//not a pen stroke, skip this pixel...

        }
      }
    }*/
    cv::Mat distImg;
    cv::distanceTransform(gray_image,
      distImg,
      cv::DIST_L2,
      cv::DIST_MASK_3);
    //recreate img:

    for (int i = 0; i < image_width; i++){
      for (int j = 0; j < image_height; j++){
        circle(out, Point(i, j), distImg.at<float>(j, i) + 0.5, Scalar(128 + distImg.at<float>(j, i) * 20), -1);
      }
    }

    return out;
  }

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

      findBiggestPenSize(output);
      thinning(output);

      if (inverse)
        output = 255 - output;

      _myOutputs["BLOCK__THINNING_OUT_IMAGE"] = output;
    }
    return !mat.empty();
  };
};