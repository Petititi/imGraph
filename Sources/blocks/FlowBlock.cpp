
#include <vector>

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/lexical_cast.hpp>
#include <opencv2/opencv.hpp>
#include <Opencv2/imgproc.hpp>
#include <Opencv2/optflow.hpp>
#include <opencv2/superres/optical_flow.hpp>
#include <opencv2/core/ocl.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using namespace charliesoft;
using namespace cv;
using namespace cv::superres;
using std::vector;
using std::string;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(OpticFlowBlock);
  //You can add methods, re implement needed functions...
  cv::Mat computeOCL_CPU(cv::Mat im1, cv::Mat im2, int method);
  BLOCK_END_INSTANTIATION(OpticFlowBlock, AlgoType::videoProcess, BLOCK__OPTICFLOW_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(OpticFlowBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE1", "BLOCK__OPTICFLOW_IN_IMAGE1_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE2", "BLOCK__OPTICFLOW_IN_IMAGE2_HELP");
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__OPTICFLOW_IN_METHOD", "BLOCK__OPTICFLOW_IN_METHOD_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(OpticFlowBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_OUT_IMAGE", "BLOCK__OPTICFLOW_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(OpticFlowBlock);
  END_BLOCK_PARAMS();

  OpticFlowBlock::OpticFlowBlock() :Block("BLOCK__OPTICFLOW_NAME", true){
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__OPTICFLOW_IN_METHOD"].addValidator({ new ValNeeded(), new ValRange(0, 2) });
  };

  cv::Mat OpticFlowBlock::computeOCL_CPU(cv::Mat src, cv::Mat dest, int method)
  {
    Mat finalFlow;
    if (method == 1 || method == 2)
    {
      Ptr<DenseOpticalFlowExt> optic;
      if (method == 1)
        optic = createOptFlow_Farneback();
      else
        optic = superres::createOptFlow_DualTVL1();

      Mat flow1, flow2;
      optic->calc(src, dest, flow1, flow2);
      optic.release();

      Mat printFlow, splitedFlow[2];
      splitedFlow[0] = flow1;
      splitedFlow[1] = flow2;

      merge(splitedFlow, 2, finalFlow);
    }
    else
    {
      Ptr<DenseOpticalFlow> optic;
      if (method == 3)
        optic = optflow::createOptFlow_DeepFlow();
      else
        optic = optflow::createOptFlow_SimpleFlow();

      optic->calc(src, dest, finalFlow);
      optic.release();
    }
    return finalFlow;
  }

  bool OpticFlowBlock::run(bool oneShot){
    cv::Mat src = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].get<cv::Mat>();
    cv::Mat dest = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].get<cv::Mat>();
    if (src.channels() != 1)
      cvtColor(src, src, COLOR_RGB2GRAY);
    else
      src = src.clone();

    if (dest.channels() != 1)
      cvtColor(dest, dest, COLOR_RGB2GRAY);
    else
      dest = dest.clone();

    int method = 0;
    if (!_myInputs["BLOCK__OPTICFLOW_IN_METHOD"].isDefaultValue())
      method = _myInputs["BLOCK__OPTICFLOW_IN_METHOD"].get<int>();

    Mat finalFlow;
    finalFlow = computeOCL_CPU(src, dest, method);
    _myOutputs["BLOCK__OPTICFLOW_OUT_IMAGE"] = finalFlow;

    return true;
  };
};