/*

Usefull to read files in raw video format

*/
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>
#include "opencv2/imgproc.hpp"
#include <fstream>
#include <cstdint>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using std::string;
using std::vector;
using namespace std;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(BlockYUVreader);
  //You can add methods, attributs, reimplement needed functions...
protected:
  ifstream * file;

  uint8_t *_y;           /**< Used internally. >*/
  uint8_t *_u;           /**< Used internally. >*/
  uint8_t *_v;           /**< Used internally. >*/
  int _width;
  int _height;
  int _nr_frames;
  string _filename;

public:
  ~BlockYUVreader();
  virtual void init();
  virtual void release();
  BLOCK_END_INSTANTIATION(BlockYUVreader, AlgoType::input, BLOCK__INPUT_RAW_VIDEO_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(BlockYUVreader);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(false, FilePath, "BLOCK__INPUT_IN_INPUT_FILE", "BLOCK__INPUT_IN_INPUT_FILE_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP", 640);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP", 480);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(BlockYUVreader);
  ADD_PARAMETER(true, Matrix, "BLOCK__INPUT_OUT_IMAGE", "BLOCK__INPUT_OUT_IMAGE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(BlockYUVreader);
  END_BLOCK_PARAMS();

  BlockYUVreader::BlockYUVreader() :Block("BLOCK__INPUT_RAW_VIDEO_NAME", false){
    _myInputs["BLOCK__INPUT_IN_INPUT_FILE"].addValidator({ new ValFileExist() });
    _myInputs["BLOCK__INPUT_INOUT_WIDTH"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].addValidator({ new ValPositiv(true) });
    _myInputs["BLOCK__INPUT_INOUT_POS_FRAMES"].addValidator({ new ValPositiv(false) });

    file = NULL;
    _y = NULL;
    _u = NULL;
    _v = NULL;
    _width = -1;
    _height = -1;
  };

  BlockYUVreader::~BlockYUVreader() {
    release();
  }

  void BlockYUVreader::init() {
    release(); //just in case an other file was opened before
    if (!boost::filesystem::exists(_filename))
      return;

    file = new ifstream(_filename, ios::in | ios::binary);
    if (!file->is_open())
    {
      delete file;
      file = NULL;
      return;
    }

    _nr_frames = 0;

    bool valueSet = false;
    if (_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue()) {
      //try to get resolution from filename (containing width x height), otherwise set to default VGA
      boost::smatch match;
      if (boost::regex_match(_filename, match, boost::regex(".*\\D(\\d+)x\\d+.*"))) {
        _width = boost::lexical_cast<int>(match[1]);
        valueSet = true;
      }
    }
    if (!valueSet)
      _width = _myInputs["BLOCK__INPUT_INOUT_WIDTH"].get<int>();

    valueSet = false;
    if (_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue()) {
      boost::smatch match;
      if (boost::regex_match(_filename, match, boost::regex(".*\\d+x(\\d+)\\D.*")))
      {
        valueSet = true;
        _height = boost::lexical_cast<int>(match[1]);
      }
    }
    if (!valueSet)
      _height = _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].get<int>();

    _y = (uint8_t *)calloc(_width * _height, sizeof(uint8_t));
    _u = (uint8_t *)calloc(_width * _height / 4, sizeof(uint8_t));
    _v = (uint8_t *)calloc(_width * _height / 4, sizeof(uint8_t));

    //those parameters are not fully processed...
    _myInputs["BLOCK__INPUT_INOUT_WIDTH"].setNew(false);
    _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].setNew(false);
  }

  void BlockYUVreader::release() {
    if (file != NULL) {
      if (file->is_open()) {
        file->close();
      }
      delete file;
      file = NULL;
    }
    free(_y);
    _y = NULL;
    free(_u);
    _u = NULL;
    free(_v);
    _v = NULL;
  }

  bool BlockYUVreader::run(bool oneShot){
    if (!_myInputs["BLOCK__INPUT_IN_INPUT_FILE"].isDefaultValue()) {
      if (_myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>() != _filename) {
        _filename = _myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>();
        release();
      }
    }
    if (!_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue() && _myInputs["BLOCK__INPUT_INOUT_WIDTH"].isNew()) {
      release();
    }
    if (!_myInputs["BLOCK__INPUT_INOUT_HEIGHT"].isDefaultValue() && _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].isNew()) {
      release();
    }

    if (file == NULL) {
      init();
      if (file == NULL) return false;
    }

    if (file->eof()){
      release();
      //end of file...
      paramsFullyProcessed();
      return false;//return false as a new img was not produced!
    }

    if (!file->read((char*)_y, _width * _height) ||
      !file->read((char*)_u, _width * _height / 4) ||
      !file->read((char*)_v, _width * _height / 4))
    {
      release();
      //end of file...
      paramsFullyProcessed();
      return false;//return false as a new img was not produced!
    }

    cv::Mat cv_yuv[3], output;
    cv_yuv[0] = cv::Mat(_height, _width, CV_8UC1, _y);
    cv::Mat cv_u_half = cv::Mat(_height / 2, _width / 2, CV_8UC1, _u);
    cv::Mat cv_v_half = cv::Mat(_height / 2, _width / 2, CV_8UC1, _v);
    cv::resize(cv_u_half, cv_yuv[1], cv::Size(), 2, 2, cv::INTER_NEAREST);
    cv::resize(cv_v_half, cv_yuv[2], cv::Size(), 2, 2, cv::INTER_NEAREST);
    cv::merge(cv_yuv, 3, output);
    cv::cvtColor(output, output, cv::COLOR_YUV2RGB);
    _myOutputs["BLOCK__INPUT_OUT_IMAGE"] = output;
    _nr_frames++;
    if (oneShot) {
      release();
    }
    //test if there is still data:
    if (file != NULL)
        if (file->peek() == EOF)
          paramsFullyProcessed();
    return true;
  };

};