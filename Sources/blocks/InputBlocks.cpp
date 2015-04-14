
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "ParamValidator.h"
#include "InputProcessor.h"
#ifdef _WIN32
#pragma warning(pop)
#endif
#include "Block.h"
using std::string;
using std::vector;

using namespace lsis_org;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockLoader);
  //You can add methods, attributs, reimplement needed functions...
  void openInput();
public:
  virtual void init();
  virtual void release();
protected:
  int loopCount;
  InputProcessor processor_;
  BLOCK_END_INSTANTIATION(BlockLoader, AlgoType::input, BLOCK__INPUT_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockLoader);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER_FULL(userConstant, ListBox, "BLOCK__INPUT_IN_INPUT_TYPE", "BLOCK__INPUT_IN_INPUT_TYPE_HELP", 0);
  ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__INPUT_IN_LOOP", "BLOCK__INPUT_IN_LOOP_HELP", 0);
  ADD_PARAMETER(notUsed, Boolean, "BLOCK__INPUT_IN_GREY", "BLOCK__INPUT_IN_GREY_HELP");
  ADD_PARAMETER(notUsed, Boolean, "BLOCK__INPUT_IN_COLOR", "BLOCK__INPUT_IN_COLOR_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP");
  ADD_PARAMETER(notUsed, Float, "BLOCK__INPUT_OUT_FRAMERATE", "BLOCK__INPUT_OUT_FRAMERATE");
  END_BLOCK_PARAMS();
  
  BEGIN_BLOCK_OUTPUT_PARAMS(BlockLoader);
  ADD_PARAMETER(true, Matrix,  "BLOCK__INPUT_OUT_IMAGE",      "BLOCK__INPUT_OUT_IMAGE_HELP");
  ADD_PARAMETER(notUsed, Float, "BLOCK__INPUT_OUT_FRAMERATE", "BLOCK__INPUT_OUT_FRAMERATE_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
  ADD_PARAMETER(notUsed, Float, "BLOCK__INPUT_INOUT_POS_RATIO", "BLOCK__INPUT_INOUT_POS_RATIO_HELP");
  ADD_PARAMETER(notUsed, Int, "BLOCK__INPUT_OUT_FORMAT", "BLOCK__INPUT_OUT_FORMAT_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockLoader);
  ADD_PARAMETER_FULL(notUsed, FilePath, "BLOCK__INPUT_IN_INPUT_TYPE.Video file.input file", "input file", "");
  ADD_PARAMETER_FULL(notUsed, FilePath, "BLOCK__INPUT_IN_INPUT_TYPE.Folder.input folder", "input folder", "");
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__INPUT_IN_INPUT_TYPE.Webcam.webcam index", "webcam index", 0);
  END_BLOCK_PARAMS();

  BlockLoader::BlockLoader() :Block("BLOCK__INPUT_NAME", false){
    _mySubParams["BLOCK__INPUT_IN_INPUT_TYPE.Video file.input file"].addValidator({ new ValFileExist() });
    _mySubParams["BLOCK__INPUT_IN_INPUT_TYPE.Folder.input folder"].addValidator({ new FileIsFolder() });
    _myInputs["BLOCK__INPUT_INOUT_WIDTH"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__INPUT_INOUT_POS_FRAMES"].addValidator({ new ValPositiv(false) });
    _myInputs["BLOCK__INPUT_INOUT_POS_RATIO"].addValidator({ new ValRange(0, 1) });
    _myInputs["BLOCK__INPUT_IN_GREY"].addValidator({ new ValExclusif(_myInputs["BLOCK__INPUT_IN_COLOR"]) });
    _myInputs["BLOCK__INPUT_IN_COLOR"].addValidator({ new ValExclusif(_myInputs["BLOCK__INPUT_IN_GREY"]) });
    _myInputs["BLOCK__INPUT_OUT_FRAMERATE"].addValidator({ new ValPositiv(true) });
  };

  void BlockLoader::openInput()
  {
    int inputType = _myInputs["BLOCK__INPUT_IN_INPUT_TYPE"].get<int>();
    switch (inputType)
    {
    case 0://webcam
    {
      int IdWebcam = _mySubParams["BLOCK__INPUT_IN_INPUT_TYPE.Webcam.webcam index"].get<int>();
      if (!processor_.setInputSource(IdWebcam))
        _error_msg = (my_format(_STR("BLOCK__INPUT_IN_FILE_PROBLEM")) % "WebCam").str();
    }
    break;
    case 2://folder
    {
      string fileName = _mySubParams["BLOCK__INPUT_IN_INPUT_TYPE.Folder.input folder"].get<string>();
      if (!processor_.setInputSource(fileName))
        _error_msg = (my_format(_STR("BLOCK__INPUT_IN_FILE_PROBLEM")) % fileName).str();
    }
    break;
    default://video
    {
      string fileName = _mySubParams["BLOCK__INPUT_IN_INPUT_TYPE.Video file.input file"].get<string>();
      if (!processor_.setInputSource(fileName))
        _error_msg = (my_format(_STR("BLOCK__INPUT_IN_FILE_PROBLEM")) % fileName).str();
    }
    }

    ParamValue& tmpParam = _myInputs["BLOCK__INPUT_IN_GREY"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      if (tmpParam.get<bool>())
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 0);

    tmpParam = _myInputs["BLOCK__INPUT_IN_COLOR"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      if (tmpParam.get<bool>())
        processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 1);

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_WIDTH"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH,
        tmpParam.get<int>());

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_HEIGHT"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_FRAME_HEIGHT,
        tmpParam.get<int>());

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_POS_FRAMES"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_POS_FRAMES,
        tmpParam.get<double>());

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_POS_RATIO"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      processor_.setProperty(cv::CAP_PROP_POS_AVI_RATIO,
        tmpParam.get<double>());

    double fps = -1;// MAX(1, processor_.getProperty(cv::CAP_PROP_FPS));
    tmpParam = _myInputs["BLOCK__INPUT_OUT_FRAMERATE"];
    tmpParam.setNew(false);
    if (!tmpParam.isDefaultValue())
      fps = tmpParam.get<double>();
  }

  void BlockLoader::init()
  {
    loopCount = -1;
  }

  void BlockLoader::release()
  {
    processor_.close();
  }

  bool BlockLoader::run(bool oneShot){

    //get current frame from stream:
    if (!processor_.isOpened())//either end of file or problem with file...
    {
      loopCount++;
      if (!oneShot)
      {
        int wantedLoop = 0;
        if (!_myInputs["BLOCK__INPUT_IN_LOOP"].isDefaultValue())
          wantedLoop = _myInputs["BLOCK__INPUT_IN_LOOP"].get<int>();
        if (wantedLoop > 0)
        {
          if (wantedLoop <= loopCount)
            return false;//nothing to do here...
        }
      }
      openInput();
      if (!processor_.isOpened())
        return false;//error
    }

    ParamValue& tmpParam = _myInputs["BLOCK__INPUT_IN_GREY"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        if (tmpParam.get<bool>())
          processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 0);
    }

    tmpParam = _myInputs["BLOCK__INPUT_IN_COLOR"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        if (tmpParam.get<bool>())
          processor_.setProperty(cv::CAP_PROP_CONVERT_RGB, 1);
    }

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_WIDTH"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        processor_.setProperty(cv::CAP_PROP_FRAME_WIDTH,
          tmpParam.get<int>());
    }

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_HEIGHT"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        processor_.setProperty(cv::CAP_PROP_FRAME_HEIGHT,
          tmpParam.get<int>());
    }

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_POS_FRAMES"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        processor_.setProperty(cv::CAP_PROP_POS_FRAMES,
          tmpParam.get<double>());
    }

    tmpParam = _myInputs["BLOCK__INPUT_INOUT_POS_RATIO"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        processor_.setProperty(cv::CAP_PROP_POS_AVI_RATIO,
          tmpParam.get<double>());
    }

    double fps = -1;// MAX(1, processor_.getProperty(cv::CAP_PROP_FPS));
    tmpParam = _myInputs["BLOCK__INPUT_OUT_FRAMERATE"];
    if (tmpParam.isNew())
    {
      tmpParam.setNew(false);
      if (!tmpParam.isDefaultValue())
        fps = tmpParam.get<double>();
    }
    
    cv::Mat frame = processor_.getFrame();
    if (frame.empty())//either end of file or problem with file...
      return false;//error

    //now set outputs:
    _myOutputs["BLOCK__INPUT_OUT_IMAGE"] = frame;
    _myOutputs["BLOCK__INPUT_OUT_FRAMERATE"] = fps;
    _myOutputs["BLOCK__INPUT_INOUT_WIDTH"] = frame.cols;
    _myOutputs["BLOCK__INPUT_INOUT_HEIGHT"] = frame.rows;
    _myOutputs["BLOCK__INPUT_INOUT_POS_FRAMES"] = (int)processor_.getProperty(cv::CAP_PROP_POS_FRAMES);
    _myOutputs["BLOCK__INPUT_INOUT_POS_RATIO"] = processor_.getProperty(cv::CAP_PROP_POS_AVI_RATIO);
    _myOutputs["BLOCK__INPUT_OUT_FORMAT"] = frame.type();


    //wait corresponding ms in order to keep fps:
    if (fps>0)
      boost::this_thread::sleep(boost::posix_time::milliseconds(static_cast<int64_t>((1. / fps)*1000)));

    if (processor_.isEndOfFile())
    {
      release();
      paramsFullyProcessed();
    }
    return true;
  };

};