
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
    translations["BUTTON_DELETE"] = "Delete";
    translations["BUTTON_BROWSE"] = "Browse...";
    translations["BUTTON_UPDATE"] = "Update";
    translations["BUTTON_COLOR"] = "Color";

    translations["MATRIX_EDITOR"] = "Matrix editor";
    translations["VECTOR_EDITOR"] = "Vector editor";
    translations["CONDITION_EDITOR"] = "Condition editor...";
    translations["CONDITION_EDITOR_HELP"] = "You can define here which conditions are needed for block rendering!";
    translations["NOT_INITIALIZED"] = "Not initialized...";
    translations["CONDITION_BLOCK_ERROR_INPUT"] = "Condition can't be input of block...";
    translations["CONDITION_BLOCK_LEFT"] = "left";
    translations["CONDITION_BLOCK_HELP"] = "link to the output's block<br/>whose value will be used in condition";
    translations["CONDITION_BLOCK_RIGHT"] = "right";
    translations["CONDITION_CARDINAL"] = "#rendering";
    translations["CONDITION_IS_EMPTY"] = "is empty";
    translations["BLOCK_OUTPUT"] = "output";
    translations["BLOCK_INPUT"] = "input";
    translations["BLOCK_OUTPUT"] = "output";
    translations["ERROR_GENERIC"] = "Error undefined!";
    translations["ERROR_GENERIC_TITLE"] = "Error!";
    translations["ERROR_TYPE"] = "The type of \"%1$s.%2$s\" (%3$s) doesn't correspond to \"%4$s.%5$s\" (%6$s)";
    translations["ERROR_LINK_WRONG_INPUT_OUTPUT"] = "You can't link %1$s to %2$s : same type (%3$s)!";
    translations["ERROR_LINK_SAME_BLOCK"] = "You can't link the same block!";
    translations["ERROR_PARAM_EXCLUSIF"] = "Params \"%1$s\" and \"%2$s\" are mutually exclusive...";
    translations["ERROR_PARAM_NEEDED"] = "Param \"%1$s\" is required...";
    translations["ERROR_PARAM_ONLY_POSITIF"] = "Param \"%1$s\":<br/>only positive value are authorized!";
    translations["ERROR_PARAM_ONLY_POSITIF_STRICT"] = "Param \"%1$s\":<br/>only strict positive value are authorized!";
    translations["ERROR_PARAM_ONLY_NEGATIF"] = "Param \"%1$s\":<br/>only negative value are authorized!";
    translations["ERROR_PARAM_ONLY_NEGATIF_STRICT"] = "Param \"%1$s\":<br/>only strict negative value are authorized!";
    translations["ERROR_PARAM_VALUE_BETWEEN"] = "Param \"%1$s\" (%2$f):<br/>should be between %3$f and %4$f";

    translations["MENU_FILE"] = "File";
    translations["MENU_FILE_OPEN"] = "Open";
    translations["MENU_FILE_OPEN_TIP"] = "Open a previous project";
    translations["MENU_FILE_CREATE"] = "New";
    translations["MENU_FILE_CREATE_TIP"] = "Create a new project";
    translations["MENU_FILE_QUIT"] = "Quit";
    translations["MENU_FILE_QUIT_TIP"] = "Quit application";

    translations["MATRIX_EDITOR_TOOLS"] = "Tools";
    translations["MATRIX_EDITOR_DATA_CHOICES"] = "Matrix data type:";
    translations["MATRIX_EDITOR_DATA_SIZE"] = "Size (rows, cols, channels):";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL"] = "Initial values:";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_0"] = "zeros";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_1"] = "constant";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_2"] = "eye";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_3"] = "ellipse";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_4"] = "rect";
    translations["MATRIX_EDITOR_DATA_INITIAL_VAL_5"] = "cross";
    translations["MATRIX_EDITOR_SECTION_PEN_COLOR"] = "Color:";
    translations["MATRIX_EDITOR_SECTION_PEN_SIZE"] = "Pencil size:";

    translations["MATRIX_EDITOR_HELP_LEFT"] = "Panning left (CTRL+arrowLEFT)";
    translations["MATRIX_EDITOR_HELP_RIGHT"] = "Panning right (CTRL+arrowRIGHT)";
    translations["MATRIX_EDITOR_HELP_UP"] = "Panning up (CTRL+arrowUP)";
    translations["MATRIX_EDITOR_HELP_DOWN"] = "Panning down (CTRL+arrowDOWN)";
    translations["MATRIX_EDITOR_HELP_ZOOM_X1"] = "Zoom x1 (CTRL+P)";
    translations["MATRIX_EDITOR_HELP_ZOOM_IN"] = "Zoom in (CTRL++)";
    translations["MATRIX_EDITOR_HELP_ZOOM_OUT"] = "Zoom out (CTRL+-)";
    translations["MATRIX_EDITOR_HELP_SAVE"] = "Save current matrix (CTRL+S)";
    translations["MATRIX_EDITOR_HELP_LOAD"] = "Load new matrix (CTRL+O)";
    translations["MATRIX_EDITOR_HELP_EDIT"] = "Edit matrix (CTRL+E)";

    translations["MENU_HELP_INFO"] = "Info";
    translations["MENU_HELP_HELP"] = "Help";

    translations["CONF_FILE_TYPE"] = "imGraph project (*.igp)";

    translations["PROJ_LOAD_FILE"] = "Open project file";
    translations["PROJ_CREATE_FILE"] = "Create project file";

    translations["DOCK_TITLE"] = "Toolbox";


    translations["BLOCK_TITLE_INPUT"] = "Input";
    translations["BLOCK_TITLE_IMG_PROCESS"] = "2D processing";
    translations["BLOCK_TITLE_SIGNAL"] = "Video processing";
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
    translations["BLOCK__OUTPUT_IN_IMAGE_HELP"] = "Image to show";
    translations["BLOCK__OUTPUT_IN_WIN_NAME"] = "win. title";
    translations["BLOCK__OUTPUT_IN_WIN_NAME_HELP"] = "Windows title";
    translations["BLOCK__OUTPUT_IN_NORMALIZE"] = "normalize";
    translations["BLOCK__OUTPUT_IN_NORMALIZE_HELP"] = "Normalize image before show";

    translations["BLOCK__LINEDRAWER_NAME"] = "Draw lines";
    translations["BLOCK__LINEDRAWER_IN_LINES"] = "lines list";
    translations["BLOCK__LINEDRAWER_IN_LINES_HELP"] = "Input of lines (4 values per row)";
    translations["BLOCK__LINEDRAWER_IN_WIDTH"] = translations["BLOCK__INPUT_INOUT_WIDTH"];
    translations["BLOCK__LINEDRAWER_IN_WIDTH_HELP"] = translations["BLOCK__INPUT_INOUT_WIDTH_HELP"];
    translations["BLOCK__LINEDRAWER_IN_HEIGHT"] = translations["BLOCK__INPUT_INOUT_HEIGHT"];
    translations["BLOCK__LINEDRAWER_IN_HEIGHT_HELP"] = translations["BLOCK__INPUT_INOUT_HEIGHT_HELP"];
    translations["BLOCK__LINEDRAWER_OUT_IMAGE"] = "image";
    translations["BLOCK__LINEDRAWER_OUT_IMAGE_HELP"] = "Binary output image";

    translations["BLOCK__NORMALIZ_NAME"] = "Normalize image";
    translations["BLOCK__NORMALIZ_IN_IMAGE"] = "image";
    translations["BLOCK__NORMALIZ_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__NORMALIZ_OUT_IMAGE"] = "image";
    translations["BLOCK__NORMALIZ_OUT_IMAGE_HELP"] = "Normalized image";

    translations["BLOCK__LINE_FINDER_NAME"] = "Find lines";
    translations["BLOCK__LINE_FINDER_IN_IMAGE"] = "image";
    translations["BLOCK__LINE_FINDER_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__LINE_FINDER_OUT_IMAGE"] = "lines list";
    translations["BLOCK__LINE_FINDER_OUT_IMAGE_HELP"] = "List of detected lines";

    translations["BLOCK__DEINTERLACE_NAME"] = "Deinterlace";
    translations["BLOCK__DEINTERLACE_IN_IMAGE"] = "image";
    translations["BLOCK__DEINTERLACE_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__DEINTERLACE_IN_TYPE"] = "output type";
    translations["BLOCK__DEINTERLACE_IN_TYPE_HELP"] = "Deinterlacing type wanted";
    translations["BLOCK__DEINTERLACE_IN_TYPE1"] = "Blend";
    translations["BLOCK__DEINTERLACE_IN_TYPE2"] = "Bob";
    translations["BLOCK__DEINTERLACE_IN_TYPE3"] = "Discard";
    translations["BLOCK__DEINTERLACE_IN_TYPE4"] = "Unfold";
    translations["BLOCK__DEINTERLACE_OUT_IMAGE"] = "image";
    translations["BLOCK__DEINTERLACE_OUT_IMAGE_HELP"] = "Deinterlaced image";

    translations["BLOCK__SKIP_FRAME_NAME"] = "Skip frame";
    translations["BLOCK__SKIP_FRAME_IN_IMAGE"] = "image";
    translations["BLOCK__SKIP_FRAME_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__SKIP_FRAME_IN_TYPE"] = "nb skip";
    translations["BLOCK__SKIP_FRAME_IN_TYPE_HELP"] = "Number of frames to skip";
    translations["BLOCK__SKIP_FRAME_OUT_IMAGE"] = "image";
    translations["BLOCK__SKIP_FRAME_OUT_IMAGE_HELP"] = "Interlaced image";

    translations["BLOCK__MERGING_NAME"] = "Merge two images";
    translations["BLOCK__MERGING_IN_IMAGE1"] = "image1";
    translations["BLOCK__MERGING_IN_IMAGE1_HELP"] = "First input image";
    translations["BLOCK__MERGING_IN_IMAGE2"] = "image2";
    translations["BLOCK__MERGING_IN_IMAGE2_HELP"] = "Second input image";
    translations["BLOCK__MERGING_OUT_IMAGE"] = "image";
    translations["BLOCK__MERGING_OUT_IMAGE_HELP"] = "Croped image";

    translations["BLOCK__CROP_NAME"] = "Crop image";
    translations["BLOCK__CROP_IN_IMAGE"] = "image";
    translations["BLOCK__CROP_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__CROP_WIDTH"] = translations["BLOCK__INPUT_INOUT_WIDTH"];
    translations["BLOCK__CROP_WIDTH_HELP"] = translations["BLOCK__INPUT_INOUT_WIDTH_HELP"];
    translations["BLOCK__CROP_HEIGHT"] = translations["BLOCK__INPUT_INOUT_HEIGHT"];
    translations["BLOCK__CROP_HEIGHT_HELP"] = translations["BLOCK__INPUT_INOUT_HEIGHT_HELP"];
    translations["BLOCK__CROP_IN_X"] = "X";
    translations["BLOCK__CROP_IN_X_HELP"] = "Position of top-left corner (X)";
    translations["BLOCK__CROP_IN_Y"] = "Y";
    translations["BLOCK__CROP_IN_Y_HELP"] = "Position of top-left corner (Y)";
    translations["BLOCK__CROP_OUT_IMAGE"] = "image";
    translations["BLOCK__CROP_OUT_IMAGE_HELP"] = "Croped image";

    translations["BLOCK__ACCUMULATOR_NAME"] = "Accumulator";
    translations["BLOCK__ACCUMULATOR_IN_IMAGE"] = "image";
    translations["BLOCK__ACCUMULATOR_IN_IMAGE_HELP"] = "Input image to accumulate";
    translations["BLOCK__ACCUMULATOR_IN_NB_HISTORY"] = "history";
    translations["BLOCK__ACCUMULATOR_IN_NB_HISTORY_HELP"] = "Size of accumulation history";
    translations["BLOCK__ACCUMULATOR_OUT_IMAGE"] = "image";
    translations["BLOCK__ACCUMULATOR_OUT_IMAGE_HELP"] = "Accumulated image";

    translations["BLOCK__MORPHOLOGIC_NAME"] = "Morpho math";
    translations["BLOCK__MORPHOLOGIC_IN_IMAGE"] = "image";
    translations["BLOCK__MORPHOLOGIC_IN_IMAGE_HELP"] = "Input image";
    translations["BLOCK__MORPHOLOGIC_ELEMENT"] = "element";
    translations["BLOCK__MORPHOLOGIC_ELEMENT_HELP"] = "Structuring element.";
    translations["BLOCK__MORPHOLOGIC_OPERATOR"] = "op";
    translations["BLOCK__MORPHOLOGIC_OPERATOR_HELP"] = "Operator (0:open, 1:close, 2:gradient, 3:tophat, 4:blackhat)";
    translations["BLOCK__MORPHOLOGIC_ITERATIONS"] = "iterations";
    translations["BLOCK__MORPHOLOGIC_ITERATIONS_HELP"] = "Number of times erosion and dilation are applied.";
    translations["BLOCK__MORPHOLOGIC_OUT_IMAGE"] = "image";
    translations["BLOCK__MORPHOLOGIC_OUT_IMAGE_HELP"] = "Filtered image";
  }
}
