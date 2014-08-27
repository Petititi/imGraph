#ifndef _BLOCK_PARAM_VALUE_HEADER_
#define _BLOCK_PARAM_VALUE_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <boost/variant.hpp>
#include <boost/thread/condition_variable.hpp>
#include <opencv2/core/core.hpp>
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
    unsigned int current_timestamp_;  // timestamp of last update
    boost::condition_variable cond_;  // parameter upgrade condition
    std::vector<ParamValidator*> validators_;
    std::set<ParamValue*> distantListeners_;
    Block *block_;
    std::string name_;
    bool isOutput_;
    VariantClasses value_;

    void notifyUpdate();
  public:
    ParamValue(Block *algo, std::string name, bool isOutput) :
      block_(algo), name_(name), isOutput_(isOutput), value_(Not_A_Value()){
      current_timestamp_ = 0;
    };
    ParamValue() :
      block_(NULL), name_(""), isOutput_(false), value_(Not_A_Value()){
      current_timestamp_ = 0;
    };
    ParamValue(bool v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      current_timestamp_ = 0;
    };
    ParamValue(int v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      current_timestamp_ = 0;
    };
    ParamValue(double v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      current_timestamp_ = 0;
    };
    ParamValue(std::string v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      current_timestamp_ = 0;
    };
    ParamValue(cv::Mat v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      current_timestamp_ = 0;
    };
    ParamValue(Not_A_Value v) :
      block_(NULL), name_(""), isOutput_(false), value_(Not_A_Value()){
      current_timestamp_ = 0;
    };
    ParamValue(ParamValue* v) :
      block_(NULL), name_(""), isOutput_(false), value_(v){
      if (v != NULL) v->distantListeners_.insert(this);
      current_timestamp_ = 0;
    };
    ParamValue(ParamValue& va) :
      block_(va.block_), name_(va.name_), isOutput_(va.isOutput_), value_(va.value_){
      current_timestamp_ = 0;
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
    BlockLink toBlockLink() const;

    void validate(const ParamValue& other) const;
    void addValidator(std::initializer_list<ParamValidator*> list);

    std::string getName() const { return name_; };

    bool isDefaultValue() const;
    unsigned int getTimestamp() const{
      if (isLinked())
        return boost::get<ParamValue*>(value_)->getTimestamp();
      else
        return current_timestamp_;
    }
    bool isLinked() const {
      return (value_.type() == typeid(ParamValue*)) &&
        boost::get<ParamValue*>(value_) != NULL;
    };

    ParamType getType() const;
    Block * getBlock() const { return block_; };

    template<typename T>
    T get() const
    {
      if (isLinked())
      {
        if (typeid(T) == typeid(ParamValue*))
          return boost::get<T>(value_);
        return boost::get<ParamValue*>(value_)->get<T>();
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

    template<typename T>
    void waitForUpdate(T& mutex)
    {
      if (isLinked())
        return boost::get<ParamValue*>(value_)->waitForUpdate(mutex);
      else
        cond_.wait(mutex);
    }

    void valid_and_set(const ParamValue& v);

  };
}

#endif