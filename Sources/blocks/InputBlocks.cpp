
#include <vector>
#include <boost/filesystem.hpp>

#include "Block.h"
#include "ParamValidator.h"
using std::string;
using std::vector;

#include "InputProcessor.h"
using namespace lsis_org;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockLoader);
  //You can add methods, attributs, reimplement needed functions...
protected:
  InputProcessor processor_;
  BLOCK_END_INSTANTIATION(BlockLoader, AlgoType::input, BLOCK__INPUT_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockLoader);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(false, FilePath,"BLOCK__INPUT_IN_FILE",         "BLOCK__INPUT_IN_FILE_HELP");
  ADD_PARAMETER(false, Boolean, "BLOCK__INPUT_IN_GREY",         "BLOCK__INPUT_IN_GREY_HELP");
  ADD_PARAMETER(false, Boolean, "BLOCK__INPUT_IN_COLOR",        "BLOCK__INPUT_IN_COLOR_HELP");
  ADD_PARAMETER(false, Int,     "BLOCK__INPUT_INOUT_WIDTH",     "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(false, Int,     "BLOCK__INPUT_INOUT_HEIGHT",    "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(false, Int,     "BLOCK__INPUT_INOUT_POS_FRAMES","BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
  ADD_PARAMETER(false, Int,     "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP");
  END_BLOCK_PARAMS();
  
  BEGIN_BLOCK_OUTPUT_PARAMS(BlockLoader);
  ADD_PARAMETER(true, Float,  "BLOCK__INPUT_OUT_IMAGE",       "BLOCK__INPUT_OUT_IMAGE_HELP");
  ADD_PARAMETER(false, Float, "BLOCK__INPUT_OUT_FRAMERATE",   "BLOCK__INPUT_OUT_FRAMERATE_HELP");
  ADD_PARAMETER(false, Int,   "BLOCK__INPUT_INOUT_WIDTH",     "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(false, Int,   "BLOCK__INPUT_INOUT_HEIGHT",    "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(false, Int,   "BLOCK__INPUT_INOUT_POS_FRAMES","BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
  ADD_PARAMETER(false, Float, "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP");
  ADD_PARAMETER(false, Int,   "BLOCK__INPUT_OUT_FORMAT",      "BLOCK__INPUT_OUT_FORMAT_HELP");
  END_BLOCK_PARAMS();

  BlockLoader::BlockLoader() :Block("BLOCK__INPUT_NAME"){
    myInputs_["BLOCK__INPUT_IN_FILE"].addValidator({ new ValNeeded(), new ValFileExist() });
    myInputs_["BLOCK__INPUT_INOUT_WIDTH"].addValidator({ new ValPositiv(true) });
    myInputs_["BLOCK__INPUT_INOUT_HEIGHT"].addValidator({ new ValPositiv(true) });
    myInputs_["BLOCK__INPUT_INOUT_POS_FRAMES"].addValidator({ new ValPositiv(false) });
    myInputs_["BLOCK__INPUT_INOUT_POS_RATIO"].addValidator({ new ValRange(0, 1) });
    myInputs_["BLOCK__INPUT_IN_GREY"].addValidator({ new ValExclusif(myInputs_["BLOCK__INPUT_IN_COLOR"]) });
    myInputs_["BLOCK__INPUT_IN_COLOR"].addValidator({ new ValExclusif(myInputs_["BLOCK__INPUT_IN_GREY"]) });
  };

  bool BlockLoader::run(){
    if (!myInputs_["BLOCK__INPUT_IN_FILE"].isDefaultValue())
    {
      string fileName = myInputs_["BLOCK__INPUT_IN_FILE"].get<string>();
      if (!processor_.setInputSource(fileName))
      {
        error_msg_ = (my_format(_STR("BLOCK__INPUT_IN_FILE_PROBLEM")) % fileName).str();
        return false;
      }
    }
    if (!myInputs_["BLOCK__INPUT_IN_GREY"].isDefaultValue())
      if (myInputs_["BLOCK__INPUT_IN_GREY"].get<bool>())
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 0);

    if (!myInputs_["BLOCK__INPUT_IN_COLOR"].isDefaultValue())
      if (myInputs_["BLOCK__INPUT_IN_COLOR"].get<bool>())
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 1);

    if (!myInputs_["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH, 
      myInputs_["BLOCK__INPUT_INOUT_WIDTH"].get<int>());

    if (!myInputs_["BLOCK__INPUT_INOUT_HEIGHT"].isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH, 
      myInputs_["BLOCK__INPUT_INOUT_HEIGHT"].get<int>());

    if (!myInputs_["BLOCK__INPUT_INOUT_POS_FRAMES"].isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_POS_FRAMES, 
      myInputs_["BLOCK__INPUT_INOUT_POS_FRAMES"].get<double>());

    if (!myInputs_["BLOCK__INPUT_INOUT_POS_RATIO"].isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_POS_AVI_RATIO, 
      myInputs_["BLOCK__INPUT_INOUT_POS_RATIO"].get<double>());

    //now set outputs:
    cv::Mat frame = processor_.getFrame();
    double fps = MAX(1, processor_.getProperty(cv::CAP_PROP_FPS));
    while (!frame.empty())
    {
      myOutputs_["BLOCK__INPUT_OUT_IMAGE"] = frame;
      myOutputs_["BLOCK__INPUT_OUT_FRAMERATE"] = fps;
      myOutputs_["BLOCK__INPUT_INOUT_WIDTH"] = frame.cols;
      myOutputs_["BLOCK__INPUT_INOUT_HEIGHT"] = frame.rows;
      myOutputs_["BLOCK__INPUT_INOUT_POS_FRAMES"] = processor_.getProperty(cv::CAP_PROP_POS_FRAMES);
      myOutputs_["BLOCK__INPUT_INOUT_POS_RATIO"] = processor_.getProperty(cv::CAP_PROP_POS_AVI_RATIO);
      myOutputs_["BLOCK__INPUT_OUT_FORMAT"] = frame.type();

      //wait corresponding ms in order to keep fps:
      boost::this_thread::sleep(boost::posix_time::milliseconds(1. / fps*1000.));
      frame = processor_.getFrame();
    }

    return true;
  };

};