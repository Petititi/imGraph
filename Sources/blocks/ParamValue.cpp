#include "ParamValue.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "ProcessManager.h"
#include "ParamValidator.h"

#include "SubBlock.h"
#include "Block.h"
#include "Graph.h"

using namespace std;
using boost::lexical_cast;

namespace charliesoft
{
  ParamValue::ParamValue(Block *algo, ParamDefinition* def, bool isOutput) :
    _block(algo), _name(def->_name), _isOutput(isOutput), _value(Not_A_Value()), _definition(def){
    _newValue = false; _paramNeeded = true;
  };

  ParamValue::~ParamValue()
  {
    if (isLinked())
      boost::get<ParamValue*>(_value)->_distantListeners.erase(this);
    for (auto it = _distantListeners.begin();
      it != _distantListeners.end(); it++)
    {
      if (*it != NULL)
        (*it)->_value = Not_A_Value();
    }
    for (auto it : _validators)
      delete it;
    _validators.clear();

    _value = Not_A_Value();
  }

  std::string ParamValue::getValFromList()
  {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    if (getType() != ListBox || _block == NULL)
      return "";
    string help = _STR(_PROCESS_MANAGER->getParamHelp(_block->getName(), _name, !_isOutput));
    size_t pos = help.find_first_of('|');
    if (pos != std::string::npos && pos<help.length()-2)
    {
      std::string params = help.substr(pos + 1);
      std::vector<std::string> values;
      boost::split(values, params, boost::is_any_of("^"));
      size_t idx = 0;
      if (!isDefaultValue())
        idx = static_cast<size_t>(boost::get<int>(_value));
      if (idx >= 0 && idx<values.size())
        return values[idx];
    };
    return "";
  }

  ParamType ParamValue::getType(bool allow_AnyType) const{
    if (_block == NULL)
      return typeError;
    ParamType out = typeError;
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    if (_definition != NULL)
      out = _definition->_type;
    if (out == typeError)//if anyType, try to detect type...
      out = _PROCESS_MANAGER->getParamType(_block->getName(), _name, !_isOutput);
    if (out == typeError || out == AnyType)//if anyType, try to detect type...
    {
      if (out != AnyType)
      {
        //try to ask block for the type:
        SubBlock* subBlock = dynamic_cast<SubBlock*>(_block);
        if (subBlock != NULL)
        {
          out = subBlock->getDef(_name, !_isOutput)._type;
          if (out != typeError)
            return out;
        }
      }
      if ((out == AnyType && !allow_AnyType) ||
        (out != AnyType))
      {
        //try to find value type:
        if (_value.type() == typeid(ParamValue*))
        {
          ParamValue* val = boost::get<ParamValue*>(_value);
          if (val == NULL)
            return typeError;
          else
            return val->getType();
        }
        if (_value.type() == typeid(bool))
          return Boolean;
        if (_value.type() == typeid(int))
          return Int;
        if (_value.type() == typeid(double))
          return Float;
        if (_value.type() == typeid(std::string))
          return String;
        if (_value.type() == typeid(cv::Scalar))
          return Color;
        if (_value.type() == typeid(cv::Mat))
          return Matrix;
      }
    }
    return out;
  };

  void ParamValue::addValidator(std::initializer_list<ParamValidator*> list)
  {
    for (auto elem : list)
    {
      elem->setParamOrigin(this);
      _validators.push_back(elem);
    }
  };

  void ParamValue::validate(const ParamValue& other) const
  {
    for (auto elem : _validators)
      elem->validate(other);//throw ErrorValidator if not valid
  }

