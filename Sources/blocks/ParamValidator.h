#ifndef _BLOCK_PARAMVALIDATOR_HEADER_
#define _BLOCK_PARAMVALIDATOR_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <boost/filesystem.hpp>
#include <boost/exception/exception.hpp>
#include <vector>
#include <stdarg.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "ParamValue.h"
#include "Internationalizator.h"

namespace charliesoft
{
  class Block;

  class ErrorValidator
  {
  public:
    std::string errorMsg;
    ErrorValidator(std::string error){
      errorMsg = error;
    }
  };

  class ParamValidator
  {
  protected:
    const ParamValue* paramToValid_;
  public:
    ParamValidator(){ paramToValid_ = NULL; };
    void setParamOrigin(const ParamValue* p){ paramToValid_ = p; };
    virtual void validate(const ParamValue& value)=0;
  };

  class ValNeeded :public ParamValidator
  {
  public:
    ValNeeded(){};
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
          throw (ErrorValidator((my_format(_STR("ERROR_PARAM_NEEDED")) % _STR(paramToValid_->getName())).str()));
      }
    }
  };

  class ValFileExist :public ParamValidator
  {
  public:
    ValFileExist(){};
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
        return;//nothing to do as value is not set!
      if (!boost::filesystem::exists(value.toString()))
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
          throw (ErrorValidator((my_format(_STR("BLOCK__INPUT_IN_FILE_NOT_FOUND")) % value.toString()).str()));
      }
    }
  };

  class FileIsFolder :public ParamValidator
  {
  public:
    FileIsFolder(){};
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
        return;//nothing to do as value is not set!
      if (!boost::filesystem::exists(value.toString()))
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
          throw (ErrorValidator((my_format(_STR("BLOCK__INPUT_IN_FILE_NOT_FOUND")) % value.toString()).str()));
      }
      if (!boost::filesystem::is_directory(value.toString()))
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
          throw (ErrorValidator((my_format(_STR("BLOCK__INPUT_IN_FILE_NOT_FOLDER")) % value.toString()).str()));
      }
    }
  };

  class ValPositiv :public ParamValidator
  {
    bool isStrict_;
  public:
    ValPositiv(bool strictOnly = false){
      isStrict_ = strictOnly;
    }
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
        return;//nothing to do as value is not set!
      bool isOK = true;
      if (isStrict_)
        isOK &= value > 0;
      else
        isOK &= value >= 0;
      if (!isOK)
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
        {
          std::string errorTmp = "";
          if (isStrict_) errorTmp = "ERROR_PARAM_ONLY_POSITIF_STRICT";
          else errorTmp = "ERROR_PARAM_ONLY_POSITIF";
          throw (ErrorValidator((my_format(_STR(errorTmp)) % _STR(paramToValid_->getName())).str()));
        }
      }
    }
  };

  class ValNegativ :public ParamValidator
  {
    bool isStrict_;
  public:
    ValNegativ(bool strictOnly = false){
      isStrict_ = strictOnly;
    }
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
        return;//nothing to do as value is not set!
      bool isOK = true;
      if (isStrict_)
        isOK &= value < 0;
      else
        isOK &= value <= 0;
      if (!isOK)
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
        {
          std::string errorTmp = "";
          if (isStrict_) errorTmp = "ERROR_PARAM_ONLY_NEGATIF_STRICT";
          else errorTmp = "ERROR_PARAM_ONLY_NEGATIF";
          throw (ErrorValidator((my_format(_STR(errorTmp)) % _STR(paramToValid_->getName())).str()));
        }
      }
    }
  };

  class ValExclusif :public ParamValidator
  {
    const ParamValue& otherVal_;
  public:
    ValExclusif(const ParamValue& dest):
      otherVal_(dest){}
    virtual void validate(const ParamValue& value)
    {
      if (value.isDefaultValue())
        return;//nothing to do as value is not set!
      if (paramToValid_ == NULL)
        throw (ErrorValidator(_STR("ERROR_GENERIC")));
      else
      {
        if (otherVal_ == value)
          throw (ErrorValidator((my_format(_STR("ERROR_PARAM_EXCLUSIF")) % _STR(paramToValid_->getName()) % _STR(otherVal_.getName())).str()));
      }
    }
  };

  class ValRange :public ParamValidator
  {
    double inf_;
    double sup_;
  public:
    ValRange(double minima, double maxima) :
      inf_(minima), sup_(maxima){}
    virtual void validate(const ParamValue& value)
    {
      if (value < inf_ || value > sup_)
      {
        if (paramToValid_ == NULL)
          throw (ErrorValidator(_STR("ERROR_GENERIC")));
        else
          throw (ErrorValidator((my_format(_STR("ERROR_PARAM__valueBETWEEN")) %
          _STR(paramToValid_->getName()) % value.get<double>() % inf_ % sup_).str()));
      }
    }
  };
}

#endif