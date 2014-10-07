
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "ParamValidator.h"
using namespace lsis_org;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(CreateMatrix);
  //You can add methods, re implement needed functions...
  BLOCK_END_INSTANTIATION(CreateMatrix, AlgoType::input, BLOCK__CREATEMATRIX_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(CreateMatrix);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Int, "BLOCK__CREATEMATRIX_IN_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP");
  ADD_PARAMETER(true, Int, "BLOCK__CREATEMATRIX_IN_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CREATEMATRIX_IN_TYPE", "BLOCK__CREATEMATRIX_IN_TYPE_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CREATEMATRIX_IN_NBCHANNEL", "BLOCK__CREATEMATRIX_IN_NBCHANNEL_HELP");
  ADD_PARAMETER(false, Int, "BLOCK__CREATEMATRIX_IN_INIT", "BLOCK__CREATEMATRIX_IN_INIT_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(CreateMatrix);
  ADD_PARAMETER(true, Matrix, "BLOCK__CREATEMATRIX_OUT_IMAGE", "BLOCK__CREATEMATRIX_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  CreateMatrix::CreateMatrix() :Block("BLOCK__CREATEMATRIX_NAME"){
    _myInputs["BLOCK__CREATEMATRIX_IN_TYPE"].addValidator({ new ValNeeded(), new ValRange(0, 6) });
    _myInputs["BLOCK__CREATEMATRIX_IN_WIDTH"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_HEIGHT"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_NBCHANNEL"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_INIT"].addValidator({ new ValNeeded(), new ValRange(0, 6) });
  };
  
  bool CreateMatrix::run(){
    //todo: verify that type index correspond to constant!
    int wantedType = CV_MAKETYPE(_myInputs["BLOCK__CREATEMATRIX_IN_TYPE"].get<int>(),
      _myInputs["BLOCK__CREATEMATRIX_IN_NBCHANNEL"].get<int>());
    int wantedRow = _myInputs["BLOCK__CREATEMATRIX_IN_HEIGHT"].get<int>();
    int wantedCol = _myInputs["BLOCK__CREATEMATRIX_IN_WIDTH"].get<int>();

    cv::Mat newMatrix;
    switch (_myInputs["BLOCK__CREATEMATRIX_IN_INIT"].get<int>())
    {
    case 1:
      newMatrix = cv::Mat::ones(wantedRow, wantedCol, wantedType) * 128;
      break;
    case 2:
      newMatrix = cv::Mat::eye(wantedRow, wantedCol, wantedType);
      break;
    case 3:
      newMatrix = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(wantedRow, wantedCol));
      break;
    case 4:
      newMatrix = getStructuringElement(cv::MORPH_RECT, cv::Size(wantedRow, wantedCol));
      break;
    case 5:
      newMatrix = getStructuringElement(cv::MORPH_CROSS, cv::Size(wantedRow, wantedCol));
      break;
    case 6:
      newMatrix = cv::Mat(wantedRow, wantedCol, wantedType);
      //todo : randomize!
      break;
    default:
      newMatrix = cv::Mat::zeros(wantedRow, wantedCol, wantedType);
      break;
    }
    _myOutputs["BLOCK__CREATEMATRIX_OUT_IMAGE"] = newMatrix;
    renderingDone();
    return true;
  };
};