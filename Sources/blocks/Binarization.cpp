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
  ADD_PARAMETER(true, Matrix, "BLOCK__BINARIZE_IN_IMAGE", "BLOCK__BINARIZE_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(true, Int, "BLOCK__BINARIZE_IN_THRESHOLD", "BLOCK__BINARIZE_IN_THRESHOLD_HELP", 128);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BinarizeBlock);
  ADD_PARAMETER(true, AnyType, "BLOCK__BINARIZE_OUT_IMAGE", "BLOCK__BINARIZE_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BinarizeBlock);
  END_BLOCK_PARAMS();

  BinarizeBlock::BinarizeBlock() :Block("BLOCK__BINARIZE_NAME", true){
    _myInputs["BLOCK__BINARIZE_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__BINARIZE_IN_THRESHOLD"].addValidator({ new ValNeeded() });
  };

  bool BinarizeBlock::run(bool oneShot){
    if (_myInputs["BLOCK__BINARIZE_IN_IMAGE"].isDefaultValue())
      return false;

    int threshold = _myInputs["BLOCK__BINARIZE_IN_THRESHOLD"].get<int>();
    
    cv::Mat mat = _myInputs["BLOCK__BINARIZE_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      cv::Mat output = MatrixConvertor::adjustChannels(mat.clone(), 1);

      if (threshold < 0)
      {
        cv::threshold(output, output, threshold, 255, cv::THRESH_OTSU);
      }
      else
      {
        cv::threshold(output, output, threshold, 255, cv::THRESH_BINARY);
      }

      _myOutputs["BLOCK__BINARIZE_OUT_IMAGE"] = output;
    }
    return !mat.empty();
  };
};