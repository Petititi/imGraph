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
  // Boolean, Int, Float, Color, Matrix, String, FilePath, ListBox, typeError
  ParamType ParamValue::getType() const{
    if (block_ == NULL)
      return typeError;
    ParamType out = _PROCESS_MANAGER->getParamType(block_->getName(), _name, !isOutput_);
    if (out == typeError)
    {
      //try to find value type:
      if (value_.type() == typeid(ParamValue*))
      {
        ParamValue* val = boost::get<ParamValue*>(value_);
        if (val == NULL)
          return typeError;
        else
          return val->getType();
      }
      if (value_.type() == typeid(bool))
        return Boolean;
      if (value_.type() == typeid(int))
        return Int;
      if (value_.type() == typeid(double))
        return Float;
      if (value_.type() == typeid(std::string))
        return String;
      if (value_.type() == typeid(cv::Scalar))
        return Color;
      if (value_.type() == typeid(cv::Mat))
        return Matrix;
    }
    return out;
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
    if (value_.type() == typeid(cv::Scalar))
    {
      cv::FileStorage fs(".xml", cv::FileStorage::WRITE + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
      fs << "colorValue" << boost::get<cv::Scalar>(value_);
      string buf = fs.releaseAndGetString();

      return buf;
    }
    if (value_.type() == typeid(cv::Mat))
    {
      cv::FileStorage fs(".xml", cv::FileStorage::WRITE + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
      fs << "matrixValue" << boost::get<cv::Mat>(value_);
      string buf = fs.releaseAndGetString();

      return buf;
    }
    return "";
  }

  bool ParamValue::isDefaultValue() const{
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    return (value_.type() == typeid(Not_A_Value)) || 
      (isLinked() && boost::get<ParamValue*>(value_)->isDefaultValue());
  };

  void ParamValue::valid_and_set(const ParamValue& v){
    validate(v);//if not valid throw an error!
    if (v.value_.type() == typeid(ParamValue *))
    {
      if (isLinked())
        boost::get<ParamValue*>(value_)->distantListeners_.erase(this);
      ParamValue* vDist = boost::get<ParamValue*>(v.value_);
      if (vDist != NULL) vDist->distantListeners_.insert(this);
      if (vDist->value_.type() != typeid(Not_A_Value))
        _newValue = true;
    }
    else
      if (*this != v && v.value_.type() != typeid(Not_A_Value))
        _newValue = true;
    value_ = v.value_;
    notifyUpdate(_newValue);//wake up waiting thread (if any)
  };


  BlockLink ParamValue::toBlockLink() const
  {
    ParamValue* other = get<ParamValue*>(false);
    return BlockLink(other->block_, block_, other->_name, _name);
  }


  ParamValue ParamValue::fromString(ParamType type, std::string value)
  {
    try{
      if (type == Boolean)
        return ParamValue(lexical_cast<bool>(value));
      if (type == Int||type == ListBox)
        return ParamValue(lexical_cast<int>(value));
      if (type == Float)
        return ParamValue(lexical_cast<double>(value));
      if (type == String || type == FilePath)
        return ParamValue(value);
      if (type == Color)
      {
        cv::FileStorage fs(value, cv::FileStorage::READ + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
        cv::Scalar val;
        fs["colorValue"] >> val;
        return ParamValue(val);
      }
      if (type == Matrix)
      {
        cv::FileStorage fs(value, cv::FileStorage::READ + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
        cv::Mat val;
        fs["matrixValue"] >> val;
        return ParamValue(val);
      }
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
  

  void ParamValue::notifyUpdate(bool isNew)
  {
    _newValue = isNew;
    for (auto listener : distantListeners_)
      listener->notifyUpdate(isNew);
  }
  ParamValue& ParamValue::operator = (bool const &rhs) {
    notifyRemove();
    if (*this != rhs)
      _newValue = true;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (int const &rhs) {
    notifyRemove();
    if (*this != rhs)
      _newValue = true;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (double const &rhs) {
    notifyRemove();
    if (*this != rhs)
      _newValue = true;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (std::string const &rhs) {
    notifyRemove();
    if (*this != rhs)
      _newValue = true;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Scalar const &rhs) {
    notifyRemove();
    if (*this != rhs)
      _newValue = true;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Mat const &rhs) {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    notifyRemove();
    _newValue = true;//it's difficult to say if it's the same matrix, so we conclude it's always new...
    value_ = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (Not_A_Value const &rhs) {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    notifyRemove();
    value_ = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue *vDist) {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    notifyRemove();
    if (vDist != NULL) vDist->distantListeners_.insert(this);
    _newValue = vDist->_newValue;
    value_ = vDist;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue const &rhs) {
    if (this != &rhs) {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      notifyRemove();
      _newValue = true;
      value_ = rhs.value_;
      if (rhs.block_ != NULL)
      {
        block_ = rhs.block_;
        _name = rhs._name;
        isOutput_ = rhs.isOutput_;
      }
      if (value_.type() != typeid(Not_A_Value))
        notifyUpdate(_newValue);
    }
    return *this;
  };

  bool ParamValue::operator== (const ParamValue &other) const
  {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
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

      if (value_.type() == typeid(cv::Scalar))
      {
        cv::Scalar val = boost::get<cv::Scalar>(value_);
        cv::Scalar val1 = boost::get<cv::Scalar>(other.value_);
        return val == val1;
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
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
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

      if (value_.type() == typeid(cv::Scalar))
      {
        cv::Scalar val = boost::get<cv::Scalar>(value_);
        cv::Scalar val1 = boost::get<cv::Scalar>(other.value_);
        if (val[0] < val1[0])
          return true;
        if (val[0] > val1[0])
          return false;
        if (val[1] < val1[1])
          return true;
        if (val[1] > val1[1])
          return false;
        if (val[2] < val1[2])
          return true;
        return false;
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
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    if (value_.type() == typeid(Not_A_Value) ||
      other.value_.type() == typeid(Not_A_Value))
      return false;//always different!
    return this->operator!=(other) && !this->operator<(other);
  }
}