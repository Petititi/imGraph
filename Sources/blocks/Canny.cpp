
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
	//default visibility, type of parameter, name (key of internationalizor), helper...
	ADD_PARAMETER(true, Matrix, "BLOCK__CANNY_IN_IMAGE", "BLOCK__CANNY_IN_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_OUTPUT_PARAMS(BlockCanny);
	ADD_PARAMETER(true, Matrix, "BLOCK__CANNY_OUT_IMAGE", "BLOCK__CANNY_OUT_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_SUBPARAMS_DEF(BlockCanny);
	END_BLOCK_PARAMS();

	BlockCanny::BlockCanny() :Block("BLOCK__CANNY_NAME"){
		_myInputs["BLOCK__CANNY_IN_IMAGE"].addValidator({ new ValNeeded() });
	};

	bool BlockCanny::run(bool oneShot){
		if (_myInputs["BLOCK__CANNY_IN_IMAGE"].isDefaultValue())
			return false;
		cv::Mat mat = _myInputs["BLOCK__CANNY_IN_IMAGE"].get<cv::Mat>(true);
		if (!mat.empty())
		{
			Canny(mat, mat, 100, 300);
			_myOutputs["BLOCK__CANNY_OUT_IMAGE"] = mat;
		}
		return true;
	};
};