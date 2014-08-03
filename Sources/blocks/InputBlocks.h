#ifndef _BLOCK_INPUTS_HEADER_
#define _BLOCK_INPUTS_HEADER_

#include "Block.h"


namespace charliesoft
{
  class BlockLoader :public Block
  {
  public:
    BlockLoader() :Block(_STR("BLOCK__INPUT_NAME")){};
    virtual void execute(){};
    virtual std::vector<std::string> getListParams();
    virtual std::vector<std::string> getListOutputs();
  };
}

#endif