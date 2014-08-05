#ifndef _BLOCK_PARAM_VALUE_HEADER_
#define _BLOCK_PARAM_VALUE_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/variant.hpp>
#include <opencv2/core/core.hpp>
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

    ParamValue& operator=(bool const &rhs);
    ParamValue& operator=(int const &rhs);
    ParamValue& operator=(double const &rhs);
    ParamValue& operator=(std::string const &rhs);
    ParamValue& operator=(cv::Mat const &rhs);
    ParamValue& operator=(Not_A_Value const &rhs);
    ParamValue& operator=(ParamValue const &rhs);
    ParamValue& operator=(ParamValue* rhs);

    std::string toString();

    ParamType getType();

    template<typename T>
    T get(bool update)
    {
      if (value_.type() == typeid(ParamValue*))
      {
        ParamValue* distantParam = boost::get<ParamValue*>(value_);
        return distantParam->get<T>(update);
      }
      if (update)
        algo_->updateIfNeeded();
      if (value_.type() == typeid(Not_A_Value))
      {
        return T();
      }
      return boost::get<T>(value_);
    }

    void set(const VariantClasses& v);
  };
}

#endif