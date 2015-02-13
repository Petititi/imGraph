#ifndef _BLOCK_PARAM__valueHEADER_
#define _BLOCK_PARAM__valueHEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <boost/variant.hpp>
#include <opencv2/core/core.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <set>
#include <QString>
#ifdef _WIN32
#pragma warning(pop)
#endif

namespace charliesoft
{
  class ParamValue;
  class ParamValidator;
  class Block;
  struct BlockLink;
  struct ParamDefinition;

  enum ParamType
  {
    Boolean = 0, Int, Float, Color, Matrix, String, FilePath, ListBox, AnyType, typeError
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
    cv::Scalar,
    cv::Mat,
    ParamValue*,
    Not_A_Value > VariantClasses;

  class ParamValue
  {
    mutable boost::recursive_mutex _mtx;    //< explicit mutex declaration
    boost::condition_variable _cond_sync;  //< global sync condition
    mutable bool _newValue;  //< is this value not yet processed?
    std::vector<ParamValidator*> _validators;
    std::set<ParamValue*> _distantListeners;
    Block *_block;
    const ParamDefinition* _definition;
    std::string _name;
    bool _isOutput;
    bool _paramNeeded;

    void notifyUpdate(bool isNew);
    void notifyRemove();
  public:
    VariantClasses _value;
    ParamValue(Block *algo, std::string name, bool isOutput) :
      _block(algo), _name(name), _isOutput(isOutput), _value(Not_A_Value()){
      _newValue = false; _paramNeeded = true; _definition = NULL;
    };
    ParamValue() :
      _block(NULL), _name(""), _isOutput(false), _value(Not_A_Value()){
      _newValue = false; _paramNeeded = true; _definition = NULL;
    };
    ParamValue(Block *algo, const ParamDefinition* def, bool isOutput);
    ParamValue(bool v) : ParamValue(){
      _value = v;
    };
    ParamValue(int v) :ParamValue(){
      _value = v;
    };
    ParamValue(double v) :ParamValue(){
      _value = v;
    };
    ParamValue(char* v) :ParamValue(std::string(v)){
    };
    ParamValue(std::string v) :ParamValue(){
      _value = v;
    };
    ParamValue(cv::Scalar v) :ParamValue(){
      _value = v;
    };
    ParamValue(cv::Mat v) :ParamValue(){
      _value = v;
    };
    ParamValue(Not_A_Value v) :ParamValue(){
    };
    ParamValue(ParamValue* v) :ParamValue(){
      _value = v;
      if (v != NULL) v->_distantListeners.insert(this);
    };
    ParamValue(const ParamValue& va) :
      _block(va._block), _name(va._name), _isOutput(va._isOutput), _value(va._value),
      _validators(va._validators), _distantListeners(va._distantListeners){
      _newValue = false; _paramNeeded = true; _definition = va._definition;
    };

    ~ParamValue()
    {
      if (isLinked())
        boost::get<ParamValue*>(_value)->_distantListeners.erase(this);
      for (auto it = _distantListeners.begin();
        it != _distantListeners.end(); it++)
      {
        if (*it != NULL)
          (*it)->_value = Not_A_Value();
      }
    }

    const std::set<ParamValue*>& getListeners() const { return  _distantListeners; };
    std::set<ParamValue*>& getListeners() { return  _distantListeners; };

    static ParamValue fromString(ParamType,std::string);

    ParamValue& operator=(bool const &rhs);
    ParamValue& operator=(int const &rhs);
    ParamValue& operator=(double const &rhs);
    ParamValue& operator=(std::string const &rhs);
    ParamValue& operator=(cv::Scalar const &rhs);
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

    ///Update the value. This will render the corresponding block and every ancestors.
    void update();

    bool isNeeded(){ return _paramNeeded; };
    void isNeeded(bool paramNeeded){ _paramNeeded = paramNeeded; };

    std::string toString() const;
    BlockLink toBlockLink() const;

    void validate(const ParamValue& other) const;
    void addValidator(std::initializer_list<ParamValidator*> list);
    template <class T>
    bool containValidator() const
    {
      for (auto& val : _validators)
      {
        if (dynamic_cast<T*>(val) != NULL)
          return true;
      }
      return false;
    };

    std::string getName() const { return _name; };
    const ParamDefinition* getDefinition() const { return _definition; }
    void setName(std::string val) { _name = val; }

    bool isNew() const
    {
      return _newValue;
    }
    void setNew(bool isNew){ _newValue = isNew; };
    bool isDefaultValue() const;
    bool isLinked() const {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      return (_value.type() == typeid(ParamValue*)) &&
        boost::get<ParamValue*>(_value) != NULL;
    };

    ParamType getType(bool realType=true) const;
    Block * getBlock() const { return _block; };
    void setBlock(Block *b) { _block=b; };

    std::string getValFromList();

    template<typename T>
    T get() const
    {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      if (isLinked())
      {
        if (typeid(T) == typeid(ParamValue*))
          return boost::get<T>(_value);
        return boost::get<ParamValue*>(_value)->get<T>();
      }
      if (_value.type() == typeid(Not_A_Value))
      {
        return T();
      }
      try
      {
        return boost::get<T>(_value);
      }
      catch (boost::bad_get&)
      {
        return T();
      }
    }

    void setValue(const ParamValue* value)
    {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      notifyRemove();
      _newValue = true;
      if (value->isLinked())
        value = value->get<ParamValue*>();
      switch (value->getType())
      {
      case Boolean:
        _value = value->get<bool>();
        break;
      case Int:
        _value = value->get<int>();
        break;
      case Float:
        _value = value->get<double>();
        break;
      case String:
      case FilePath:
        _value = value->get<std::string>();
        break;
      case Color:
        _value = value->get<cv::Scalar>();
        break;
      default:
        _value = value->get<cv::Mat>();
        break;
      }
      notifyUpdate(_newValue);
    }

    void valid_and_set(const ParamValue& v);

  };
}

#endif