
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(WriteVideo);
  //You can add methods, re implement needed functions...
  cv::VideoWriter vr;
  BLOCK_END_INSTANTIATION(WriteVideo, AlgoType::output, BLOCK__WRITE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(WriteVideo);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__WRITE_IN_IMAGE", "BLOCK__WRITE_IN_IMAGE_HELP");
  ADD_PARAMETER(false, FilePath, "BLOCK__WRITE_IN_FILENAME", "BLOCK__WRITE_IN_FILENAME_HELP");
  ADD_PARAMETER_FULL(false, Float, "BLOCK__WRITE_IN_FPS", "BLOCK__WRITE_IN_FPS_HELP", 25.);
  ADD_PARAMETER_FULL(false, String, "BLOCK__WRITE_IN_CODEC", "BLOCK__WRITE_IN_CODEC_HELP", "XVID");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(WriteVideo);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(WriteVideo);
  END_BLOCK_PARAMS();

  WriteVideo::WriteVideo() :Block("BLOCK__WRITE_NAME"){
    _myInputs["BLOCK__WRITE_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__WRITE_IN_FILENAME"].addValidator({ new ValNeeded() });
  };

  bool WriteVideo::run(bool oneShot){
    cv::Mat out = _myInputs["BLOCK__WRITE_IN_IMAGE"].get<cv::Mat>(true);
    if (out.empty())
      return false;
    if (!vr.isOpened() 
      || _myInputs["BLOCK__WRITE_IN_FILENAME"].isNew()
      || _myInputs["BLOCK__WRITE_IN_CODEC"].isNew()
      || _myInputs["BLOCK__WRITE_IN_FPS"].isNew())
    {
      if (vr.isOpened())
        vr.release();
      string filename = _myInputs["BLOCK__WRITE_IN_FILENAME"].get<std::string>(true);
      string codecName = _myInputs["BLOCK__WRITE_IN_CODEC"].get<std::string>(true);
      int codecCode = -1;
      if (codecName.length() == 4)
        codecCode = cv::VideoWriter::fourcc(codecName[0], codecName[1], codecName[2], codecName[3]);
      if (codecName.empty())
        codecCode = 0;
      vr.open(filename, codecCode, _myInputs["BLOCK__WRITE_IN_FPS"].get<double>(true), out.size());
    }
    vr.write(out);

    return true;
  };
};