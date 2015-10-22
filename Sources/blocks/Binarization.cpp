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
  BLOCK_BEGIN_INSTANTIATION(BinarizeBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(BinarizeBlock, AlgoType::mathOperator, BLOCK__BINARIZE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BinarizeBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__BINARIZE_IN_IMAGE", "BLOCK__BINARIZE_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(userConstant, ListBox, "BLOCK__BINARIZE_IN_METHOD", "BLOCK__BINARIZE_IN_METHOD_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BinarizeBlock);
  ADD_PARAMETER(toBeLinked, AnyType, "BLOCK__BINARIZE_OUT_IMAGE", "BLOCK__BINARIZE_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BinarizeBlock);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__BINARIZE_IN_METHOD.Simple threshold.threshold", "threshold", 128.);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__BINARIZE_IN_METHOD.Adaptative.threshold", "threshold", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__BINARIZE_IN_METHOD.Adaptative.window size", "window size", 7);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__BINARIZE_IN_METHOD.Sauvola.threshold", "threshold", 0.3);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__BINARIZE_IN_METHOD.Sauvola.window size", "window size", 9);
  END_BLOCK_PARAMS();

  BinarizeBlock::BinarizeBlock() :Block("BLOCK__BINARIZE_NAME", true){
    _myInputs["BLOCK__BINARIZE_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  cv::Mat binarizeSauvolaIntegral(cv::Mat gray_image, int whalf, double k)
  {
    cv::Mat out(gray_image.size(), CV_8UC1);
    int w = whalf * 2 + 1;

    if (k < 0.02 || k > 0.95)
      k = 0.2;
    if (w < 2)
      w = 15;

    int image_width = gray_image.cols;
    int image_height = gray_image.rows;

    // Calculate the integral image, and integral of the squared image
    cv::Mat integral_image(gray_image.size(), CV_64FC1), rowsum_image(gray_image.size(), CV_64FC1);
    cv::Mat integral_sqimg(gray_image.size(), CV_64FC1), rowsum_sqimg(gray_image.size(), CV_64FC1);

    int xmin, ymin, xmax, ymax;
    double diagsum, idiagsum, diff, sqdiagsum, sqidiagsum, sqdiff, area;
    double mean, std, threshold;

    cv::integral(gray_image, integral_image, integral_sqimg, CV_64F);
    //Calculate the mean and standard deviation using the integral image

    for (int i = 0; i < image_width; i++){
      for (int j = 0; j < image_height; j++){
        xmin = cv::max(0, i - whalf);
        ymin = cv::max(0, j - whalf);
        xmax = cv::min(image_width - 1, i + whalf);
        ymax = cv::min(image_height - 1, j + whalf);
        xmin++; ymin++; xmax++; ymax++;//first column/row is empty...
        area = (xmax - xmin + 1)*(ymax - ymin + 1);
        if (xmin <= 1 && ymin <= 1){ // Point at origin
          diff = integral_image.at<double>(ymax, xmax);
          sqdiff = integral_sqimg.at<double>(ymax, xmax);
        }
        else if (xmin <= 1 && ymin > 1){ // first column
          diff = integral_image.at<double>(ymax, xmax) - integral_image.at<double>(ymin - 1, xmax);
          sqdiff = integral_sqimg.at<double>(ymax, xmax) - integral_sqimg.at<double>(ymin - 1, xmax);
        }
        else if (xmin > 1 && ymin <= 1){ // first row
          diff = integral_image.at<double>(ymax, xmax) - integral_image.at<double>(ymax, xmin - 1);
          sqdiff = integral_sqimg.at<double>(ymax, xmax) - integral_sqimg.at<double>(ymax, xmin - 1);
        }
        else{ // rest of the image
          diagsum = integral_image.at<double>(ymax, xmax) + integral_image.at<double>(ymin - 1, xmin - 1);
          idiagsum = integral_image.at<double>(ymin - 1, xmax) + integral_image.at<double>(ymax, xmin - 1);
          diff = diagsum - idiagsum;
          sqdiagsum = integral_sqimg.at<double>(ymax, xmax) + integral_sqimg.at<double>(ymin - 1, xmin - 1);
          sqidiagsum = integral_sqimg.at<double>(ymin - 1, xmax) + integral_sqimg.at<double>(ymax, xmin - 1);
          sqdiff = sqdiagsum - sqidiagsum;
        }

        mean = diff / area;
        double var = (sqdiff - (diff*diff / area)) / area;
        if (var < 0)
          var = 0;
        std = sqrt(var);
        threshold = mean*(1.0 + k*((std / 128.0) - 1.0));

        if (gray_image.at<uchar>(j, i) <= threshold)
          out.at<uchar>(j, i) = 0;
        else
          out.at<uchar>(j, i) = 254;
      }
    }
    return out;
  }

  bool BinarizeBlock::run(bool oneShot){
    if (_myInputs["BLOCK__BINARIZE_IN_IMAGE"].isDefaultValue())
      return false;

    cv::Mat mat = _myInputs["BLOCK__BINARIZE_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      cv::Mat output = MatrixConvertor::adjustChannels(mat.clone(), 1);
      double threshold = 128;
      int winSize = 15;
      switch (_myInputs["BLOCK__CREATEMATRIX_IN_INIT"].get<int>())
      {
      case 1://Otsu
        cv::threshold(output, output, 128, 255, cv::THRESH_OTSU);
        break;
      case 2://Adaptative
        cv::adaptiveThreshold(output, output, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,
          _mySubParams["BLOCK__BINARIZE_IN_METHOD.Adaptative.window size"].get<int>(),
          _mySubParams["BLOCK__BINARIZE_IN_METHOD.Adaptative.threshold"].get<double>()
          );
        break;
      case 3://Sauvola
        output = binarizeSauvolaIntegral(output, _mySubParams["BLOCK__BINARIZE_IN_METHOD.Sauvola.window size"].get<int>(),
          _mySubParams["BLOCK__BINARIZE_IN_METHOD.Sauvola.threshold"].get<double>()
          );
        break;
      default://Simple threshold
        threshold = _mySubParams["BLOCK__BINARIZE_IN_METHOD.Simple threshold.threshold"].get<double>();
        cv::threshold(output, output, threshold, 255, cv::THRESH_BINARY);
      }

      _myOutputs["BLOCK__BINARIZE_OUT_IMAGE"] = output;
    }
    return !mat.empty();
  };
};