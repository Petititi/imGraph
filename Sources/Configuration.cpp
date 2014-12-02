
#include "Configuration.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif
#include "view/Window.h"
#include "view/GraphicView.h"

#include "SubBlock.h"
#include "Graph.h"
#include "ProcessManager.h"
#include "blocks/ParamValidator.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  GlobalConfig* GlobalConfig::_ptr;
  recursive_mutex _configMutex;

  GlobalConfig* GlobalConfig::getInstance()
  {
    lock_guard<recursive_mutex> guard(_configMutex);
    if (_ptr == NULL)
      _ptr = new GlobalConfig();
    return _ptr;
  }
  void GlobalConfig::release()
  {
    lock_guard<recursive_mutex> guard(_configMutex);
    if (_ptr != NULL)
      delete _ptr;
    _ptr = NULL;
  }

  void GlobalConfig::saveConfig()
  {
    ptree localElement;
    localElement.put("GlobConfig.LastProject", lastProject_);
    localElement.put("GlobConfig.PrefLang", prefLang_);
    localElement.put("GlobConfig.styleSheet", styleSheet_);
    localElement.put("GlobConfig.ShowMaximized", isMaximized);
    localElement.put("GlobConfig.lastPosition.x", lastPosition.x());
    localElement.put("GlobConfig.lastPosition.y", lastPosition.y());
    localElement.put("GlobConfig.lastPosition.width", lastPosition.width());
    localElement.put("GlobConfig.lastPosition.height", lastPosition.height());
    localElement.put("GlobConfig.processSync", processSync_);

    charliesoft::GraphOfProcess* graph = Window::getInstance()->getMainWidget()->getModel();
    while (graph->getParent() != NULL)
      graph = graph->getParent();

    graph->saveGraph(localElement);

    if (lastProject_.empty())
      lastProject_ = "config.xml";
    boost::property_tree::xml_writer_settings<char> settings(' ', 2);
    write_xml(lastProject_, localElement, std::locale(), settings);
  }

  void GlobalConfig::loadConfig()
  {
    bool xmlOK = false;
    ptree xmlTree;
    //try to read config.xml:
    ifstream ifs("config.xml");
    if (ifs.is_open())
    {
      string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
      stringstream contentStreamed;
      contentStreamed << str;
      try
      {
        read_xml(contentStreamed, xmlTree);
        xmlOK = true;
      }
      catch (boost::property_tree::ptree_bad_path&)
      {
        //nothing to do...
      }
    }
    if (xmlOK)
    {
      lastProject_ = xmlTree.get("GlobConfig.LastProject", "");
      prefLang_ = xmlTree.get("GlobConfig.PrefLang", "en");
      isMaximized = xmlTree.get("GlobConfig.ShowMaximized", true);
      styleSheet_ = xmlTree.get("GlobConfig.styleSheet", "");
      processSync_ = xmlTree.get("GlobConfig.processSync", true);

      lastPosition.setLeft(xmlTree.get("GlobConfig.lastPosition.x", 0));
      lastPosition.setTop(xmlTree.get("GlobConfig.lastPosition.y", 0));
      lastPosition.setWidth(xmlTree.get("GlobConfig.lastPosition.width", 1024));
      lastPosition.setHeight(xmlTree.get("GlobConfig.lastPosition.height", 768));

      charliesoft::GraphOfProcess* graph = Window::getInstance()->getMainWidget()->getModel();
      graph->fromGraph(xmlTree);
    }
    else
    {
      styleSheet_ = "QToolTip {font-style:italic; color: #ffffff; background-color: #2a82aa; border: 1px solid white;}"
        "QWidget#MainWidget{ background:white; background-image:url(logo.png); background-repeat:no-repeat; background-position:center; }"
        "QWidget#DraggableWidget{ max - height:50px; padding: 2px; margin:5px; border:1px solid #888; border-radius: 5px;"
        " background: qradialgradient(cx : 0.3, cy : -0.4, fx : 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #bbb); }"
        "QWidget#VertexRepresentation{ border:2px solid #555; border-radius: 11px;"
        " background: qradialgradient(cx : 0.3, cy : -0.4, fx : 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #888); }"
        "QWidget#VertexTitle{ background - color:rgba(255, 255, 255, 32); border:none; border-radius:5px; }"
        "QWidget#VertexTitleLine{ border: 2px solid #555; border-radius:0px; }"
        "QWidget#ParamRepresentation{ background-color:rgba(255, 255, 255, 255); border:1px solid #555; padding:1px; }";
      lastProject_ = "";
      prefLang_ = "en";
      isMaximized = true;
      lastPosition.setLeft(0);
      lastPosition.setTop(0);
      lastPosition.setWidth(1024);
      lastPosition.setHeight(768);
    }
  }
}