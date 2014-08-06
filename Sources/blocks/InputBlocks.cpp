
#include <vector>
#include <boost/filesystem.hpp>

#include "Block.h"
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

  BlockLoader::BlockLoader() :Block("BLOCK__INPUT_NAME"){};
  
  void BlockLoader::run(){
  };

  std::vector<ParamDefinition> BlockLoader::getListParams(){
    std::vector<ParamDefinition> output;
    output.push_back(ParamDefinition(false, FilePath,
      "BLOCK__INPUT_IN_FILE", "BLOCK__INPUT_IN_FILE_HELP"));
    output.push_back(ParamDefinition(false, Boolean,
      "BLOCK__INPUT_IN_GREY", "BLOCK__INPUT_IN_GREY_HELP"));
    output.push_back(ParamDefinition(false, Boolean,
      "BLOCK__INPUT_IN_COLOR", "BLOCK__INPUT_IN_COLOR_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP"));
    return output;
  };
  std::vector<ParamDefinition> BlockLoader::getListOutputs(){
    std::vector<ParamDefinition> output;
    output.push_back(ParamDefinition(true, Float,
      "BLOCK__INPUT_OUT_IMAGE", "BLOCK__INPUT_OUT_IMAGE_HELP"));
    output.push_back(ParamDefinition(false, Float,
      "BLOCK__INPUT_OUT_FRAMERATE", "BLOCK__INPUT_OUT_FRAMERATE_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP"));
    output.push_back(ParamDefinition(false, Float,
      "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP"));
    output.push_back(ParamDefinition(false, Int,
      "BLOCK__INPUT_OUT_FORMAT", "BLOCK__INPUT_OUT_FORMAT_HELP"));
    return output;
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