
#include <vector>
#include <boost/filesystem.hpp>

#include "Block.h"
using std::string;
using std::vector;

#include "InputProcessor.h"
using namespace lsis_org;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockLoader);
  //You can add methods, attributs, reimplement needed functions...
  //Here we need validateParams:
  virtual bool validateParams(std::string param, const ParamValue&);
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

  BlockLoader::BlockLoader() :Block("BLOCK__INPUT_NAME"){};

  bool BlockLoader::run(){
    if (myInputs_["BLOCK__INPUT_IN_FILE"].isNew())
    {
      string fileName = myInputs_["BLOCK__INPUT_IN_FILE"].get<string>(true);
      if (!processor_.setInputSource(fileName))
      {
        error_msg_ = (my_format(_STR("BLOCK__INPUT_IN_FILE_PROBLEM")) % fileName).str();
        return false;
      }
    }
    if (myInputs_["BLOCK__INPUT_IN_GREY"].isNew())
      if (myInputs_["BLOCK__INPUT_IN_GREY"].get<bool>(true))
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 0);

    if (myInputs_["BLOCK__INPUT_IN_COLOR"].isNew())
      if (myInputs_["BLOCK__INPUT_IN_COLOR"].get<bool>(true))
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 1);

    if (myInputs_["BLOCK__INPUT_INOUT_WIDTH"].isNew())
      processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH, 
      myInputs_["BLOCK__INPUT_INOUT_WIDTH"].get<int>(true));

    if (myInputs_["BLOCK__INPUT_INOUT_HEIGHT"].isNew())
      processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH, 
      myInputs_["BLOCK__INPUT_INOUT_HEIGHT"].get<int>(true));

    if (myInputs_["BLOCK__INPUT_INOUT_POS_FRAMES"].isNew())
      processor_.setProperty(cv::CAP_PROP_POS_FRAMES, 
      myInputs_["BLOCK__INPUT_INOUT_POS_FRAMES"].get<double>(true));

    if (myInputs_["BLOCK__INPUT_INOUT_POS_RATIO"].isNew())
      processor_.setProperty(cv::CAP_PROP_POS_AVI_RATIO, 
      myInputs_["BLOCK__INPUT_INOUT_POS_RATIO"].get<double>(true));

    //now set outputs:
    cv::Mat frame = processor_.getFrame();
    myOutputs_["BLOCK__INPUT_OUT_IMAGE"] = frame;
    myOutputs_["BLOCK__INPUT_OUT_FRAMERATE"] = processor_.getProperty(cv::CAP_PROP_FPS);
    myOutputs_["BLOCK__INPUT_INOUT_WIDTH"] = frame.cols;
    myOutputs_["BLOCK__INPUT_INOUT_HEIGHT"] = frame.rows;
    myOutputs_["BLOCK__INPUT_INOUT_POS_FRAMES"] = processor_.getProperty(cv::CAP_PROP_POS_FRAMES);
    myOutputs_["BLOCK__INPUT_INOUT_POS_RATIO"] = processor_.getProperty(cv::CAP_PROP_POS_AVI_RATIO);
    myOutputs_["BLOCK__INPUT_OUT_FORMAT"] = frame.type();

    return true;
  };

  bool BlockLoader::validateParams(std::string paramName, const ParamValue& value){
    bool isOk = true;
    error_msg_ = "";
    //we need BLOCK__INPUT_IN_FILE to be set:
    if (paramName.compare("BLOCK__INPUT_IN_FILE") == 0)
    {
      if (value.isDefaultValue())
      {
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_NEEDED")) % _STR("BLOCK__OUTPUT_IN_IMAGE")).str() + "<br/>";
      }
      else
      {
        if (!boost::filesystem::exists(value.toString()))    // does p actually exist?
        {
          isOk = false;
          error_msg_ += (my_format(_STR("BLOCK__INPUT_IN_FILE_NOT_FOUND")) % value.toString()).str() + "<br/>";
        }
      }
    }
    //other are not required but some contraints exists:
    if (paramName.compare("BLOCK__INPUT_IN_GREY") == 0)
    {
      if (value == myInputs_["BLOCK__INPUT_IN_COLOR"] && value.get_const<bool>())
      {//parameters exclusif:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_EXCLUSIF")) % _STR("BLOCK__INPUT_IN_GREY") % _STR("BLOCK__INPUT_IN_COLOR")).str() + "<br/>";
      }
    }
    if (paramName.compare("BLOCK__INPUT_IN_COLOR") == 0)
    {
      if (value == myInputs_["BLOCK__INPUT_IN_GREY"] && value.get_const<bool>())
      {//parameters exclusif:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_EXCLUSIF")) % _STR("BLOCK__INPUT_IN_GREY") % _STR("BLOCK__INPUT_IN_COLOR")).str() + "<br/>";
      }
    }
    if (paramName.compare("BLOCK__INPUT_INOUT_WIDTH") == 0)
    {
      if (value <= 0)
      {//parameters wrong:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_ONLY_POSITIF_STRICT")) % _STR("BLOCK__INPUT_INOUT_WIDTH")).str() + "<br/>";
      }
    }
    if (paramName.compare("BLOCK__INPUT_INOUT_HEIGHT") == 0)
    {
      if (value <= 0)
      {//parameters wrong:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_ONLY_POSITIF_STRICT")) % _STR("BLOCK__INPUT_INOUT_HEIGHT")).str() + "<br/>";
      }
    }
    if (paramName.compare("BLOCK__INPUT_INOUT_POS_FRAMES") == 0)
    {
      if (value < 0)
      {//parameters wrong:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_ONLY_POSITIF")) % _STR("BLOCK__INPUT_INOUT_POS_FRAMES")).str() + "<br/>";
      }
    }
    if (paramName.compare("BLOCK__INPUT_INOUT_POS_RATIO") == 0)
    {
      if (value < 0 || value > 1)
      {//parameters wrong:
        isOk = false;
        error_msg_ += (my_format(_STR("ERROR_PARAM_VALUE_BETWEEN")) % _STR("BLOCK__INPUT_INOUT_POS_RATIO") % 0. % 1.).str() + "<br/>";
      }
    }
    return isOk;
  };


};