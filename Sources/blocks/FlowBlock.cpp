
#include <vector>

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <Opencv2/imgproc.hpp>
#include <opencv2/superres/optical_flow.hpp>
#include <opencv2/ocl/ocl.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using namespace lsis_org;
using namespace cv;
using namespace cv::superres;
using std::vector;
using std::string;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(OpticFlowBlock);
  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(OpticFlowBlock, AlgoType::videoProcess, BLOCK__OPTICFLOW_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(OpticFlowBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE1", "BLOCK__OPTICFLOW_IN_IMAGE1_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE2", "BLOCK__OPTICFLOW_IN_IMAGE2_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__OPTICFLOW_IN_METHOD", "BLOCK__OPTICFLOW_IN_METHOD_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(OpticFlowBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_OUT_IMAGE", "BLOCK__OPTICFLOW_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  OpticFlowBlock::OpticFlowBlock() :Block("BLOCK__OPTICFLOW_NAME"){
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__OPTICFLOW_IN_METHOD"].addValidator({ new ValRange(0,2) });
  };
  
  bool OpticFlowBlock::run(){

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
    Ptr<DenseOpticalFlowExt> optic;
    switch (method)
    {
    case 1:
      optic = createOptFlow_Farneback_OCL();
      break;
    case 2:
      optic = createOptFlow_DualTVL1_OCL();
      break;
    default:
      optic = createOptFlow_Simple();
      break;
    }
    ocl::oclMat ocltmp1(src), ocltmp2(dest), flow1, flow2;

    optic->calc(ocltmp1, ocltmp2, flow1, flow2);

    Mat finalFlow, printFlow, splitedFlow[2];
    splitedFlow[0] = flow1;
    splitedFlow[1] = flow2;

    optic.release();

    merge(splitedFlow, 2, finalFlow);
    _myOutputs["BLOCK__OPTICFLOW_OUT_IMAGE"] = finalFlow;
    renderingDone();
    return true;
  };
};