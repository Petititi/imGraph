#include "ParamValue.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "ProcessManager.h"
#include "ParamValidator.h"

#include "Block.h"
#include "Graph.h"

using namespace std;
using boost::lexical_cast;

namespace charliesoft
{
  ParamType ParamValue::getType() const{
    if (block_ == NULL)
      return typeError;
    return _PROCESS_MANAGER->getParamType(block_->getName(), _name, !isOutput_);
  };

  void ParamValue::addValidator(std::initializer_list<ParamValidator*> list)
  {
    for (auto elem : list)
    {
      elem->setParamOrigin(this);
      validators_.push_back(elem);
    }
  };

  void ParamValue::validate(const ParamValue& other) const
  {
    for (auto elem : validators_)
      elem->validate(other);//throw ErrorValidator if not valid
  }

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
  };

  void ParamValue::valid_and_set(const ParamValue& v){
    validate(v);//if not valid throw an error!
    if (v.value_.type() == typeid(ParamValue *))
    {
      if (isLinked())
        boost::get<ParamValue*>(value_)->distantListeners_.erase(this);
      ParamValue* vDist = boost::get<ParamValue*>(v.value_);
      if (vDist != NULL) vDist->distantListeners_.insert(this);
    }
    value_ = v.value_;
    _current_timestamp = GraphOfProcess::_current_timestamp;
    notifyUpdate();//wake up waiting thread (if any)
  };


  BlockLink ParamValue::toBlockLink() const
  {
    ParamValue* other = get<ParamValue*>();
    return BlockLink(other->block_, block_, other->_name, _name);
  }


  ParamValue ParamValue::fromString(ParamType type, std::string value)
  {
    try{
      if (type == Boolean)
        return ParamValue(lexical_cast<bool>(value));
      if (type == Int)
        return ParamValue(lexical_cast<int>(value));
      if (type == Float)
        return ParamValue(lexical_cast<double>(value));
      if (type == String || type == FilePath)
        return ParamValue(value);
    }
    catch (...)//not the correct type, so create an empty value...
    {
    }
    return ParamValue();
  }

  void ParamValue::notifyRemove()
  {
    if (isLinked())
      boost::get<ParamValue*>(value_)->distantListeners_.erase(this);
  }
  
  void ParamValue::notifyUpdate()
  { 
    _current_timestamp = GraphOfProcess::_current_timestamp;
    for (auto listener : distantListeners_)
    {
      listener->_current_timestamp = _current_timestamp;
    }
  }
  ParamValue& ParamValue::operator = (bool const &rhs) {
    notifyRemove();
    value_ = rhs;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (int const &rhs) {
    notifyRemove();
    value_ = rhs;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (double const &rhs) {
    notifyRemove();
    value_ = rhs;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (std::string const &rhs) {
    notifyRemove();
    value_ = rhs;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Mat const &rhs) {
    notifyRemove();
    value_ = rhs;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (Not_A_Value const &rhs) {
    notifyRemove();
    value_ = rhs;
    _current_timestamp = GraphOfProcess::_current_timestamp;
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue *vDist) {
    notifyRemove();
    if (vDist != NULL) vDist->distantListeners_.insert(this);
    value_ = vDist;
    notifyUpdate();
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue const &rhs) {
    if (this != &rhs) {
      notifyRemove();
      value_ = rhs.value_;
      block_ = rhs.block_;
      _name = rhs._name;
      isOutput_ = rhs.isOutput_;
      _current_timestamp = rhs._current_timestamp;
      if (value_.type() != typeid(Not_A_Value))
        notifyUpdate();
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
      {//compare addresses
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
        return false;

      if (value_.type() == typeid(ParamValue*))
      {//compare addresses
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
        if (other.value_.type() == typeid(int))
          return val < boost::get<int>(other.value_);
        if (other.value_.type() == typeid(double))
          return val < boost::get<double>(other.value_);
      }

      if (value_.type() == typeid(double))
      {
        double val = boost::get<double>(value_);
        if (other.value_.type() == typeid(int))
          return val < boost::get<int>(other.value_);
        if (other.value_.type() == typeid(double))
          return val < boost::get<double>(other.value_);
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