  std::string ParamValue::toString() const
  {
    if (_value.type() == typeid(ParamValue*))
    {
      ParamValue* val = boost::get<ParamValue*>(_value);
      if (val == NULL)
        return "NULL";
      else
        return boost::get<ParamValue*>(_value)->toString();
    }
    if (_value.type() == typeid(Not_A_Value))
      return _STR("NOT_INITIALIZED");

    if (_value.type() == typeid(bool))
      return lexical_cast<string>(boost::get<bool>(_value));
    if (_value.type() == typeid(int))
      return lexical_cast<string>(boost::get<int>(_value));
    if (_value.type() == typeid(double))
      return lexical_cast<string>(boost::get<double>(_value));
    if (_value.type() == typeid(std::string))
      return boost::get<std::string>(_value);
    if (_value.type() == typeid(cv::Scalar))
    {
      cv::FileStorage fs(".xml", cv::FileStorage::WRITE + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
      fs << "colorValue" << boost::get<cv::Scalar>(_value);
      string buf = fs.releaseAndGetString();

      return buf;
    }
    if (_value.type() == typeid(cv::Mat))
    {
      cv::FileStorage fs(".xml", cv::FileStorage::WRITE + cv::FileStorage::MEMORY + cv::FileStorage::FORMAT_YAML);
      fs << "matrixValue" << boost::get<cv::Mat>(_value);
      string buf = fs.releaseAndGetString();

      return buf;
    }
    return "";
  }

  void ParamValue::setDefaultValue(){
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _value = Not_A_Value();
  };

  bool ParamValue::isDefaultValue() const{
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    return (_value.type() == typeid(Not_A_Value)) ||
      //(_value.type() == typeid(cv::Mat) && boost::get<cv::Mat>(_value).empty()) ||
      (isLinked() && boost::get<ParamValue*>(_value)->isDefaultValue());
  };


  void ParamValue::update()
  {
    if (_block == NULL)
      return;//nothing to do, this value can't be updated!
    if (_isOutput)//the wanted value is the output of this block...
      _block->update();
    else if (isLinked())//the wanted value is the output of the connected block...
      boost::get<ParamValue*>(_value)->_block->update();
    //else : nothing to do!
  }

  void ParamValue::valid_and_set(const ParamValue& v){
    validate(v);//if not valid throw an error!
    if (v._value.type() == typeid(ParamValue *))
    {
      if (isLinked())
        boost::get<ParamValue*>(_value)->_distantListeners.erase(this);
      ParamValue* vDist = boost::get<ParamValue*>(v._value);
      if (vDist != NULL) vDist->_distantListeners.insert(this);
      if (vDist->_value.type() != typeid(Not_A_Value))
        _newValue = true;
    }
    else
      if (*this != v && v._value.type() != typeid(Not_A_Value))
        _newValue = true;
    _value = v._value;
    notifyUpdate(_newValue);//wake up waiting thread (if any)
  };

  BlockLink ParamValue::toBlockLink() const
  {
    ParamValue* other = get<ParamValue*>();
    return BlockLink(other->_block, _block, other->_name, _name);
  }

  ParamValue ParamValue::fromString(ParamType type, std::string value)
  {
    try{
      if (type == Boolean)
        return ParamValue(lexical_cast<bool>(value));
      if (type == Int||type == ListBox)
        return ParamValue(lexical_cast<int>(value));
      if (type == Float)
      {
        replace(value.begin(), value.end(), ',', '.');
        return ParamValue(lexical_cast<double>(value));
      }
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
      boost::get<ParamValue*>(_value)->_distantListeners.erase(this);
  }
  void ParamValue::notifyUpdate(bool isNew)
  {
    emit paramUpdated();//for UI
    _cond_sync.notify_all();
    _newValue = isNew;
    for (auto listener : _distantListeners)
      listener->notifyUpdate(isNew);
  }

  ParamValue& ParamValue::operator = (bool const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (int const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (double const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator=(const char* rhs)
  {
    return operator=(std::string(rhs));
  }
  ParamValue& ParamValue::operator = (std::string const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Scalar const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (cv::Mat const &rhs) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _newValue = true;
    _value = rhs;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (Not_A_Value const &rhs) {
    notifyRemove();
    while (!_distantListeners.empty())
      *(*(_distantListeners.begin())) = Not_A_Value();

    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _value = rhs;
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue *vDist) {
    notifyRemove();
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    if (_definition == NULL)
      _definition = vDist->_definition;
    if (vDist != NULL) vDist->_distantListeners.insert(this);
    _newValue = true;
    _value = vDist;
    notifyUpdate(_newValue);
    return *this;
  };
  ParamValue& ParamValue::operator = (ParamValue const &rhs) {
    if (this != &rhs) {
      notifyRemove();
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      if (_definition == NULL)
        _definition = rhs._definition;
      _newValue = true;
      _value = rhs._value;
      if (rhs._block != NULL)
      {
        _block = rhs._block;
        _name = rhs._name;
        _isOutput = rhs._isOutput;
      }
      if (_value.type() != typeid(Not_A_Value))
        notifyUpdate(_newValue);
    }
    return *this;
  };

  bool ParamValue::operator== (const ParamValue &other) const
  {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    try
    {
      if (_value.type() == typeid(Not_A_Value) ||
        other._value.type() == typeid(Not_A_Value))
        return false;//always different!

      if (_value.type() == typeid(ParamValue*))
      {//compare addresses
        ParamValue* val = boost::get<ParamValue*>(_value);
        ParamValue* val1 = boost::get<ParamValue*>(other._value);
        return val == val1;
      }

      if (_value.type() == typeid(bool))
      {
        bool val = boost::get<bool>(_value);
        bool val1 = boost::get<bool>(other._value);
        return val == val1;
      }

      if (_value.type() == typeid(int))
      {
        int val = boost::get<int>(_value);
        int val1 = boost::get<int>(other._value);
        return val == val1;
      }

      if (_value.type() == typeid(double))
      {
        double val = boost::get<double>(_value);
        double val1 = boost::get<double>(other._value);
        return val == val1;
      }

      if (_value.type() == typeid(std::string))
      {
        string val = boost::get<string>(_value);
        string val1 = boost::get<string>(other._value);
        return val.compare(val1) == 0;
      }

      if (_value.type() == typeid(cv::Scalar))
      {
        cv::Scalar val = boost::get<cv::Scalar>(_value);
        cv::Scalar val1 = boost::get<cv::Scalar>(other._value);
        return val == val1;
      }

      if (_value.type() == typeid(cv::Mat))
      {//compare data adresses
        cv::Mat val = boost::get<cv::Mat>(_value);
        cv::Mat val1 = boost::get<cv::Mat>(other._value);
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
      if (_value.type() == typeid(Not_A_Value) ||
        other._value.type() == typeid(Not_A_Value))
        return false;

      if (_value.type() == typeid(ParamValue*))
      {//compare addresses
        ParamValue* val = boost::get<ParamValue*>(_value);
        ParamValue* val1 = boost::get<ParamValue*>(other._value);
        return val < val1;
      }

      if (_value.type() == typeid(bool))
      {
        bool val = boost::get<bool>(_value);
        bool val1 = boost::get<bool>(other._value);
        return val < val1;
      }

      if (_value.type() == typeid(int))
      {
        int val = boost::get<int>(_value);
        if (other._value.type() == typeid(int))
          return val < boost::get<int>(other._value);
        if (other._value.type() == typeid(double))
          return val < boost::get<double>(other._value);
      }

      if (_value.type() == typeid(double))
      {
        double val = boost::get<double>(_value);
        if (other._value.type() == typeid(int))
          return val < boost::get<int>(other._value);
        if (other._value.type() == typeid(double))
          return val < boost::get<double>(other._value);
      }

      if (_value.type() == typeid(std::string))
      {
        string val = boost::get<string>(_value);
        string val1 = boost::get<string>(other._value);
        return val.compare(val1) < 0;
      }

      if (_value.type() == typeid(cv::Scalar))
      {
        cv::Scalar val = boost::get<cv::Scalar>(_value);
        cv::Scalar val1 = boost::get<cv::Scalar>(other._value);
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

      if (_value.type() == typeid(cv::Mat))
      {//compare data adresses
        cv::Mat val = boost::get<cv::Mat>(_value);
        cv::Mat val1 = boost::get<cv::Mat>(other._value);
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
    if (_value.type() == typeid(Not_A_Value) ||
      other._value.type() == typeid(Not_A_Value))
      return false;//always different!
    return this->operator!=(other) && !this->operator<(other);
  }
}