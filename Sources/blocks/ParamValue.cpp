#include "ParamValue.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "ProcessManager.h"

#include "blocks/Block.h"

using namespace std;
using boost::lexical_cast;

namespace charliesoft
{
  ParamType ParamValue::getType(){
    return _PROCESS_MANAGER->getParamType(algo_->getName(), name_);
  };

  std::string ParamValue::toString() const
  {
    if (value_.type() == typeid(ParamValue*))
    {
      ParamValue* val = boost::get<ParamValue*>(value_);
      if (val == NULL)
        return "NULL";
      else
        return boost::get<ParamValue*>(value_)->toString();
    }
    if (value_.type() == typeid(Not_A_Value))
      return _STR("NOT_INITIALIZED");

    if (value_.type() == typeid(bool))
      return lexical_cast<string>(boost::get<bool>(value_));
    if (value_.type() == typeid(int))
      return lexical_cast<string>(boost::get<int>(value_));
    if (value_.type() == typeid(double))
      return lexical_cast<string>(boost::get<double>(value_));
    if (value_.type() == typeid(std::string))
      return boost::get<std::string>(value_);
    if (value_.type() == typeid(cv::Mat))
      return lexical_cast<string>(boost::get<cv::Mat>(value_));
    return "";
  }


  bool ParamValue::isDefaultValue() const{
    return (value_.type() == typeid(Not_A_Value));
      //|| (value_.type() == typeid(ParamValue*) && boost::get<ParamValue*>(value_) == NULL);
  };

  void ParamValue::set(const VariantClasses& v){
    value_ = v;
    algo_->setUpToDate(false);
  };


  ParamValue ParamValue::fromString(ParamType type, std::string value)
  {
    if (type == Boolean)
      return ParamValue(lexical_cast<bool>(value));
    if (type == Int)
      return ParamValue(lexical_cast<int>(value));
    if (type == Float)
      return ParamValue(lexical_cast<double>(value));
    if (type == String || type == FilePath)
      return ParamValue(value);
    return ParamValue();
  }

  //Boolean, Int, Float, Vector, Mat, String, FilePath, typeError
  void ParamValue::setString(const std::string& v){
    if (getType() == Boolean)
      value_ = lexical_cast<bool>(v);
    if (getType() == Int)
      value_ = lexical_cast<int>(v);
    if (getType() == Float)
      value_ = lexical_cast<double>(v);
    if (getType() == String || getType() == FilePath)
      value_ = v;
  };

  ParamValue& ParamValue::operator = (bool const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (int const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (double const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (std::string const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Mat const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (Not_A_Value const &rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue *rhs) {
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue const &rhs) {
    if (this != &rhs) {
      value_ = rhs.value_;
      algo_ = rhs.algo_;
      name_ = rhs.name_;
      isOutput_ = rhs.isOutput_;
    }
    return *this;
  };

  bool ParamValue::operator== (const ParamValue &other) const
  {
    try
    {
      if (value_.type() == typeid(Not_A_Value) ||
        other.value_.type() == typeid(Not_A_Value))
        return false;//always different!

      if (value_.type() == typeid(ParamValue*))
      {//compare adresses
        ParamValue* val = boost::get<ParamValue*>(value_);
        ParamValue* val1 = boost::get<ParamValue*>(other.value_);
        return val == val1;
      }

      if (value_.type() == typeid(bool))
      {
        bool val = boost::get<bool>(value_);
        bool val1 = boost::get<bool>(other.value_);
        return val == val1;
      }

      if (value_.type() == typeid(int))
      {
        int val = boost::get<int>(value_);
        int val1 = boost::get<int>(other.value_);
        return val == val1;
      }

      if (value_.type() == typeid(double))
      {
        double val = boost::get<double>(value_);
        double val1 = boost::get<double>(other.value_);
        return val == val1;
      }

      if (value_.type() == typeid(std::string))
      {
        string val = boost::get<string>(value_);
        string val1 = boost::get<string>(other.value_);
        return val.compare(val1) == 0;
      }

      if (value_.type() == typeid(cv::Mat))
      {//compare data adresses
        cv::Mat val = boost::get<cv::Mat>(value_);
        cv::Mat val1 = boost::get<cv::Mat>(other.value_);
        return val.ptr<char>() == val1.ptr<char>();
      }
    }
    catch (boost::bad_get&)
    {
    }
    return false;
  }

  bool ParamValue::operator<(const ParamValue &other) const
  {
    try
    {
      if (value_.type() == typeid(Not_A_Value) ||
        other.value_.type() == typeid(Not_A_Value))

        if (value_.type() == typeid(ParamValue*))
        {//compare adresses
        ParamValue* val = boost::get<ParamValue*>(value_);
        ParamValue* val1 = boost::get<ParamValue*>(other.value_);
        return val < val1;
        }

      if (value_.type() == typeid(bool))
      {
        bool val = boost::get<bool>(value_);
        bool val1 = boost::get<bool>(other.value_);
        return val < val1;
      }

      if (value_.type() == typeid(int))
      {
        int val = boost::get<int>(value_);
        int val1 = boost::get<int>(other.value_);
        return val < val1;
      }

      if (value_.type() == typeid(double))
      {
        double val = boost::get<double>(value_);
        double val1 = boost::get<double>(other.value_);
        return val < val1;
      }

      if (value_.type() == typeid(std::string))
      {
        string val = boost::get<string>(value_);
        string val1 = boost::get<string>(other.value_);
        return val.compare(val1) < 0;
      }

      if (value_.type() == typeid(cv::Mat))
      {//compare data adresses
        cv::Mat val = boost::get<cv::Mat>(value_);
        cv::Mat val1 = boost::get<cv::Mat>(other.value_);
        return val.ptr<char>() < val1.ptr<char>();
      }
    }
    catch (boost::bad_get&)
    {
    }
    return false;
  }

  bool ParamValue::operator> (const ParamValue &other) const
  {
    if (value_.type() == typeid(Not_A_Value) ||
      other.value_.type() == typeid(Not_A_Value))
      return false;//always different!
    return this->operator!=(other) && !this->operator<(other);
  }
}