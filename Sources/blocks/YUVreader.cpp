/*

Usefull to read files in raw video format

*/
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "opencv2/opencv.hpp"
#include <fstream>

using std::string;
using std::vector;
using namespace std;

namespace charliesoft
{
    BLOCK_BEGIN_INSTANTIATION(BlockYUVreader);
    //You can add methods, attributs, reimplement needed functions...
protected:
    ifstream * file;

    cv::Mat y;            /**< Used internally. >*/
    cv::Mat cb;           /**< Used internally. >*/
    cv::Mat cr;           /**< Used internally. >*/
    cv::Mat cb_half;      /**< Used internally. >*/
    cv::Mat cr_half;      /**< Used internally. >*/
    cv::Mat ycrcb;        /**< The most-recently image (width x height, 24
                            bit).  Stored in YCrCb order. >*/
    int _width;
    int _height;
    int _nr_frames;

public:
    ~BlockYUVreader();
    bool init(string filepath);
    void uninit();
    BLOCK_END_INSTANTIATION(BlockYUVreader, AlgoType::input, BLOCK__INPUT_RAW_VIDEO_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(BlockYUVreader);
    //Add parameters, with following parameters:
    //default visibility, type of parameter, name (key of internationalizor), helper...
    ADD_PARAMETER(false, FilePath, "BLOCK__INPUT_IN_INPUT_FILE", "BLOCK__INPUT_IN_INPUT_FILE_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
    ADD_PARAMETER(false, Int, "BLOCK__INPUT_INOUT_POS_FRAMES", "BLOCK__INPUT_INOUT_POS_FRAMES_HELP");
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
    };

    BlockYUVreader::~BlockYUVreader() {
        this->uninit();
    }

    bool BlockYUVreader::init(string filepath) {
        this->uninit(); //just in case an other file was opened before
        if (! boost::filesystem::is_regular_file(filepath)) {
            return false;
        }
        file->open(filepath.c_str(), ios::in | ios::binary | ios::ate);
        if (!file->is_open()) {
            return false;
        }
        //regex, file width x height if not specified
        return true;
    }

    void BlockYUVreader::uninit() {
        if (file != NULL) {
            if (file->is_open()) {
                file->close();
            }
            free(file);
            file = NULL;
        }
    }

    bool BlockYUVreader::run(bool oneShot){
        
        if (file == NULL) {
            if (!init(_myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>(true))) {
                return false;
            }
        }

        if (oneShot && file->is_open()) {
            file->seekg(0); // got to beginning of the file
        }
        return true;
    };

};