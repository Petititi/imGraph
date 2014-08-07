
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

  boost::format my_format(const std::string & f_string) {
    using namespace boost::io;
    boost::format fmter(f_string);
    fmter.exceptions(no_error_bits);//no exceptions wanted!
    return fmter;
  }

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
    translations["BUTTON_OK"] = "OK";
    translations["BUTTON_CANCEL"] = "Cancel";
    translations["BUTTON_BROWSE"] = "Browse...";

    translations["MATRIX_EDITOR"] = "Matrix editor";
    translations["VECTOR_EDITOR"] = "Vector editor";
    translations["NOT_INITIALIZED"] = "Not initialized...";
    translations["ERROR_GENERIC_TITLE"] = "Error!";
    translations["ERROR_LINK_WRONG_INPUT_OUTPUT"] = "You can't link %1$s to %2$s : same type (%3$s)!";
    translations["ERROR_LINK_SAME_BLOCK"] = "You can't link the same block!";
    translations["ERROR_PARAM_EXCLUSIF"] = "Param \"%1$s\" and \"%2$s\" are mutually exclusive...";
    translations["ERROR_PARAM_NEEDED"] = "Param \"%1$s\" is required...";
    translations["ERROR_PARAM_ONLY_POSITIF"] = "Param \"%1$s\":<br/>only positive value are authorized!";
    translations["ERROR_PARAM_ONLY_POSITIF_STRICT"] = "Param \"%1$s\":<br/>only strict positive value are authorized!";
    translations["ERROR_PARAM_VALUE_BETWEEN"] = "Param \"%1$s\":<br/>should be between %2$f and %3$f";

    translations["MENU_FILE"] = "File";
    translations["MENU_FILE_OPEN"] = "Open";
    translations["MENU_FILE_OPEN_TIP"] = "Open a previous project";
    translations["MENU_FILE_CREATE"] = "New";
    translations["MENU_FILE_CREATE_TIP"] = "Create a new project";
    translations["MENU_FILE_QUIT"] = "Quit";
    translations["MENU_FILE_QUIT_TIP"] = "Quit application";

    translations["MENU_HELP_INFO"] = "Info";
    translations["MENU_HELP_HELP"] = "Help";

    translations["CONF_FILE_TYPE"] = "imGraph project (*.igp)";

    translations["PROJ_LOAD_FILE"] = "Open project file";
    translations["PROJ_CREATE_FILE"] = "Create project file";

    translations["DOCK_TITLE"] = "Toolbox";


    translations["BLOCK_TITLE_INPUT"] = "Input";
    translations["BLOCK_TITLE_IMG_PROCESS"] = "2D processing";
    translations["BLOCK_TITLE_SIGNAL"] = "1D processing";
    translations["BLOCK_TITLE_MATH"] = "Math op.";
    translations["BLOCK_TITLE_OUTPUT"] = "Output";

    translations["BLOCK__INPUT_NAME"] = "File Loader";
    translations["BLOCK__INPUT_IN_FILE"] = "filename";
    translations["BLOCK__INPUT_IN_FILE_FILTER"] = "media files";
    translations["BLOCK__INPUT_IN_FILE_HELP"] = "File used to load the image(s).";
    translations["BLOCK__INPUT_IN_FILE_NOT_FOUND"] = "File \"%1$s\" not found!";
    translations["BLOCK__INPUT_IN_FILE_PROBLEM"] = "File \"%1$s\" can't be loaded!";
    translations["BLOCK__INPUT_IN_GREY"] = "grey";
    translations["BLOCK__INPUT_IN_GREY_HELP"] = "Convert image to a grayscale one";
    translations["BLOCK__INPUT_IN_COLOR"] = "color";
    translations["BLOCK__INPUT_IN_COLOR_HELP"] = "Convert image to a color one";
    translations["BLOCK__INPUT_OUT_IMAGE"] = "image";
    translations["BLOCK__INPUT_OUT_IMAGE_HELP"] = "Output image";
    translations["BLOCK__INPUT_OUT_FRAMERATE"] = "framerate";
    translations["BLOCK__INPUT_OUT_FRAMERATE_HELP"] = "Number of frames per second";
    translations["BLOCK__INPUT_INOUT_WIDTH"] = "width";
    translations["BLOCK__INPUT_INOUT_WIDTH_HELP"] = "Wanted width of images (in pixels)";
    translations["BLOCK__INPUT_INOUT_HEIGHT"] = "height";
    translations["BLOCK__INPUT_INOUT_HEIGHT_HELP"] = "Wanted height of images (in pixels)";
    translations["BLOCK__INPUT_INOUT_POS_FRAMES"] = "position";
    translations["BLOCK__INPUT_INOUT_POS_FRAMES_HELP"] = "0-based index of the frame to be decoded/captured";
    translations["BLOCK__INPUT_INOUT_POS_RATIO"] = "pos. ratio";
    translations["BLOCK__INPUT_INOUT_POS_RATIO_HELP"] = "Relative position in the video file (0-begining, 1-end)";
    translations["BLOCK__INPUT_OUT_FORMAT"] = "out format";
    translations["BLOCK__INPUT_OUT_FORMAT_HELP"] = "The format of the Mat objects";

    translations["BLOCK__OUTPUT_NAME"] = "Display image";
    translations["BLOCK__OUTPUT_IN_IMAGE"] = "image";
    translations["BLOCK__OUTPUT_IN_IMAGE_HELP"] = "Output image";
    translations["BLOCK__OUTPUT_IN_WIN_NAME"] = "win. title";
    translations["BLOCK__OUTPUT_IN_WIN_NAME_HELP"] = "Windows title";
    translations["BLOCK__OUTPUT_IN_WIDTH"] = translations["BLOCK__INPUT_INOUT_WIDTH"];
    translations["BLOCK__OUTPUT_IN_WIDTH_HELP"] = translations["BLOCK__INPUT_INOUT_WIDTH_HELP"];
    translations["BLOCK__OUTPUT_IN_HEIGHT"] = translations["BLOCK__INPUT_INOUT_HEIGHT"];
    translations["BLOCK__OUTPUT_IN_HEIGHT_HELP"] = translations["BLOCK__INPUT_INOUT_HEIGHT_HELP"];

  }
}
