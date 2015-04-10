
#include <vector>

#include "Block.h"
#include "ParamValidator.h"
#include "opencv2/imgproc/imgproc.hpp"

using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
    BLOCK_BEGIN_INSTANTIATION(BlockCanny);
    BLOCK_END_INSTANTIATION(BlockCanny, AlgoType::imgProcess, BLOCK__CANNY_NAME);

    BEGIN_BLOCK_INPUT_PARAMS(BlockCanny);
    //Add parameters, with following parameters:
    //Based on opencv::Canny( InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize = 3, bool L2gradient = false );
    ADD_PARAMETER(true, Matrix, "BLOCK__CANNY_IN_IMAGE", "BLOCK__CANNY_IN_IMAGE_HELP");
    ADD_PARAMETER_FULL(false, Float, "BLOCK__CANNY_IN_THRESHOLD_1", "BLOCK__CANNY_IN_THRESHOLD_1_HELP",100.f);
    ADD_PARAMETER_FULL(false, Float, "BLOCK__CANNY_IN_THRESHOLD_2", "BLOCK__CANNY_IN_THRESHOLD_2_HELP",300.f);
    ADD_PARAMETER_FULL(false, Int, "BLOCK__CANNY_IN_APERTURE_SIZE", "BLOCK__CANNY_IN_APERTURE_SIZE_HELP",3);
    ADD_PARAMETER_FULL(false, Boolean, "BLOCK__CANNY_IN_L2_GRADIENT", "BLOCK__CANNY_IN_L2_GRADIENT_HELP",false);
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_OUTPUT_PARAMS(BlockCanny);
    ADD_PARAMETER(true, Matrix, "BLOCK__CANNY_OUT_IMAGE", "BLOCK__CANNY_OUT_IMAGE_HELP");
    END_BLOCK_PARAMS();

    BEGIN_BLOCK_SUBPARAMS_DEF(BlockCanny);
    END_BLOCK_PARAMS();

    BlockCanny::BlockCanny() :Block("BLOCK__CANNY_NAME"){
        _myInputs["BLOCK__CANNY_IN_IMAGE"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__CANNY_IN_THRESHOLD_1"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__CANNY_IN_THRESHOLD_2"].addValidator({ new ValNeeded() });
        _myInputs["BLOCK__CANNY_IN_APERTURE_SIZE"].addValidator({ new ValNeeded(), new ValPositiv(true) });
        _myInputs["BLOCK__CANNY_IN_L2_GRADIENT"].addValidator({ new ValNeeded() });
    };

    bool BlockCanny::run(bool oneShot){
        if (_myInputs["BLOCK__CANNY_IN_IMAGE"].isDefaultValue())
            return false;
        cv::Mat mat = _myInputs["BLOCK__CANNY_IN_IMAGE"].get<cv::Mat>(true);
        cv::Mat output;
        if (!mat.empty())
        {
            Canny(mat, 
                  output,
                  _myInputs["BLOCK__CANNY_IN_THRESHOLD_1"].get<double>(true),
                  _myInputs["BLOCK__CANNY_IN_THRESHOLD_2"].get<double>(true),
                  _myInputs["BLOCK__CANNY_IN_APERTURE_SIZE"].get<int>(true),
                  _myInputs["BLOCK__CANNY_IN_L2_GRADIENT"].get<bool>(true) );
            _myOutputs["BLOCK__CANNY_OUT_IMAGE"] = output;
        }
        return true;
    };
};