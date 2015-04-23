
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)
#endif
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"

using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
	BLOCK_BEGIN_INSTANTIATION(Filter2D);
	BLOCK_END_INSTANTIATION(Filter2D, AlgoType::imgProcess, BLOCK__FILTER2D_NAME);

	BEGIN_BLOCK_INPUT_PARAMS(Filter2D);
	//Add parameters, with following parameters:
	ADD_PARAMETER(true, Matrix, "BLOCK__FILTER2D_IN_IMAGE", "BLOCK__FILTER2D_IN_IMAGE_HELP");
	ADD_PARAMETER(true, Matrix, "BLOCK__FILTER2D_IN_KERNEL", "BLOCK__FILTER2D_IN_KERNEL_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_OUTPUT_PARAMS(Filter2D);
	ADD_PARAMETER(true, Matrix, "BLOCK__FILTER2D_OUT_IMAGE", "BLOCK__FILTER2D_OUT_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_SUBPARAMS_DEF(Filter2D);
	END_BLOCK_PARAMS();

	Filter2D::Filter2D() :Block("BLOCK__FILTER2D_NAME", true){
		_myInputs["BLOCK__FILTER2D_IN_IMAGE"].addValidator({ new ValNeeded() });
		_myInputs["BLOCK__FILTER2D_IN_KERNEL"].addValidator({ new ValNeeded() });
	};

	bool Filter2D::run(bool oneShot){
		if (_myInputs["BLOCK__FILTER2D_IN_IMAGE"].isDefaultValue())
			return false;
		cv::Mat mat = _myInputs["BLOCK__FILTER2D_IN_IMAGE"].get<cv::Mat>();
		cv::Mat kernel = _myInputs["BLOCK__FILTER2D_IN_KERNEL"].get<cv::Mat>();
		cv::Mat output;
		if (!mat.empty())
		{
			cv::filter2D(mat, output, -1, kernel);
			_myOutputs["BLOCK__FILTER2D_OUT_IMAGE"] = output;
		}
		return !mat.empty();
	};
};