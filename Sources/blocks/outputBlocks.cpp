
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>
#include <sstream>
#include <boost/lexical_cast.hpp>

#include "OpenCV_filter.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "view/GuiReceiver.h"
#include "Block.h"
#include "ParamValidator.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockShow);
  //You can add methods, re implement needed functions...
  NormalizeFilter filter;
  BLOCK_END_INSTANTIATION(BlockShow, AlgoType::output, BLOCK__OUTPUT_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockShow);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__OUTPUT_IN_IMAGE", "BLOCK__OUTPUT_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, String, "BLOCK__OUTPUT_IN_WIN_NAME", "BLOCK__OUTPUT_IN_WIN_NAME_HELP", "ImShow");
  ADD_PARAMETER_FULL(false, Boolean, "BLOCK__OUTPUT_IN_NORMALIZE", "BLOCK__OUTPUT_IN_NORMALIZE_HELP", false);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockShow);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockShow);
  END_BLOCK_PARAMS();

  BlockShow::BlockShow() :Block("BLOCK__OUTPUT_NAME", true){
    _myInputs["BLOCK__OUTPUT_IN_IMAGE"].addValidator({ new ValNeeded() });
  };
  
  bool BlockShow::run(bool oneShot){
    if (_myInputs["BLOCK__OUTPUT_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__OUTPUT_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      if (_myInputs["BLOCK__OUTPUT_IN_NORMALIZE"].get<bool>())
        mat = filter.process(mat);

      ::imshow(_myInputs["BLOCK__OUTPUT_IN_WIN_NAME"].get<string>(), mat);
    }

    return true;
  };
};