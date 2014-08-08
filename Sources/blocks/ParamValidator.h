#ifndef _BLOCK_PARAMVALIDATOR_HEADER_
#define _BLOCK_PARAMVALIDATOR_HEADER_

#include <boost/filesystem.hpp>
#include <vector>
#include <stdarg.h>
#include "ParamValue.h"

namespace charliesoft
{
  class Block;

  class ParamValidator
  {
  public:
    virtual bool validate(const ParamValue& value)=0;
  };

  class ValNeeded :public ParamValidator
  {
  public:
    ValNeeded(){};
    virtual bool validate(const ParamValue& value)
    {
      return !value.isDefaultValue();
    }
  };

  class ValFileExist :public ParamValidator
  {
  public:
    ValFileExist(){};
    virtual bool validate(const ParamValue& value)
    {
      return boost::filesystem::exists(value.toString());
    }
  };

  class ValPositiv :public ParamValidator
  {
    bool isStrict_;
  public:
    ValPositiv(bool strictOnly = false){
      isStrict_ = strictOnly;
    }
    virtual bool validate(const ParamValue& value)
    {
      if (value.getType() != Int || value.getType() != Float)
        return false;
      if (isStrict_)
        return value > 0;
      return value >= 0;
    }
  };

  class ValNegativ :public ParamValidator
  {
    bool isStrict_;
  public:
    ValNegativ(bool strictOnly = false){
      isStrict_ = strictOnly;
    }
    virtual bool validate(const ParamValue& value)
    {
      if (value.getType() != Int || value.getType() != Float)
        return false;
      if (isStrict_)
        return value < 0;
      return value >= 0;
    }
  };

  class ValExclusif :public ParamValidator
  {
    const ParamValue& val1;
    const ParamValue& val2;
  public:
    ValExclusif(const ParamValue& src, const ParamValue& dest):
      val1(src), val2(dest){}
    virtual bool validate(const ParamValue& value)
    {
      return (val2 != value);
    }
  };

  class ValRange :public ParamValidator
  {
    double inf_;
    double sup_;
  public:
    ValRange(double minima, double maxima) :
      inf_(minima), sup_(maxima){}
    virtual bool validate(const ParamValue& value)
    {
      return value >= inf_ && value <= sup_;
    }
  };
}

#endif