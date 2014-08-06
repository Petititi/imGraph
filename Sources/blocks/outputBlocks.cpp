
#include <vector>

#include "Block.h"
using namespace lsis_org;
using std::vector;

namespace charliesoft
{
  BLOCK_INSTANTIATE(BlockShow, AlgoType::output, BLOCK__OUTPUT_NAME);

  BlockShow::BlockShow() :Block("BLOCK__OUTPUT_NAME"){};
  
  void BlockShow::run(){
  };

  std::vector<ParamDefinition> BlockShow::getListParams(){
    std::vector<ParamDefinition> output;
    output.push_back(ParamDefinition(true, Mat,
      "BLOCK__OUTPUT_IN_IMAGE", "BLOCK__OUTPUT_IN_IMAGE_HELP"));
    return output;
  };
  std::vector<ParamDefinition> BlockShow::getListOutputs(){
    return std::vector<ParamDefinition>();
  };


};