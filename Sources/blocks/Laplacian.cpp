
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
	BLOCK_BEGIN_INSTANTIATION(Laplacian);
	BLOCK_END_INSTANTIATION(Laplacian, AlgoType::imgProcess, BLOCK__LAPLACIAN_NAME);

	BEGIN_BLOCK_INPUT_PARAMS(Laplacian);
	//Add parameters, with following parameters:
	ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LAPLACIAN_IN_IMAGE", "BLOCK__LAPLACIAN_IN_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_OUTPUT_PARAMS(Laplacian);
	ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__LAPLACIAN_OUT_IMAGE", "BLOCK__LAPLACIAN_OUT_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_SUBPARAMS_DEF(Laplacian);
	END_BLOCK_PARAMS();

	Laplacian::Laplacian() :Block("BLOCK__LAPLACIAN_NAME", true){
		_myInputs["BLOCK__LAPLACIAN_IN_IMAGE"].addValidator({ new ValNeeded() });
	};

	bool Laplacian::run(bool oneShot){
		if (_myInputs["BLOCK__LAPLACIAN_IN_IMAGE"].isDefaultValue())
			return false;
		cv::Mat mat = _myInputs["BLOCK__LAPLACIAN_IN_IMAGE"].get<cv::Mat>();
		if (!mat.empty())
		{
			cv::Mat output;
			cv::Laplacian(mat, output, -1);
			_myOutputs["BLOCK__LAPLACIAN_OUT_IMAGE"] = output;
		}
		return !mat.empty();
	};
};