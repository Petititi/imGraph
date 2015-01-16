
#include <vector>

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/lexical_cast.hpp>
#include <opencv2/opencv.hpp>
//#include <Opencv2/imgproc.hpp>
#include <opencv2/superres/optical_flow.hpp>
#include <opencv2/core/ocl.hpp>
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
  Ptr<DenseOpticalFlow> optic;

    public:
        ~OpticFlowBlock();

  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(OpticFlowBlock, AlgoType::videoProcess, BLOCK__OPTICFLOW_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(OpticFlowBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE1", "BLOCK__OPTICFLOW_IN_IMAGE1_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_IN_IMAGE2", "BLOCK__OPTICFLOW_IN_IMAGE2_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(OpticFlowBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__OPTICFLOW_OUT_IMAGE", "BLOCK__OPTICFLOW_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(OpticFlowBlock);
  END_BLOCK_PARAMS();

  OpticFlowBlock::OpticFlowBlock() :Block("BLOCK__OPTICFLOW_NAME"){
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    optic = cv::createOptFlow_DualTVL1();
  };

  OpticFlowBlock::~OpticFlowBlock(){
      optic.release();
  };

  bool OpticFlowBlock::run(bool oneShot){
    cv::Mat src = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE1"].get<cv::Mat>(true);
    cv::Mat dest = _myInputs["BLOCK__OPTICFLOW_IN_IMAGE2"].get<cv::Mat>(true);
    if (src.channels() != 1)
      cvtColor(src, src, COLOR_RGB2GRAY);

    if (dest.channels() != 1)
      cvtColor(dest, dest, COLOR_RGB2GRAY);
    
    cv::Mat flow;
    optic->calc(src, dest, flow);

    _myOutputs["BLOCK__OPTICFLOW_OUT_IMAGE"] = flow;
    return true;
  };
};