
#include <vector>

#include "Block.h"
using namespace lsis_org;
using std::vector;

namespace charliesoft
{
  BLOCK_INSTANTIATE(BlockLoader, AlgoType::input, BLOCK__INPUT_NAME);

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


};