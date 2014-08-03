
#include "Internationalizator.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>


using namespace std;
using boost::recursive_mutex;
using boost::lock_guard;

namespace charliesoft
{
  recursive_mutex _internationalizatorMutex;
  Internationalizator *Internationalizator::ptr = NULL;

  Internationalizator::Internationalizator()
  {
    initTranslations();
  }

  Internationalizator* Internationalizator::getInstance()
  {
    lock_guard<recursive_mutex> guard(_internationalizatorMutex);
    if (ptr == NULL)
      ptr = new Internationalizator();
    return ptr;
  };

  void Internationalizator::releaseInstance()
  {
    lock_guard<recursive_mutex> guard(_internationalizatorMutex);
    if (ptr != NULL)
      delete ptr;
    ptr = NULL;
  };

  void Internationalizator::setLang(std::string resourceFile)
  {
    //TODO!
  };

  std::string Internationalizator::getTranslation(std::string key){
    if (translations.find(key) != translations.end())
      return translations[key];
    else
      return key;
  };


  std::string _STR(std::string key)
  {
    return Internationalizator::getInstance()->getTranslation(key).c_str();
  };

  QString _QT(std::string key)
  {
    QString output = Internationalizator::getInstance()->getTranslation(key).c_str();
    return output;
  };

  void Internationalizator::initTranslations()
  {
    translations["MENU_FILE"] = "File";
    translations["MENU_FILE_OPEN"] = "Open";
    translations["MENU_FILE_OPEN_TIP"] = "Open a previous project";
    translations["MENU_FILE_CREATE"] = "New";
    translations["MENU_FILE_CREATE_TIP"] = "Create a new project";
    translations["MENU_FILE_QUIT"] = "Quit";
    translations["MENU_FILE_QUIT_TIP"] = "Quit application";

    translations["MENU_HELP_INFO"] = "Info";
    translations["MENU_HELP_HELP"] = "Help";

    translations["DOCK_BLOCK_TITLE"] = "List of blocks";

    translations["CONF_FILE_TYPE"] = "imGraph project (*.igp)";

    translations["PROJ_LOAD_FILE"] = "Open project file";
    translations["PROJ_CREATE_FILE"] = "Create project file";


    translations["BLOCK_INPUT"] = "Input";
    translations["BLOCK_OUTPUT"] = "Output";

    translations["ERROR_GENERIC_TITLE"] = "Error";
    translations["ERROR_LINK_WRONG_INPUT_OUTPUT"] = "You can't link %1$s to %2$s : same type (%3$s)!";
    translations["ERROR_LINK_SAME_BLOCK"] = "You can't link the same block!";


    translations["BLOCK__INPUT_NAME"] = "File Loader";
    translations["BLOCK__INPUT_PARAM_IN_FILE"] = "filename";
    translations["BLOCK__INPUT_PARAM_OUT_FRAMERATE"] = "framerate";
    translations["BLOCK__INPUT_PARAM_OUT_WIDTH"] = "width";
    translations["BLOCK__INPUT_PARAM_OUT_HEIGHT"] = "height";
  }
}
