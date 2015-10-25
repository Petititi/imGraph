
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(LineDrawer);
  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(LineDrawer, AlgoType::input, BLOCK__LINEDRAWER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(LineDrawer);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LINEDRAWER_IN_LINES", "BLOCK__LINEDRAWER_IN_LINES_HELP");
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LINEDRAWER_IN_IMAGE", "BLOCK__LINEDRAWER_IN_IMAGE_HELP");
  ADD_PARAMETER(userConstant, Color, "BLOCK__LINEDRAWER_IN_COLOR", "BLOCK__LINEDRAWER_IN_COLOR_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__LINEDRAWER_IN_SIZE", "BLOCK__LINEDRAWER_IN_SIZE_HELP", 1);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(LineDrawer);
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LINEDRAWER_OUT_IMAGE", "BLOCK__LINEDRAWER_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(LineDrawer);
  END_BLOCK_PARAMS();

  LineDrawer::LineDrawer() :Block("BLOCK__LINEDRAWER_NAME", true){
    _myInputs["BLOCK__LINEDRAWER_IN_LINES"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__LINEDRAWER_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__LINEDRAWER_IN_SIZE"].addValidator({ new ValPositiv(true) });
  };

  bool LineDrawer::run(bool oneShot){
    cv::Mat out = _myInputs["BLOCK__LINEDRAWER_IN_IMAGE"].get<cv::Mat>().clone();
    int size = 1;
    cv::Scalar color = cv::Scalar(255, 255, 255);

    if (!_myInputs["BLOCK__LINEDRAWER_IN_SIZE"].isDefaultValue())
      size = _myInputs["BLOCK__LINEDRAWER_IN_SIZE"].get<int>();
    if (!_myInputs["BLOCK__LINEDRAWER_IN_COLOR"].isDefaultValue())
      color = _myInputs["BLOCK__LINEDRAWER_IN_COLOR"].get<cv::Scalar>();

    cv::Mat lines = _myInputs["BLOCK__LINEDRAWER_IN_LINES"].get<cv::Mat>();
    int nbChanels = lines.channels();
    if (nbChanels != 1)
      lines = lines.reshape(1, lines.rows);
    if (lines.cols < 4)
      return false;
    for (int i = 0; i < lines.rows; i++)
    {
      int type = lines.type();
      if (type == CV_8UC1)//char
      {
        uchar* l = lines.ptr<uchar>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
      if (type == CV_16UC1)//char
      {
        ushort* l = lines.ptr<ushort>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
      if (type == CV_16SC1)//char
      {
        short* l = lines.ptr<short>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
      if (type == CV_32SC1)//int
      {
        int* l = lines.ptr<int>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
      if (type == CV_32FC1)//float
      {
        float* l = lines.ptr<float>(i);
        line(out, cv::Point((int)l[0], (int)l[1]), 
          cv::Point((int)l[2], (int)l[3]), color, size);
      }
      if (type == CV_64FC1)//double
      {
        double* l = lines.ptr<double>(i);
        line(out, cv::Point((int)l[0], (int)l[1]), 
          cv::Point((int)l[2], (int)l[3]), color, size);
      }
    }

    _myOutputs["BLOCK__LINEDRAWER_OUT_IMAGE"] = out;

    return true;
  };
  BLOCK_BEGIN_INSTANTIATION(PointDrawer);
  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(PointDrawer, AlgoType::input, BLOCK__POINTDRAWER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(PointDrawer);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__POINTDRAWER_IN_POINTS", "BLOCK__POINTDRAWER_IN_POINTS_HELP");
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__POINTDRAWER_IN_IMAGE", "BLOCK__POINTDRAWER_IN_IMAGE_HELP");
  ADD_PARAMETER(userConstant, Color, "BLOCK__POINTDRAWER_IN_COLOR", "BLOCK__POINTDRAWER_IN_COLOR_HELP");
  ADD_PARAMETER(userConstant, Int, "BLOCK__POINTDRAWER_IN_SIZE", "BLOCK__POINTDRAWER_IN_SIZE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(PointDrawer);
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__POINTDRAWER_OUT_IMAGE", "BLOCK__POINTDRAWER_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(PointDrawer);
  END_BLOCK_PARAMS();

  PointDrawer::PointDrawer() :Block("BLOCK__POINTDRAWER_NAME", true){
    _myInputs["BLOCK__POINTDRAWER_IN_POINTS"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINTDRAWER_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  template<typename T>
  inline void drawCircles(cv::Mat out, T* linePtr, int nbLines, int nbPts, size_t sizeOfALine, cv::Scalar color)
  {
    for (int i = 0; i < nbLines; i++){
      for (int j = 0; j < nbPts; j++){
        if (linePtr[j] >= 1)
          circle(out, cv::Point(j, i), linePtr[j], color, -1);
      }
      linePtr += sizeOfALine;
    }
  }

  bool PointDrawer::run(bool oneShot){
    cv::Mat out = _myInputs["BLOCK__POINTDRAWER_IN_IMAGE"].get<cv::Mat>().clone();
    int size = 1;
    cv::Scalar color = cv::Scalar(255, 255, 255);

    if (!_myInputs["BLOCK__POINTDRAWER_IN_SIZE"].isDefaultValue())
      size = _myInputs["BLOCK__POINTDRAWER_IN_SIZE"].get<int>();
    if (!_myInputs["BLOCK__POINTDRAWER_IN_COLOR"].isDefaultValue())
      color = _myInputs["BLOCK__POINTDRAWER_IN_COLOR"].get<cv::Scalar>();

    cv::Mat points = _myInputs["BLOCK__POINTDRAWER_IN_POINTS"].get<cv::Mat>();
    int nbChanels = points.channels();
    if (nbChanels == 1)
    {//it's a map of point size...
      if (out.empty())
        out = points.clone();
      if (points.size() != out.size())
        throw _STR("BLOCK__POINTDRAWER_ERROR_POINT_SIZE");
      int image_width = points.cols;
      int image_height = points.rows;

      switch (points.depth())
      {
      case CV_8U://char
      {
        drawCircles(out, points.ptr<uchar>(), image_height, image_width, points.step1(), color);
        break;
      }
      case CV_16U://char
      {
        drawCircles(out, points.ptr<uchar>(), image_height, image_width, points.step1(), color);
        break;
      }
      case CV_16S://char
      {
        drawCircles(out, points.ptr<short>(), image_height, image_width, points.step1(), color);
        break;
      }
      case CV_32S://int
      {
        drawCircles(out, points.ptr<int>(), image_height, image_width, points.step1(), color);
        break;
      }
      case CV_32F://float
      {
        drawCircles(out, points.ptr<float>(), image_height, image_width, points.step1(), color);
        break;
      }
      case CV_64F://double
      {
        drawCircles(out, points.ptr<double>(), image_height, image_width, points.step1(), color);
        break;
      }
      }
    }
    else if (nbChanels == 2)
    {//it's a vector of coordinates...
      points = points.reshape(1, points.rows);
      for (int i = 0; i < points.rows; i++)
      {
        switch (points.type())
        {
        case CV_8UC1://char
        {
          uchar* l = points.ptr<uchar>(i);
          circle(out, cv::Point(l[0], l[1]), size, color, -1);
          break;
        }
        case CV_16UC1://char
        {
          ushort* l = points.ptr<ushort>(i);
          circle(out, cv::Point(l[0], l[1]), size, color, -1);
          break;
        }
        case CV_16SC1://char
        {
          short* l = points.ptr<short>(i);
          circle(out, cv::Point(l[0], l[1]), size, color, -1);
          break;
        }
        case CV_32SC1://int
        {
          int* l = points.ptr<int>(i);
          circle(out, cv::Point(l[0], l[1]), size, color, -1);
          break;
        }
        case CV_32FC1://float
        {
          float* l = points.ptr<float>(i);
          circle(out, cv::Point((int)l[0], (int)l[1]), size, color, -1);
          break;
        }
        case CV_64FC1://double
        {
          double* l = points.ptr<double>(i);
          circle(out, cv::Point((int)l[0], (int)l[1]), size, color, -1);
          break;
        }
        }
      }
    }
    else if (nbChanels == 3)
    {//it's a vector of coordinates + point size...
      points = points.reshape(1, points.rows);
      for (int i = 0; i < points.rows; i++)
      {
        switch (points.type())
        {
        case CV_8UC1://char
        {
          uchar* l = points.ptr<uchar>(i);
          circle(out, cv::Point(l[0], l[1]), l[2], color, -1);
          break;
        }
        case CV_16UC1://char
        {
          ushort* l = points.ptr<ushort>(i);
          circle(out, cv::Point(l[0], l[1]), l[2], color, -1);
          break;
        }
        case CV_16SC1://char
        {
          short* l = points.ptr<short>(i);
          circle(out, cv::Point(l[0], l[1]), l[2], color, -1);
          break;
        }
        case CV_32SC1://int
        {
          int* l = points.ptr<int>(i);
          circle(out, cv::Point(l[0], l[1]), l[2], color, -1);
          break;
        }
        case CV_32FC1://float
        {
          float* l = points.ptr<float>(i);
          circle(out, cv::Point((int)l[0], (int)l[1]), l[2], color, -1);
          break;
        }
        case CV_64FC1://double
        {
          double* l = points.ptr<double>(i);
          circle(out, cv::Point((int)l[0], (int)l[1]), l[2], color, -1);
          break;
        }
        }
      }
    }

    _myOutputs["BLOCK__POINTDRAWER_OUT_IMAGE"] = out;

    return true;
  };
};