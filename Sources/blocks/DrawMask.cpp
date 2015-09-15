
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
    BLOCK_BEGIN_INSTANTIATION(MaskDrawer);
    //You can add methods, re implement needed functions...
    BLOCK_END_INSTANTIATION(MaskDrawer, AlgoType::input, BLOCK__MASKDRAWER_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(MaskDrawer);
    //Add parameters, with following parameters:
    //default visibility, type of parameter, name (key of internationalizor), helper...
    ADD_PARAMETER(true, Matrix, "BLOCK__MASKDRAWER_IN_MASK", "BLOCK__MASKDRAWER_IN_MASK_HELP");
    ADD_PARAMETER(true, Matrix, "BLOCK__MASKDRAWER_IN_IMAGE", "BLOCK__MASKDRAWER_IN_IMAGE_HELP");
    ADD_PARAMETER(false, Color, "BLOCK__MASKDRAWER_IN_COLOR", "BLOCK__MASKDRAWER_IN_COLOR_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(MaskDrawer);
    ADD_PARAMETER(true, Matrix, "BLOCK__MASKDRAWER_OUT_IMAGE", "BLOCK__MASKDRAWER_OUT_IMAGE_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(MaskDrawer);
    END_BLOCK_PARAMS();

    MaskDrawer::MaskDrawer() :Block("BLOCK__MASKDRAWER_NAME", true){
        _myInputs["BLOCK__MASKDRAWER_IN_MASK"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].addValidator({ new ValNeeded() });
    };

    bool MaskDrawer::run(bool oneShot){
        cv::Mat out = _myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].get<cv::Mat>().clone();
        cv::Scalar color = cv::Scalar(255, 255, 255);

        if (!_myInputs["BLOCK__MASKDRAWER_IN_COLOR"].isDefaultValue())
            color = _myInputs["BLOCK__MASKDRAWER_IN_COLOR"].get<cv::Scalar>();

        cv::Mat mask = _myInputs["BLOCK__MASKDRAWER_IN_MASK"].get<cv::Mat>();
        if (mask.type() != CV_8UC1) {
            throw "Expected mask of type CV_8UC1";
        }

        if (!_myInputs["BLOCK__MASKDRAWER_IN_MASK"].isDefaultValue() && !_myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].isDefaultValue()){
            if (mask.size() != out.size()) {
                throw "Image an mask have different sizes";
            }
            out.setTo(color, mask);
            _myOutputs["BLOCK__MASKDRAWER_OUT_IMAGE"] = out;
        }

        return true;
    };
};