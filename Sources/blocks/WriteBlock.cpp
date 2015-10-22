
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif


#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(WriteVideo);
  //You can add methods, re implement needed functions...
  cv::VideoWriter vr;
  virtual void release();
  ///\todo: add init and release functions
  BLOCK_END_INSTANTIATION(WriteVideo, AlgoType::output, BLOCK__WRITE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(WriteVideo);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__WRITE_IN_IMAGE", "BLOCK__WRITE_IN_IMAGE_HELP");
  ADD_PARAMETER(userConstant, FilePath, "BLOCK__WRITE_IN_FILENAME", "BLOCK__WRITE_IN_FILENAME_HELP");
  ADD_PARAMETER_FULL(false, Float, "BLOCK__WRITE_IN_FPS", "BLOCK__WRITE_IN_FPS_HELP", 25.);
  ADD_PARAMETER_FULL(false, String, "BLOCK__WRITE_IN_CODEC", "BLOCK__WRITE_IN_CODEC_HELP", "XVID");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(WriteVideo);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(WriteVideo);
  END_BLOCK_PARAMS();

  WriteVideo::WriteVideo() :Block("BLOCK__WRITE_NAME", true){
    _myInputs["BLOCK__WRITE_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__WRITE_IN_FILENAME"].addValidator({ new ValNeeded() });
  };

  void WriteVideo::release()
  {
    vr.release();
  }

  bool WriteVideo::run(bool oneShot){
    if (oneShot)
      return true;//nothing to do (risk of overriding file!
    cv::Mat out = _myInputs["BLOCK__WRITE_IN_IMAGE"].get<cv::Mat>();
    if (out.empty())
      return false;
    if (!vr.isOpened()
      || _myInputs["BLOCK__WRITE_IN_FILENAME"].isNew()
      || _myInputs["BLOCK__WRITE_IN_CODEC"].isNew()
      || _myInputs["BLOCK__WRITE_IN_FPS"].isNew())
    {
      if (vr.isOpened())
        vr.release();
      string filename = _myInputs["BLOCK__WRITE_IN_FILENAME"].get<std::string>();
      string codecName = _myInputs["BLOCK__WRITE_IN_CODEC"].get<std::string>();
      int codecCode = -1;
      if (codecName.length() == 4)
        codecCode = cv::VideoWriter::fourcc(codecName[0], codecName[1], codecName[2], codecName[3]);
      if (codecName.empty())
        codecCode = 0;
      vr.open(filename, codecCode, _myInputs["BLOCK__WRITE_IN_FPS"].get<double>(), out.size());
    }
    vr.write(out);

    return true;
  };

  BLOCK_BEGIN_INSTANTIATION(WriteImage);
  //You can add methods, re implement needed functions...
  cv::VideoWriter vr;
  ///\todo: add init and release functions
  BLOCK_END_INSTANTIATION(WriteImage, AlgoType::output, BLOCK__IMWRITE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(WriteImage);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__IMWRITE_IN_IMAGE", "BLOCK__IMWRITE_IN_IMAGE_HELP");
  ADD_PARAMETER(userConstant, FilePath, "BLOCK__IMWRITE_IN_FILENAME", "BLOCK__IMWRITE_IN_FILENAME_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__IMWRITE_IN_QUALITY", "BLOCK__IMWRITE_IN_QUALITY_HELP", 95);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(WriteImage);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(WriteImage);
  END_BLOCK_PARAMS();

  WriteImage::WriteImage() :Block("BLOCK__IMWRITE_NAME", true){
    _myInputs["BLOCK__IMWRITE_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__IMWRITE_IN_FILENAME"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__IMWRITE_IN_QUALITY"].addValidator({ new ValRange(0, 100) });
  };

  bool WriteImage::run(bool oneShot){
    cv::Mat out = _myInputs["BLOCK__IMWRITE_IN_IMAGE"].get<cv::Mat>();
    if (out.empty())
      return false;
    if (_myInputs["BLOCK__IMWRITE_IN_FILENAME"].isDefaultValue())
      return false;
    QString filename = _myInputs["BLOCK__IMWRITE_IN_FILENAME"].get<std::string>().c_str();
    int codec = 0;//no compression...
    vector<int> params;
    if (!_myInputs["BLOCK__IMWRITE_IN_QUALITY"].isDefaultValue())
    {
      int valQuality = _myInputs["BLOCK__IMWRITE_IN_QUALITY"].get<int>();
      if (filename.endsWith(".jpg", Qt::CaseInsensitive))
      {
        params.push_back(CV_IMWRITE_JPEG_QUALITY);
        params.push_back(valQuality);
      }
      else if (filename.endsWith(".png", Qt::CaseInsensitive))
      {
        params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        params.push_back((100-valQuality) / 10);
      }
      else if (filename.endsWith(".ppm", Qt::CaseInsensitive) ||
        filename.endsWith(".pgm", Qt::CaseInsensitive) ||
        filename.endsWith(".pbm", Qt::CaseInsensitive))
      {
        params.push_back(CV_IMWRITE_PXM_BINARY);
        params.push_back(valQuality>=50?0:1);
      }
    }
    try
    {
      cv::imwrite(filename.toStdString(), out, params);
    }
    catch (cv::Exception&)
    {
      return false;
    }
    return true;
  };
};