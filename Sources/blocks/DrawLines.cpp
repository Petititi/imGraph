
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace lsis_org;
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
  ADD_PARAMETER(true, Matrix, "BLOCK__LINEDRAWER_IN_LINES", "BLOCK__LINEDRAWER_IN_LINES_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__LINEDRAWER_IN_IMAGE", "BLOCK__LINEDRAWER_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Color, "BLOCK__LINEDRAWER_IN_COLOR", "BLOCK__LINEDRAWER_IN_COLOR_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__LINEDRAWER_IN_SIZE", "BLOCK__LINEDRAWER_IN_SIZE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(LineDrawer);
  ADD_PARAMETER(true, Matrix, "BLOCK__LINEDRAWER_OUT_IMAGE", "BLOCK__LINEDRAWER_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  LineDrawer::LineDrawer() :Block("BLOCK__LINEDRAWER_NAME"){
    _myInputs["BLOCK__LINEDRAWER_IN_LINES"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__LINEDRAWER_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__LINEDRAWER_IN_SIZE"].addValidator({ new ValPositiv(true) });
  };
  
  bool LineDrawer::run(){
    cv::Mat out = _myInputs["BLOCK__LINEDRAWER_IN_IMAGE"].get<cv::Mat>().clone();
    int size = 1;
    cv::Scalar color = cv::Scalar(255, 255, 255);

    if (!_myInputs["BLOCK__LINEDRAWER_IN_SIZE"].isDefaultValue())
      size = _myInputs["BLOCK__LINEDRAWER_IN_SIZE"].get<int>();
    if (!_myInputs["BLOCK__LINEDRAWER_IN_COLOR"].isDefaultValue())
      color = _myInputs["BLOCK__LINEDRAWER_IN_COLOR"].get<cv::Scalar>();

    cv::Mat lines = _myInputs["BLOCK__LINEDRAWER_IN_LINES"].get<cv::Mat>();
    int nbChanels = lines.channels();
    if (nbChanels!=1)
      lines = lines.reshape(1, lines.rows);
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
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
      if (type == CV_64FC1)//double
      {
        double* l = lines.ptr<double>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, size);
      }
    }

    _myOutputs["BLOCK__LINEDRAWER_OUT_IMAGE"] = out;
    renderingDone();
    return true;
  };
};