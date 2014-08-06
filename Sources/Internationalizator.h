#ifndef _INTERNATIONALIZATOR_HEADER_
#define _INTERNATIONALIZATOR_HEADER_

#include <boost/format.hpp>

#include <string>
#include <map>
#include <QString>

namespace charliesoft
{
  boost::format my_format(const std::string & f_string);

  class Internationalizator
  {
    Internationalizator();
    ~Internationalizator(){};//nothing to do...

    static Internationalizator *ptr;

    std::map< std::string, std::string > translations;

    void initTranslations();
  public:
    static Internationalizator* getInstance();
    static void releaseInstance();

    void setLang(std::string resourceFile);

    std::string getTranslation(std::string key);
  };

  ///Helper function...
  QString _QT(std::string key);
  std::string _STR(std::string key);
}


#endif