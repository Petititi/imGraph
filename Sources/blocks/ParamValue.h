#ifndef _BLOCK_PARAM_VALUE_HEADER_
#define _BLOCK_PARAM_VALUE_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/variant.hpp>
#include <opencv2/core/core.hpp>
#include <QString>
#ifdef _WIN32
#pragma warning(pop)
#endif

namespace charliesoft
{
  class ParamValue;
  class Block;

  enum ParamType
  {
    Boolean, Int, Float, Vector, Mat, String, FilePath, typeError
  };

  struct Not_A_Value
  {
    bool justForMe_;
    bool operator == (const Not_A_Value &b) const
    {
      return false;
    }
  };

  typedef boost::variant <
    bool,
    int,
    double,
    std::string,
    cv::Mat,
    ParamValue*,
    Not_A_Value > VariantClasses;

  class ParamValue
  {
    Block *algo_;
    std::string name_;
    bool isOutput_;
    VariantClasses value_;
  public:
    ParamValue(Block *algo, std::string name, bool isOutput) :
      algo_(algo), name_(name), isOutput_(isOutput), value_(Not_A_Value()){};
    ParamValue() :
      algo_(NULL), name_(""), isOutput_(false), value_(Not_A_Value()){};
    ParamValue(bool v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(int v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(double v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(std::string v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(cv::Mat v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(Not_A_Value v) :
      algo_(NULL), name_(""), isOutput_(false), value_(Not_A_Value()){};
    ParamValue(ParamValue* v) :
      algo_(NULL), name_(""), isOutput_(false), value_(v){};
    ParamValue(ParamValue& va) :
      algo_(va.algo_), name_(va.name_), isOutput_(va.isOutput_), value_(va.value_){};

    static ParamValue fromString(ParamType,std::string);

    ParamValue& operator=(bool const &rhs);
    ParamValue& operator=(int const &rhs);
    ParamValue& operator=(double const &rhs);
    ParamValue& operator=(std::string const &rhs);
    ParamValue& operator=(cv::Mat const &rhs);
    ParamValue& operator=(Not_A_Value const &rhs);
    ParamValue& operator=(ParamValue const &rhs);
    ParamValue& operator=(ParamValue* rhs);
    bool operator== (const ParamValue &b) const;
    bool operator< (const ParamValue &b) const;
    bool operator> (const ParamValue &b) const;
    bool operator<= (const ParamValue &b) const
    {
      return this->operator==(b) || this->operator<(b);
    }
    bool operator>= (const ParamValue &b) const
    {
      return this->operator==(b) || this->operator>(b);
    }
    bool operator!= (const ParamValue &b) const
    {
      return !(this->operator==(b));
    }

    std::string toString() const;

    bool isDefaultValue() const;

    ParamType getType();

    template<typename T>
    T get(bool update)
    {
      if (value_.type() == typeid(ParamValue*))
      {
        ParamValue* distantParam = boost::get<ParamValue*>(value_);
        if (distantParam == NULL)
          return T();
        else
          return distantParam->get<T>(update);
      }
      if (update)
        algo_->updateIfNeeded();
      if (value_.type() == typeid(Not_A_Value))
      {
        return T();
      }
      try
      {
        return boost::get<T>(value_);
      }
      catch (boost::bad_get&)
      {
        return T();
      }
    }

    template<typename T>
    T get_const() const
    {
      if (value_.type() == typeid(ParamValue*))
      {
        ParamValue* distantParam = boost::get<ParamValue*>(value_);
        if (distantParam == NULL)
          return T();
        else
          return distantParam->get_const<T>();
      }
      if (value_.type() == typeid(Not_A_Value))
      {
        return T();
      }
      try
      {
        return boost::get<T>(value_);
      }
      catch (boost::bad_get&)
      {
        return T();
      }
    }

    void set(const VariantClasses& v);
    void setString(const std::string& v);
  };
}

#endif