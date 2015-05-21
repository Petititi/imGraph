
#ifdef _WIN32
#ifdef IMGRAPH_PLUGIN_VCAM

#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)

#include <vector>
#include <memory>
#include <functional>
#include "opencv2/imgproc/imgproc.hpp"
#pragma warning(pop)
//#pragma comment(lib, "VCam.lib")
#include <windows.h>
#include <dshow.h>

#pragma comment(lib, "strmiids")
#include "Block.h"
#include "ParamValidator.h"

//#include "Filters.h"

using std::vector;
using std::string;
using cv::Mat;

DEFINE_GUID(CLSID_VirtualCam,
    0x8e14549a, 0xdb61, 0x4309, 0xaf, 0xa1, 0x35, 0x78, 0xe9, 0x27, 0xe9, 0x33);

namespace charliesoft
{
	BLOCK_BEGIN_INSTANTIATION(VCam);
protected:
    //ICVCam* ptrCam;

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
        IBaseFilter *pFilter = NULL;
        HRESULT hr = CoCreateInstance(CLSID_VirtualCam, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFilter));

	};

	bool VCam::run(bool oneShot){
		if (_myInputs["BLOCK__VCAM_IN_IMAGE"].isDefaultValue())
			return false;
		cv::Mat mat = _myInputs["BLOCK__VCAM_IN_IMAGE"].get<cv::Mat>();
		//if (!mat.empty()) {
		//	LONG width, height;
		//	cam->GetSize(&width, &height);
		//	_myOutputs["BLOCK__VCAM_OUT_WIDTH"] = (int)width;
		//	_myOutputs["BLOCK__VCAM_OUT_HEIGHT"] = (int)height;
		//}
		return mat.empty();
	};
};

#endif //IMGRAPH_PLUGIN_VCAM
#endif //_WIN32