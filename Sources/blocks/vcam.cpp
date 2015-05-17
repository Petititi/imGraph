
#ifdef _WIN32
#ifdef IMGRAPH_PLUGIN_VCAM

#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)

#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#pragma warning(pop)
#pragma comment(lib, "VCam.lib")

#include "Block.h"
#include "ParamValidator.h"
#include "Filters.h"
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
	BLOCK_BEGIN_INSTANTIATION(VCam);
protected:
	CVCam * cam;

	BLOCK_END_INSTANTIATION(VCam, AlgoType::output, BLOCK__VCAM_NAME);

	BEGIN_BLOCK_INPUT_PARAMS(VCam);
	//Add parameters, with following parameters:
	ADD_PARAMETER(true, Matrix, "BLOCK__VCAM_IN_IMAGE", "BLOCK__VCAM_IN_IMAGE_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_OUTPUT_PARAMS(VCam);
	ADD_PARAMETER(true, Int, "BLOCK__VCAM_OUT_WIDTH", "BLOCK__VCAM_OUT_WIDTH_HELP");
	ADD_PARAMETER(true, Int, "BLOCK__VCAM_OUT_HEIGHT", "BLOCK__VCAM_OUT_HEIGHT_HELP");
	END_BLOCK_PARAMS();

	BEGIN_BLOCK_SUBPARAMS_DEF(VCam);
	END_BLOCK_PARAMS();

	VCam::VCam() :Block("BLOCK__VCAM_NAME", true){
		_myInputs["BLOCK__VCAM_IN_IMAGE"].addValidator({ new ValNeeded() });
		HRESULT phr;
		cam = (CVCam*) CVCam::CreateInstance(NULL, &phr);
	};

	bool VCam::run(bool oneShot){
		if (_myInputs["BLOCK__VCAM_IN_IMAGE"].isDefaultValue())
			return false;
		cv::Mat mat = _myInputs["BLOCK__VCAM_IN_IMAGE"].get<cv::Mat>();
		if (!mat.empty()) {
			LONG width, height;
			cam->GetSize(&width, &height);
			_myOutputs["BLOCK__VCAM_OUT_WIDTH"] = (int)width;
			_myOutputs["BLOCK__VCAM_OUT_HEIGHT"] = (int)height;
		}
		return mat.empty();
	};
};

#endif //IMGRAPH_PLUGIN_VCAM
#endif //_WIN32