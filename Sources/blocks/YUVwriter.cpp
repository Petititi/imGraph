/*

Usefull to write files in raw video format

*/
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>
#include "opencv2/imgproc.hpp"
#include <fstream>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using std::string;
using namespace std;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockYUVwriter);
  //You can add methods, attributs, reimplement needed functions...
protected:
  ofstream * file;
  string _filename;

public:
  ~BlockYUVwriter();
  virtual void init();
  virtual void release();
  BLOCK_END_INSTANTIATION(BlockYUVwriter, AlgoType::output, BLOCK__OUTPUT_RAW_VIDEO_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockYUVwriter);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__WRITE_IN_IMAGE", "BLOCK__WRITE_IN_IMAGE_HELP");
  ADD_PARAMETER(userConstant, FilePath, "BLOCK__WRITE_IN_FILENAME", "BLOCK__WRITE_IN_FILENAME_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockYUVwriter);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockYUVwriter);
  END_BLOCK_PARAMS();

  BlockYUVwriter::BlockYUVwriter() :Block("BLOCK__OUTPUT_RAW_VIDEO_NAME", true){
    _myInputs["BLOCK__WRITE_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__WRITE_IN_FILENAME"].addValidator({ new ValNeeded() });
    file = NULL;
  };

  BlockYUVwriter::~BlockYUVwriter() {
    release();
  }

  void BlockYUVwriter::init() {
    release(); //just in case an other file was opened before
    file = new ofstream(_filename, ios::out | ios::binary);
  }

  void BlockYUVwriter::release() {
    if (file != NULL) {
      if (file->is_open())
        file->close();

      delete file;
      file = NULL;
    }
  }

  bool BlockYUVwriter::run(bool oneShot){
    if (oneShot) {
      return true;
    }

    if (file == NULL || _myInputs["BLOCK__WRITE_IN_FILENAME"].get<string>() != _filename) {
      _filename = _myInputs["BLOCK__WRITE_IN_FILENAME"].get<string>();
      init();
      if (file == NULL)
        return false;
    }
    if (!file->is_open()) {
      release();
      return false;
    }
    cv::Mat out = _myInputs["BLOCK__WRITE_IN_IMAGE"].get<cv::Mat>().clone();
    cv::Mat cv_yuv[3], cv_u_half, cv_v_half;
    cv::cvtColor(out, out, cv::COLOR_BGR2YUV);
    cv::split(out, cv_yuv);

    file->write((char*)cv_yuv[0].data, cv_yuv[0].size().width * cv_yuv[0].size().height);

    cv::resize(cv_yuv[1], cv_u_half, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
    cv::resize(cv_yuv[2], cv_v_half, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);

    file->write((char*)cv_v_half.data, cv_v_half.size().width * cv_v_half.size().height); //chromas are inverted, copy V first
    file->write((char*)cv_u_half.data, cv_u_half.size().width * cv_u_half.size().height);

    return true;
  };

};