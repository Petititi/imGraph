#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)
#pragma warning(pop)
#endif

#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include "Block.h"
#include "ParamValidator.h"

using std::vector;
using cv::Mat;
using std::string;
using namespace std;

namespace charliesoft
{
    BLOCK_BEGIN_INSTANTIATION(FaceDetection);
protected:
    cv::CascadeClassifier face_cascade;

public:

    BLOCK_END_INSTANTIATION(FaceDetection, AlgoType::imgProcess, BLOCK__FACEDETECTION_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(FaceDetection);
    //Add parameters, with following parameters:
    ADD_PARAMETER(true, Matrix, "BLOCK__FACEDETECTION_IN_IMAGE", "BLOCK__FACEDETECTION_IN_IMAGE_HELP");
    ADD_PARAMETER(false, FilePath, "BLOCK__FACEDETECTION_IN_CASCADE_FILE", "BLOCK__FACEDETECTION_IN_CASCADE_FILE_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(FaceDetection);
    ADD_PARAMETER(true, Matrix, "BLOCK__FACEDETECTION_OUT_IMAGE", "BLOCK__FACEDETECTION_OUT_IMAGE_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(FaceDetection);
    END_BLOCK_PARAMS();

    FaceDetection::FaceDetection() :Block("BLOCK__FACEDETECTION_NAME", true){
        _myInputs["BLOCK__FACEDETECTION_IN_IMAGE"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__FACEDETECTION_IN_CASCADE_FILE"].addValidator({ new ValNeeded(), new ValFileExist() });
    };

    bool FaceDetection::run(bool oneShot){
        if (_myInputs["BLOCK__FACEDETECTION_IN_IMAGE"].isDefaultValue())
            return false;

        if (_myInputs["BLOCK__FACEDETECTION_IN_CASCADE_FILE"].isNew()) {
            if (boost::filesystem::exists(_myInputs["BLOCK__FACEDETECTION_IN_CASCADE_FILE"].get<string>())) {
                face_cascade.load(_myInputs["BLOCK__FACEDETECTION_IN_CASCADE_FILE"].get<string>());
            }
        }

        if (face_cascade.empty())
            return false;

        cv::Mat output = _myInputs["BLOCK__FACEDETECTION_IN_IMAGE"].get<cv::Mat>().clone();

        //convert captured image to gray scale and equalize
        cv::cvtColor(output, output, CV_BGR2GRAY);
        cv::equalizeHist(output, output);

        //create a vector array to store the face found
        std::vector<cv::Rect> faces;
        //find faces and store them in the vector array
        face_cascade.detectMultiScale(output, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
		output = _myInputs["BLOCK__FACEDETECTION_IN_IMAGE"].get<cv::Mat>().clone();
        //draw a rectangle for all found faces in the vector array on the original image
        for (int i = 0; i < faces.size(); i++)
        {
            cv::Point pt1(faces[i].x + faces[i].width, faces[i].y + faces[i].height);
            cv::Point pt2(faces[i].x, faces[i].y);

            cv::rectangle(output, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);
        }


        if (!output.empty())
        {
            _myOutputs["BLOCK__FACEDETECTION_OUT_IMAGE"] = output;
        }
        return !output.empty();
    };
};