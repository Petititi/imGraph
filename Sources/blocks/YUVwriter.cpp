/*

Usefull to read files in raw video format

*/
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "opencv2/opencv.hpp"
#include <fstream>
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
    bool _init();
    void _uninit();
    BLOCK_END_INSTANTIATION(BlockYUVwriter, AlgoType::output, BLOCK__OUTPUT_RAW_VIDEO_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(BlockYUVwriter);
    //Add parameters, with following parameters:
    //default visibility, type of parameter, name (key of internationalizor), helper...
    ADD_PARAMETER(true, Matrix, "BLOCK__WRITE_IN_IMAGE", "BLOCK__WRITE_IN_IMAGE_HELP");
    ADD_PARAMETER(false, FilePath, "BLOCK__WRITE_IN_FILENAME", "BLOCK__WRITE_IN_FILENAME_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(BlockYUVwriter);
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(BlockYUVwriter);
    END_BLOCK_PARAMS();

    BlockYUVwriter::BlockYUVwriter() :Block("BLOCK__OUTPUT_RAW_VIDEO_NAME"){
        _myInputs["BLOCK__WRITE_IN_IMAGE"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__WRITE_IN_FILENAME"].addValidator({ new ValNeeded() });
        file = NULL;
    };

    BlockYUVwriter::~BlockYUVwriter() {
        _uninit();
    }

    bool BlockYUVwriter::_init() {
        _uninit(); //just in case an other file was opened before
        file = new ofstream(_filename, ios::out | ios::binary);
        return true;
    }

    void BlockYUVwriter::_uninit() {
        if (file != NULL) {
            if (file->is_open()) {
                file->close();
            }
            free(file);
            file = NULL;
        }
    }

    bool BlockYUVwriter::run(bool oneShot){
        if (oneShot) {
            return true;
        }
        
        if (file == NULL || _myInputs["BLOCK__WRITE_IN_FILENAME"].isNew()) {
            _filename = _myInputs["BLOCK__WRITE_IN_FILENAME"].get<string>(true);
            if (!_init()) {
                return false;
            }
        }
        if (!file->is_open()) {
            return false;
        }
        cv::Mat out = _myInputs["BLOCK__WRITE_IN_IMAGE"].get<cv::Mat>(true);
        cv::Mat cv_yuv[3], cv_u_half, cv_v_half;
        cv::cvtColor(out, out, cv::COLOR_RGB2YUV);
        cv::split(out, cv_yuv);

        file->write((char*)cv_yuv[0].data, cv_yuv[0].size().width * cv_yuv[0].size().height);

        cv::resize(cv_yuv[1], cv_u_half, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
        cv::resize(cv_yuv[2], cv_v_half, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);

        file->write((char*)cv_u_half.data, cv_u_half.size().width * cv_u_half.size().height);
        file->write((char*)cv_v_half.data, cv_v_half.size().width * cv_v_half.size().height);

        return true;
    };

};