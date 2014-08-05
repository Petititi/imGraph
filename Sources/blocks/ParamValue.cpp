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

  std::string ParamValue::toString()
  {
    if (value_.type() == typeid(ParamValue*))
      return boost::get<ParamValue*>(value_)->toString();
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

  void ParamValue::set(const VariantClasses& v){
    value_ = v;
    algo_->setUpToDate(false);
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
}