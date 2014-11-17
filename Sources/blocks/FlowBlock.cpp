
#include <vector>

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/lexical_cast.hpp>
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
  cv::Mat computeOpenCL(cv::Mat im1, cv::Mat im2, int method);
  cv::Mat computeCUDA(cv::Mat im1, cv::Mat im2, int method);
  cv::Mat computeCPU(cv::Mat im1, cv::Mat im2, int method);
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

  OpticFlowBlock::OpticFlowBlock() :Block("BLOCK__OPTICFLOW_NAME"){
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__OPTICFLOW_IN_METHOD"].addValidator({ new ValNeeded(), new ValRange(0, 2) });
  };

  cv::Mat OpticFlowBlock::computeOpenCL(cv::Mat src, cv::Mat dest, int method)
  {
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
      optic = createOptFlow_PyrLK_OCL();
      break;
    }
    ocl::oclMat ocltmp1(src), ocltmp2(dest), flow1, flow2;

    optic->calc(ocltmp1, ocltmp2, flow1, flow2);

    Mat finalFlow, printFlow, splitedFlow[2];
    splitedFlow[0] = flow1;
    splitedFlow[1] = flow2;

    optic.release();

    merge(splitedFlow, 2, finalFlow);
    return finalFlow;
  }
  cv::Mat OpticFlowBlock::computeCUDA(cv::Mat im1, cv::Mat im2, int method)
  {
    return im1;
  }
  cv::Mat OpticFlowBlock::computeCPU(cv::Mat src, cv::Mat dest, int method)
  {
    Ptr<DenseOpticalFlowExt> optic;
    switch (method)
    {
    case 1:
      optic = createOptFlow_Farneback();
      break;
    case 2:
      optic = createOptFlow_DualTVL1();
      break;
    default:
      optic = createOptFlow_Simple();
      break;
    }

    cv::Mat flow1, flow2;
    optic->calc(src, dest, flow1, flow2);

    Mat finalFlow, printFlow, splitedFlow[2];
    splitedFlow[0] = flow1;
    splitedFlow[1] = flow2;

    optic.release();

    merge(splitedFlow, 2, finalFlow);
    return finalFlow;
  }

  bool OpticFlowBlock::run(bool oneShot){
    cv::Mat src = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].get<cv::Mat>(true);
    cv::Mat dest = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].get<cv::Mat>(true);
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
      method = _myInputs["BLOCK__OPTICFLOW_IN_METHOD"].get<int>(true);


    //is Opencl OK?
    cv::ocl::DevicesInfo devices;
    cv::ocl::getOpenCLDevices(devices);
    bool opencl = false;
    for (size_t i = 0; i < devices.size(); i++)
    {
      if (devices[i]->deviceVersionMajor > 1)
        opencl = true;
      else
        if (devices[i]->deviceVersionMajor == 1 && devices[i]->deviceVersionMinor >= 1)
          opencl = true;
    }
    Mat finalFlow;
    if (opencl)
      finalFlow = computeOpenCL(src, dest, method);
    else
      finalFlow = computeCPU(src, dest, method);
    _myOutputs["BLOCK__OPTICFLOW_OUT_IMAGE"] = finalFlow;
    return true;
  };
};