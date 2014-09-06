#include <opencv2/imgproc.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>

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
  ADD_PARAMETER(true, Mat, "BLOCK__DEINTERLACE_IN_IMAGE", "BLOCK__DEINTERLACE_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__DEINTERLACE_IN_TYPE", "BLOCK__DEINTERLACE_IN_TYPE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(DeinterlaceBlock);
  ADD_PARAMETER(true, Mat, "BLOCK__DEINTERLACE_OUT_IMAGE", "BLOCK__DEINTERLACE_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  DeinterlaceBlock::DeinterlaceBlock() :Block("BLOCK__DEINTERLACE_NAME"){
    _myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].addValidator({ new ValNeeded() });
  };
  
  bool DeinterlaceBlock::run(){
    if (_myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__DEINTERLACE_IN_IMAGE"].get<cv::Mat>();

    int type = _myInputs["BLOCK__DEINTERLACE_IN_TYPE"].get<int>();

    filter.set("type", 1);

    if (!mat.empty())
    {
      _myOutputs["BLOCK__DEINTERLACE_OUT_IMAGE"] = filter.process(mat);
      renderingDone(false);

      mat = filter.getProducedFrame();//get an other img?
      string output = "DeinterlaceBlock " + boost::lexical_cast<string>(_timestamp)+" ; " + boost::lexical_cast<string>(_work_timestamp)+"\n";
      std::cout << output;

      while (!mat.empty())
      {
        _myOutputs["BLOCK__DEINTERLACE_OUT_IMAGE"] = mat;
        renderingDone(false);
        mat = filter.getProducedFrame();
        string output = "DeinterlaceBlock " + boost::lexical_cast<string>(_timestamp)+" ; " + boost::lexical_cast<string>(_work_timestamp)+"\n";
        std::cout << output;
      }
    }

    return true;
  };


  BLOCK_BEGIN_INSTANTIATION(InterlaceBlock);
  //You can add methods, re implement needed functions... 
  DeInterlaceFilter filter;
  BLOCK_END_INSTANTIATION(InterlaceBlock, AlgoType::videoProcess, BLOCK__INTERLACE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(InterlaceBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Mat, "BLOCK__INTERLACE_IN_IMAGE", "BLOCK__INTERLACE_IN_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__INTERLACE_IN_TYPE", "BLOCK__INTERLACE_IN_TYPE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(InterlaceBlock);
  ADD_PARAMETER(true, Mat, "BLOCK__INTERLACE_OUT_IMAGE", "BLOCK__INTERLACE_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  InterlaceBlock::InterlaceBlock() :Block("BLOCK__INTERLACE_NAME"){
    _myInputs["BLOCK__INTERLACE_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  bool InterlaceBlock::run(){
    if (_myInputs["BLOCK__INTERLACE_IN_IMAGE"].isDefaultValue())
      return false;
    cv::Mat mat = _myInputs["BLOCK__INTERLACE_IN_IMAGE"].get<cv::Mat>();

    int type = _myInputs["BLOCK__INTERLACE_IN_TYPE"].get<int>();

    filter.set("type", 1);

    if (!mat.empty())
    {
      _myOutputs["BLOCK__INTERLACE_OUT_IMAGE"] = filter.process(mat);
      renderingDone(false);

      mat = filter.getProducedFrame();//get an other img?
      string output = "InterlaceBlock " + boost::lexical_cast<string>(_timestamp)+" ; " + boost::lexical_cast<string>(_work_timestamp)+"\n";
      std::cout << output;

      while (!mat.empty())
      {
        _myOutputs["BLOCK__INTERLACE_OUT_IMAGE"] = mat;
        renderingDone(false);
        mat = filter.getProducedFrame();
        string output = "InterlaceBlock " + boost::lexical_cast<string>(_timestamp)+" ; " + boost::lexical_cast<string>(_work_timestamp)+"\n";
        std::cout << output;
      }
    }

    return true;
  };
};