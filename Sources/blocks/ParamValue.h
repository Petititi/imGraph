#ifndef _BLOCK_PARAM_VALUE_HEADER_
#define _BLOCK_PARAM_VALUE_HEADER_

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

  enum ParamType
  {
    Boolean=0, Int, Float, Color, Matrix, String, FilePath, ListBox, typeError
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
    std::vector<ParamValidator*> validators_;
    std::set<ParamValue*> distantListeners_;
    Block *block_;
    std::string _name;
    bool isOutput_;
    bool _paramNeeded;

    void notifyUpdate(bool isNew);
    void notifyRemove();
  public:
    VariantClasses value_;
    ParamValue(Block *algo, std::string name, bool isOutput) :
      block_(algo), _name(name), isOutput_(isOutput), value_(Not_A_Value()){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue() :
      block_(NULL), _name(""), isOutput_(false), value_(Not_A_Value()){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(bool v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(int v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(double v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(std::string v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(cv::Scalar v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(cv::Mat v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(Not_A_Value v) :
      block_(NULL), _name(""), isOutput_(false), value_(Not_A_Value()){
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(ParamValue* v) :
      block_(NULL), _name(""), isOutput_(false), value_(v){
      if (v != NULL) v->distantListeners_.insert(this);
      _newValue = false; _paramNeeded = true;
    };
    ParamValue(const ParamValue& va) :
      block_(va.block_), _name(va._name), isOutput_(va.isOutput_), value_(va.value_),
      validators_(va.validators_), distantListeners_(va.distantListeners_){
      _newValue = false; _paramNeeded = true;
    };

    ~ParamValue()
    {
      if (isLinked())
        boost::get<ParamValue*>(value_)->distantListeners_.erase(this);
      for (auto it = distantListeners_.begin();
        it != distantListeners_.end(); it++)
      {
        if (*it != NULL)
          (*it)->value_ = Not_A_Value();
      }
    }

    std::set<ParamValue*>& getListeners() { return  distantListeners_; };

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

    bool isNeeded(){ return _paramNeeded; };
    void isNeeded(bool paramNeeded){ _paramNeeded = paramNeeded; };

    std::string toString() const;
    BlockLink toBlockLink() const;

    void validate(const ParamValue& other) const;
    void addValidator(std::initializer_list<ParamValidator*> list);

    std::string getName() const { return _name; };

    bool isNew() const
    {
      return _newValue && !isDefaultValue();
    }
    bool isDefaultValue() const;
    bool isLinked() const {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      return (value_.type() == typeid(ParamValue*)) &&
        boost::get<ParamValue*>(value_) != NULL;
    };

    ParamType getType() const;
    Block * getBlock() const { return block_; };
    void setBlock(Block *b) { block_=b; };

    ///Param useData, when true indicate that algorithm will process this data, so it can
    ///be marked as _isNew=false...
    template<typename T>
    T get(bool useData) const
    {
      boost::unique_lock<boost::recursive_mutex> lock(_mtx);
      if (useData)
        _newValue = false;
      if (isLinked())
      {
        if (typeid(T) == typeid(ParamValue*))
          return boost::get<T>(value_);
        return boost::get<ParamValue*>(value_)->get<T>(false);
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

    void valid_and_set(const ParamValue& v);

  };
}

#endif