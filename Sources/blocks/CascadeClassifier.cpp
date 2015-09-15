#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)
#endif
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"

using std::vector;
using cv::Mat;
using std::string;
using namespace std;

namespace charliesoft
{
    BLOCK_BEGIN_INSTANTIATION(CascadeClassifier);
protected:
    cv::CascadeClassifier face_cascade;

    BLOCK_END_INSTANTIATION(CascadeClassifier, AlgoType::imgProcess, BLOCK__CASCADECLASSIFIER_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(CascadeClassifier);
    //Add parameters, with following parameters:
    ADD_PARAMETER(true, Matrix, "BLOCK__CASCADECLASSIFIER_IN_IMAGE", "BLOCK__CASCADECLASSIFIER_IN_IMAGE_HELP");
    ADD_PARAMETER(userConstant, FilePath, "BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE", "BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE_HELP");
    ADD_PARAMETER_FULL(userConstant, Float, "BLOCK__CASCADECLASSIFIER_IN_SCALE_FACTOR", "BLOCK__CASCADECLASSIFIER_IN_SCALE_FACTOR_HELP", 1.1);
    ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__CASCADECLASSIFIER_IN_MIN_NEIGHBORS", "BLOCK__CASCADECLASSIFIER_IN_MIN_NEIGHBORS_HELP", 3);
    ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__CASCADECLASSIFIER_IN_MIN_WIDTH", "BLOCK__CASCADECLASSIFIER_IN_MIN_WIDTH_HELP", 60);
    ADD_PARAMETER_FULL(userConstant, Int, "BLOCK__CASCADECLASSIFIER_IN_MIN_HEIGHT", "BLOCK__CASCADECLASSIFIER_IN_MIN_HEIGHT_HELP", 60);
    ADD_PARAMETER(notUsed, Int, "BLOCK__CASCADECLASSIFIER_IN_MAX_WIDTH", "BLOCK__CASCADECLASSIFIER_IN_MAX_WIDTH_HELP");
    ADD_PARAMETER(notUsed, Int, "BLOCK__CASCADECLASSIFIER_IN_MAX_HEIGHT", "BLOCK__CASCADECLASSIFIER_IN_MAX_HEIGHT_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(CascadeClassifier);
    ADD_PARAMETER(true, Matrix, "BLOCK__CASCADECLASSIFIER_OUT_IMAGE", "BLOCK__CASCADECLASSIFIER_OUT_IMAGE_HELP");
    ADD_PARAMETER(notUsed, Int, "BLOCK__CASCADECLASSIFIER_OUT_OBJECTS", "BLOCK__CASCADECLASSIFIER_OUT_OBJECTS_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(CascadeClassifier);
    END_BLOCK_PARAMS();

    CascadeClassifier::CascadeClassifier() :Block("BLOCK__CASCADECLASSIFIER_NAME", true){
        _myInputs["BLOCK__CASCADECLASSIFIER_IN_IMAGE"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE"].addValidator({ new ValNeeded(), new ValFileExist(), new ValFileTypes("XML (*.xml)")});
    };

    bool CascadeClassifier::run(bool oneShot){
        if (_myInputs["BLOCK__CASCADECLASSIFIER_IN_IMAGE"].isDefaultValue())
            return false;

        if (_myInputs["BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE"].isNew()) {
            if (boost::filesystem::exists(_myInputs["BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE"].get<string>())) {
                face_cascade.load(_myInputs["BLOCK__CASCADECLASSIFIER_IN_CASCADE_FILE"].get<string>());
            }
        }

        if (face_cascade.empty())
            return false;

        cv::Size minSize,maxSize;
        if (_myInputs["BLOCK__CASCADECLASSIFIER_IN_MIN_WIDTH"].get<int>() > 0 || _myInputs["BLOCK__CASCADECLASSIFIER_IN_MIN_HEIGHT"].get<int>() > 0) {
                minSize = cv::Size(_myInputs["BLOCK__CASCADECLASSIFIER_IN_MIN_WIDTH"].get<int>(), _myInputs["BLOCK__CASCADECLASSIFIER_IN_MIN_HEIGHT"].get<int>());
        }

        if (_myInputs["BLOCK__CASCADECLASSIFIER_IN_MAX_WIDTH"].get<int>() > 0 || _myInputs["BLOCK__CASCADECLASSIFIER_IN_MAX_HEIGHT"].get<int>() > 0) {
                maxSize = cv::Size(_myInputs["BLOCK__CASCADECLASSIFIER_IN_MAX_WIDTH"].get<int>(), _myInputs["BLOCK__CASCADECLASSIFIER_IN_MAX_HEIGHT"].get<int>());
        }

        cv::Mat output = _myInputs["BLOCK__CASCADECLASSIFIER_IN_IMAGE"].get<cv::Mat>().clone();

        //convert captured image to gray scale and equalize
        cv::cvtColor(output, output, CV_BGR2GRAY);
        cv::equalizeHist(output, output);

        //create a vector array to store the face found
        std::vector<cv::Rect> faces;
        //find faces and store them in the vector array
        face_cascade.detectMultiScale(output, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, minSize, maxSize);
        cv::Mat mask = cv::Mat::zeros(output.size(), CV_8U);

        //draw a rectangle for all found faces in the vector array on the original image
        for (int i = 0; i < faces.size(); i++)
        {
            mask(faces[i]) = 1;
        }

        if (!output.empty())
        {
            _myOutputs["BLOCK__CASCADECLASSIFIER_OUT_IMAGE"] = mask;
            _myOutputs["BLOCK__CASCADECLASSIFIER_OUT_OBJECTS"] = (int)(faces.size());
        }
        return !output.empty();
    };
};