
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
#include "Convertor.h"
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
    ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__MASKDRAWER_IN_MASK", "BLOCK__MASKDRAWER_IN_MASK_HELP");
    ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__MASKDRAWER_IN_IMAGE", "BLOCK__MASKDRAWER_IN_IMAGE_HELP");
    ADD_PARAMETER_FULL(userConstant, ListBox, "BLOCK__MASKDRAWER_IN_PRINTMASK", "BLOCK__MASKDRAWER_IN_PRINTMASK_HELP", 0);
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(MaskDrawer);
    ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__MASKDRAWER_OUT_IMAGE", "BLOCK__MASKDRAWER_OUT_IMAGE_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(MaskDrawer);
    ADD_PARAMETER_FULL(notUsed, Color, "BLOCK__MASKDRAWER_IN_PRINTMASK.Draw mask.color", "color", (int)0xFFFFFFFF);
    END_BLOCK_PARAMS();

    MaskDrawer::MaskDrawer() :Block("BLOCK__MASKDRAWER_NAME", true){
        _myInputs["BLOCK__MASKDRAWER_IN_MASK"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].addValidator({ new ValNeeded() });
    };

    bool MaskDrawer::run(bool oneShot){
        cv::Mat out = _myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].get<cv::Mat>().clone();
        cv::Scalar color = cv::Scalar(255, 255, 255);


        cv::Mat mask = _myInputs["BLOCK__MASKDRAWER_IN_MASK"].get<cv::Mat>();
        if (mask.type() != CV_8UC1) {
            throw "Expected mask of type CV_8UC1";
        }

        if (!_myInputs["BLOCK__MASKDRAWER_IN_MASK"].isDefaultValue() && !_myInputs["BLOCK__MASKDRAWER_IN_IMAGE"].isDefaultValue()){
          if (mask.size() != out.size()) {
            throw "Image an mask have different sizes";
          }

          if (_myInputs["BLOCK__MASKDRAWER_IN_PRINTMASK"].get<int>() == 0)
          {
            //normalize output if it's float values:
            if (out.depth() != CV_8U)
            {
              cv::Mat tmp;
              cv::normalize(out, tmp, 0, 255, cv::NORM_MINMAX, CV_8UC3);
              out = tmp;
            }
            out = MatrixConvertor::adjustChannels(out, 3);

            if (!_mySubParams["BLOCK__MASKDRAWER_IN_PRINTMASK.Draw mask.color"].isDefaultValue())
              color = _mySubParams["BLOCK__MASKDRAWER_IN_PRINTMASK.Draw mask.color"].get<cv::Scalar>();
            out.setTo(color, mask);
          }
          else
          {
            cv::Mat tmp;
            out.copyTo(tmp, mask);
            out = tmp;
          }

          _myOutputs["BLOCK__MASKDRAWER_OUT_IMAGE"] = out;
        }

        return true;
    };
};