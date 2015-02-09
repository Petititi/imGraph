
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
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(DeinterlaceBlock);
  //You can add methods, re implement needed functions... 
  DeInterlaceFilter filter;
  BLOCK_END_INSTANTIATION(DeinterlaceBlock, AlgoType::videoProcess, BLOCK__DEINTERLACE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(DeinterlaceBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__DEINTERLACE_IN_IMAGE", "BLOCK__DEINTERLACE_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__DEINTERLACE_IN_TYPE", "BLOCK__DEINTERLACE_IN_TYPE_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(DeinterlaceBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__DEINTERLACE_OUT_IMAGE", "BLOCK__DEINTERLACE_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(DeinterlaceBlock);
  END_BLOCK_PARAMS();

  DeinterlaceBlock::DeinterlaceBlock() :Block("BLOCK__DEINTERLACE_NAME", false){
    _myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].addValidator({ new ValNeeded() });
  };
  
  bool DeinterlaceBlock::run(bool oneShot){
    if (_myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].isDefaultValue())
      return false;

    if (_myInputs["BLOCK__DEINTERLACE_IN_TYPE"].isNew())
    {
      _myInputs["BLOCK__DEINTERLACE_IN_TYPE"].setNew(false);
      int type = _myInputs["BLOCK__DEINTERLACE_IN_TYPE"].get<int>();
      filter.set("type", type);
    }

    cv::Mat mat;
    if (filter.canProduceFrame())
      mat = filter.getProducedFrame();
    else
      mat = filter.process(_myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].get<cv::Mat>());

    _myOutputs["BLOCK__DEINTERLACE_OUT_IMAGE"] = mat;

    if (!filter.canProduceFrame())
      paramsFullyProcessed();


    return true;
  };


  BLOCK_BEGIN_INSTANTIATION(InterlaceBlock);
  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(InterlaceBlock, AlgoType::videoProcess, BLOCK__SKIP_FRAME_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(InterlaceBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__SKIP_FRAME_IN_IMAGE", "BLOCK__SKIP_FRAME_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__SKIP_FRAME_IN_TYPE", "BLOCK__SKIP_FRAME_IN_TYPE_HELP", 1);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(InterlaceBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__SKIP_FRAME_OUT_IMAGE", "BLOCK__SKIP_FRAME_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(InterlaceBlock);
  END_BLOCK_PARAMS();

  InterlaceBlock::InterlaceBlock() :Block("BLOCK__SKIP_FRAME_NAME", true){
    _myInputs["BLOCK__SKIP_FRAME_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  bool InterlaceBlock::run(bool oneShot){
    if (_myInputs["BLOCK__SKIP_FRAME_IN_IMAGE"].isDefaultValue())
      return false;

    int nbSkip = _myInputs["BLOCK__SKIP_FRAME_IN_TYPE"].get<int>();
    int nbFrame = 0;
    if (!oneShot)
    {
      while (nbFrame < nbSkip)
      {
        nbFrame++;
        _myInputs["BLOCK__SKIP_FRAME_IN_IMAGE"].update();//discard current value and get a new one
      }
    }

    cv::Mat mat = _myInputs["BLOCK__SKIP_FRAME_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      _myOutputs["BLOCK__SKIP_FRAME_OUT_IMAGE"] = mat;
    }

    return true;
  };
};