
#include <vector>

#include "Block.h"
#include "window_QT.h"
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
  ADD_PARAMETER(true, Mat, "BLOCK__LINEDRAWER_IN_LINES", "BLOCK__LINEDRAWER_IN_LINES_HELP");
  ADD_PARAMETER(true, Int, "BLOCK__LINEDRAWER_IN_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(true, Int, "BLOCK__LINEDRAWER_IN_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(LineDrawer);
  ADD_PARAMETER(true, Mat, "BLOCK__LINEDRAWER_OUT_IMAGE", "BLOCK__LINEDRAWER_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  LineDrawer::LineDrawer() :Block("BLOCK__LINEDRAWER_NAME"){
    _myInputs["BLOCK__LINEDRAWER_IN_LINES"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__LINEDRAWER_IN_WIDTH"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__LINEDRAWER_IN_HEIGHT"].addValidator({ new ValNeeded(), new ValPositiv(true) });
  };
  
  bool LineDrawer::run(){
    cv::Mat out = cv::Mat::zeros(_myInputs["BLOCK__LINEDRAWER_IN_HEIGHT"].get<int>(),
      _myInputs["BLOCK__LINEDRAWER_IN_WIDTH"].get<int>(),
      CV_8UC1);

    cv::Mat lines = _myInputs["BLOCK__LINEDRAWER_IN_LINES"].get<cv::Mat>();
    int nbChanels = lines.channels();
    if (nbChanels!=1)
      lines = lines.reshape(1, lines.rows);
    for (int i = 0; i < lines.rows; i++)
    {
      int type = lines.type();
      if (type == CV_32SC1)//int
      {
        int* l = lines.ptr<int>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
          cv::Scalar(255), 1);
      }
      if (type == CV_32FC1)//float
      {
        float* l = lines.ptr<float>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
          cv::Scalar(255), 1);
      }
      if (type == CV_64FC1)//double
      {
        double* l = lines.ptr<double>(i);
        line(out, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
          cv::Scalar(255), 1);
      }
    }

    _myOutputs["BLOCK__LINEDRAWER_OUT_IMAGE"] = out;
    renderingDone();
    return true;
  };
};