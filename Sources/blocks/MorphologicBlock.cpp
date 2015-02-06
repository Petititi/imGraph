
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <vector>
#include <opencv2/imgproc.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(MorphoBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(MorphoBlock, AlgoType::imgProcess, BLOCK__MORPHOLOGIC_NAME);
  BEGIN_BLOCK_INPUT_PARAMS(MorphoBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__MORPHOLOGIC_IN_IMAGE", "BLOCK__MORPHOLOGIC_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, Matrix, "BLOCK__MORPHOLOGIC_ELEMENT", "BLOCK__MORPHOLOGIC_ELEMENT_HELP", getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__MORPHOLOGIC_OPERATOR", "BLOCK__MORPHOLOGIC_OPERATOR_HELP", 0);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__MORPHOLOGIC_ITERATIONS", "BLOCK__MORPHOLOGIC_ITERATIONS_HELP", 1);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(MorphoBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__MORPHOLOGIC_OUT_IMAGE", "BLOCK__MORPHOLOGIC_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(MorphoBlock);
  END_BLOCK_PARAMS();

  MorphoBlock::MorphoBlock() :Block("BLOCK__MORPHOLOGIC_NAME"){
    _myInputs["BLOCK__MORPHOLOGIC_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__MORPHOLOGIC_ELEMENT"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__MORPHOLOGIC_OPERATOR"].addValidator({ new ValNeeded(), new ValRange(0, 4) });
    _myInputs["BLOCK__MORPHOLOGIC_ITERATIONS"].addValidator({ new ValPositiv(true) });
  };
  
  bool MorphoBlock::run(bool oneShot){
    if (_myInputs["BLOCK__MORPHOLOGIC_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__MORPHOLOGIC_IN_IMAGE"].get<cv::Mat>();
    cv::Mat element = _myInputs["BLOCK__MORPHOLOGIC_ELEMENT"].get<cv::Mat>();
    int operation = _myInputs["BLOCK__MORPHOLOGIC_OPERATOR"].get<int>();
    int iter = _myInputs["BLOCK__MORPHOLOGIC_ITERATIONS"].get<int>();
    if (iter <= 0)
      iter = 1;
    cv::Mat tmp;
    if (!mat.empty() && !element.empty())
    {
      cv::morphologyEx(mat, tmp, operation, element, cv::Point(-1, -1), iter);
      _myOutputs["BLOCK__MORPHOLOGIC_OUT_IMAGE"] = tmp;
    }

    paramsFullyProcessed();
    return true;
  };
};