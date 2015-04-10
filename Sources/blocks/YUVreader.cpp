/*

Usefull to read files in raw video format

*/
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "opencv2/opencv.hpp"
#include <fstream>
#include <cstdint>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
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
    bool _init();
    void _uninit();
    BLOCK_END_INSTANTIATION(BlockYUVreader, AlgoType::input, BLOCK__INPUT_RAW_VIDEO_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(BlockYUVreader);
    //Add parameters, with following parameters:
    //default visibility, type of parameter, name (key of internationalizor), helper...
    ADD_PARAMETER(false, FilePath, "BLOCK__INPUT_IN_INPUT_FILE", "BLOCK__INPUT_IN_INPUT_FILE_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(BlockYUVreader);
    ADD_PARAMETER(true, Matrix, "BLOCK__INPUT_OUT_IMAGE", "BLOCK__INPUT_OUT_IMAGE_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(BlockYUVreader);
    END_BLOCK_PARAMS();

    BlockYUVreader::BlockYUVreader() :Block("BLOCK__INPUT_RAW_VIDEO_NAME"){
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
        _uninit();
    }

    bool BlockYUVreader::_init() {
        _uninit(); //just in case an other file was opened before
        if (!boost::filesystem::exists(_filename)) {
            return false;
        }
        file = new ifstream(_filename, ios::in | ios::binary);
        if (!file->is_open()) {
            return false;
        }
        _nr_frames = 0;

        if (_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue()) {
            //try to get resolution from filename (containing widthxheight), otherwise set to default VGA
            boost::smatch match;
            if (boost::regex_match(_filename, match, boost::regex(".*\\D(\\d+)x\\d+.*"))) {
                _width = boost::lexical_cast<int>(match[1]);
            } else {
                _width = 640;
            }
        } else {
            _width = _myInputs["BLOCK__INPUT_INOUT_WIDTH"].get<int>(true);
        }
        if (_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue()) {
            boost::smatch match;
            if (boost::regex_match(_filename, match, boost::regex(".*\\d+x(\\d+)\\D.*"))) {
                _height = boost::lexical_cast<int>(match[1]);
            } else {
                _height = 480;
            }
        } else {
            _height = _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].get<int>(true);
        }
        _y = (uint8_t *)calloc(_width * _height, sizeof(uint8_t));
        _u = (uint8_t *)calloc(_width * _height / 4, sizeof(uint8_t));
        _v = (uint8_t *)calloc(_width * _height / 4, sizeof(uint8_t));

        return true;
    }

    void BlockYUVreader::_uninit() {
        if (file != NULL) {
            if (file->is_open()) {
                file->close();
            }
            free(file);
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
            if (_myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>(true) != _filename) {
                _filename = _myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>(true);
                _uninit();
            }
        }
        if (!_myInputs["BLOCK__INPUT_INOUT_WIDTH"].isDefaultValue() && _myInputs["BLOCK__INPUT_INOUT_WIDTH"].isNew()) {
            _uninit();
        }
        if (!_myInputs["BLOCK__INPUT_INOUT_HEIGHT"].isDefaultValue() && _myInputs["BLOCK__INPUT_INOUT_HEIGHT"].isNew()) {
            _uninit();
        }

        if (file == NULL) {
            if (!_init()) {
                return false;
            }
        }

        if (file->eof()){
            _nr_frames = 0;
            file->clear();
            file->seekg(0, ios_base::beg);
        }

        file->read((char*)_y, _width * _height);
        file->read((char*)_u, _width * _height / 4);
        file->read((char*)_v, _width * _height / 4);

        cv::Mat cv_yuv[3],output;
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
            if (file->is_open()) {
                file->clear();
                file->seekg(0,ios_base::beg); // go to beginning of the file
            }
            _nr_frames = 0;
        }
        return true;
    };

};