#ifndef _CONFIGURATOR_HEADER_
#define _CONFIGURATOR_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif

#include <boost/property_tree/ptree.hpp>
#include <QRect>
#include <string>

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace charliesoft
{
  class GlobalConfig
  {
    GlobalConfig(){};
    ~GlobalConfig(){};
    static GlobalConfig* _ptr;

    boost::property_tree::ptree _xmlTree;
  public:
    static GlobalConfig* getInstance();
    static void release();
    void loadConfig();
    void saveConfig();

    boost::property_tree::ptree getXML() const { return _xmlTree; }

    bool processSync_;
    std::string lastProject_;
    std::string prefLang_;
    std::string styleSheet_;
    bool isMaximized;
    QRect lastPosition;
  };
}


#endif