#include "InputBlocks.h"
#include <vector>

using namespace lsis_org;
using std::vector;

namespace charliesoft
{
  BLOCK_INSTANTIATE(BlockLoader, AlgoType::input, BLOCK__INPUT_NAME);

  std::vector<std::string> BlockLoader::getListParams(){
    std::vector<std::string> output;
    output.push_back(charliesoft::_STR("BLOCK__INPUT_PARAM_IN_FILE"));
    return output;
  };
  std::vector<std::string> BlockLoader::getListOutputs(){
    std::vector<std::string> output;
    output.push_back(charliesoft::_STR("BLOCK__INPUT_PARAM_OUT_FRAMERATE"));
    output.push_back(charliesoft::_STR("BLOCK__INPUT_PARAM_OUT_WIDTH"));
    output.push_back(charliesoft::_STR("BLOCK__INPUT_PARAM_OUT_HEIGHT"));
    return output;
  };


};