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

	unsigned char *_y;           /**< Used internally. >*/
	unsigned char *_u;           /**< Used internally. >*/
	unsigned char *_v;           /**< Used internally. >*/
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
		_y = NULL;
		_u = NULL;
		_v = NULL;
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
		_nr_frames = 0;
        //regex, file width x height if not specified
		_y = (unsigned char *)calloc(_width * _height, sizeof(unsigned char));
		_u = (unsigned char *)calloc(_width * _height / 4, sizeof(unsigned char));
		_v = (unsigned char *)calloc(_width * _height / 4, sizeof(unsigned char));

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
		if (_y != NULL) {
			free(_y);
			_y = NULL;
		}
		if (_u != NULL) {
			free(_u);
			_u = NULL;
		}
		if (_v != NULL) {
			free(_v);
			_v = NULL;
		}
    }

    bool BlockYUVreader::run(bool oneShot){
        
        if (file == NULL) {
            if (!init(_myInputs["BLOCK__INPUT_IN_INPUT_FILE"].get<string>(true))) {
                return false;
            }
        }

		if (file->eof()){
			file->seekg(0);
		}

		file->read((char*)_y, _width * _height);
		file->read((char*)_u, _width * _height / 4);
		file->read((char*)_v, _width * _height / 4);

		cv::Mat cv_y = cv::Mat(_height, _width, CV_8UC1, _y);
		cv::Mat cv_u = cv::Mat(_height / 2, _width / 2, CV_8UC1, _u);
		cv::Mat cv_v = cv::Mat(_height / 2, _width / 2, CV_8UC1, _v);

		if (oneShot) {
			if (file->is_open()) {
				file->seekg(0); // go to beginning of the file
			}
		}
		else {
			_nr_frames++;
		}
        return true;
    };

};