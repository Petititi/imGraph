
#include <vector>

#include "Block.h"
using namespace lsis_org;
using std::vector;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockShow);
  //You can add methods, reimplement needed functions...
  //Here we need validateParams:
  virtual bool validateParams();
  BLOCK_END_INSTANTIATION(BlockShow, AlgoType::output, BLOCK__OUTPUT_NAME);

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

  bool BlockShow::validateParams(){
    bool isOk = true;
    //we need BLOCK__OUTPUT_IN_IMAGE to be set:
    if (myInputs_["BLOCK__OUTPUT_IN_IMAGE"].isDefaultValue())
    {
      isOk = false;
      error_msg_ += (my_format(_STR("ERROR_PARAM_NEEDED")) % _STR("BLOCK__OUTPUT_IN_IMAGE")).str();
    }
    return isOk;
  };


};