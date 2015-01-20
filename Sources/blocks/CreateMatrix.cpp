
#include <vector>

#include "Block.h"
#include "view/MatrixViewer.h"
#include "MatrixConvertor.h"
#include "ParamValidator.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include "opencv2/core/utility.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;
using cv::RNG;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(CreateMatrix);
  //You can add methods, re implement needed functions...
public:
  BLOCK_END_INSTANTIATION(CreateMatrix, AlgoType::input, BLOCK__CREATEMATRIX_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(CreateMatrix);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER_FULL(true, Int, "BLOCK__CREATEMATRIX_IN_WIDTH", "BLOCK__INPUT_INOUT_WIDTH_HELP", 640);
  ADD_PARAMETER_FULL(true, Int, "BLOCK__CREATEMATRIX_IN_HEIGHT", "BLOCK__INPUT_INOUT_HEIGHT_HELP", 480);
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__CREATEMATRIX_IN_TYPE", "BLOCK__CREATEMATRIX_IN_TYPE_HELP", 0);
  ADD_PARAMETER_FULL(false, Int, "BLOCK__CREATEMATRIX_IN_NBCHANNEL", "BLOCK__CREATEMATRIX_IN_NBCHANNEL_HELP", 1);
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__CREATEMATRIX_IN_INIT", "BLOCK__CREATEMATRIX_IN_INIT_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(CreateMatrix);
  ADD_PARAMETER(true, Matrix, "BLOCK__CREATEMATRIX_OUT_IMAGE", "BLOCK__CREATEMATRIX_OUT_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(CreateMatrix);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CREATEMATRIX_IN_INIT.random uniform.minValue", "minValue", 0.);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CREATEMATRIX_IN_INIT.random uniform.maxValue", "maxValue", 255.);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CREATEMATRIX_IN_INIT.random gaussian.mean", "mean", 128.);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CREATEMATRIX_IN_INIT.random gaussian.std dev", "std dev", 128.);
  END_BLOCK_PARAMS();

  CreateMatrix::CreateMatrix() :Block("BLOCK__CREATEMATRIX_NAME"){
    _myInputs["BLOCK__CREATEMATRIX_IN_TYPE"].addValidator({ new ValNeeded(), new ValRange(0, 6) });
    _myInputs["BLOCK__CREATEMATRIX_IN_WIDTH"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_HEIGHT"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_NBCHANNEL"].addValidator({ new ValNeeded(), new ValPositiv(true) });
    _myInputs["BLOCK__CREATEMATRIX_IN_INIT"].addValidator({ new ValNeeded(), new ValRange(0, 8) });
  };
  
  bool CreateMatrix::run(bool oneShot){
    //todo: verify that type index correspond to constant!
    int nbChannels = _myInputs["BLOCK__CREATEMATRIX_IN_NBCHANNEL"].get<int>(true);
    int wantedType = CV_MAKETYPE(_myInputs["BLOCK__CREATEMATRIX_IN_TYPE"].get<int>(true), nbChannels);
    int wantedRow = _myInputs["BLOCK__CREATEMATRIX_IN_HEIGHT"].get<int>(true);
    int wantedCol = _myInputs["BLOCK__CREATEMATRIX_IN_WIDTH"].get<int>(true);

    static RNG rng(cv::getTickCount());

    cv::Mat newMatrix;
    switch (_myInputs["BLOCK__CREATEMATRIX_IN_INIT"].get<int>(true))
    {
    case 1:
      newMatrix = cv::Mat::ones(wantedRow, wantedCol, wantedType) * 128;
      break;
    case 2:
      newMatrix = cv::Mat::eye(wantedRow, wantedCol, wantedType);
      break;
    case 3:
      newMatrix = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(wantedCol, wantedRow));
      newMatrix = charliesoft::MatrixConvertor::convert(newMatrix, wantedType);
      break;
    case 4:
      newMatrix = getStructuringElement(cv::MORPH_RECT, cv::Size(wantedCol, wantedRow));
      newMatrix = charliesoft::MatrixConvertor::convert(newMatrix, wantedType);
      break;
    case 5:
      newMatrix = getStructuringElement(cv::MORPH_CROSS, cv::Size(wantedCol, wantedRow));
      newMatrix = charliesoft::MatrixConvertor::convert(newMatrix, wantedType);
      break;
    case 6:
    {
      newMatrix = cv::Mat(wantedRow * wantedCol, 1, wantedType);
      double a = _mySubParams["BLOCK__CREATEMATRIX_IN_INIT.random uniform.minValue"].get<double>(true);
      double b = _mySubParams["BLOCK__CREATEMATRIX_IN_INIT.random uniform.maxValue"].get<double>(true);
      rng.fill(newMatrix, RNG::UNIFORM, a, b);
      newMatrix = newMatrix.reshape(nbChannels, wantedRow);
      break;
    }
    case 7:
    {
      newMatrix = cv::Mat(wantedRow * wantedCol, 1, wantedType);
      double a = _mySubParams["BLOCK__CREATEMATRIX_IN_INIT.random gaussian.mean"].get<double>(true);
      double b = _mySubParams["BLOCK__CREATEMATRIX_IN_INIT.random gaussian.std dev"].get<double>(true);
      rng.fill(newMatrix, RNG::NORMAL, a, b);
      newMatrix = newMatrix.reshape(nbChannels, wantedRow);
      break;
    }
    default:
      newMatrix = cv::Mat::zeros(wantedRow, wantedCol, wantedType);
      break;
    }
    _myOutputs["BLOCK__CREATEMATRIX_OUT_IMAGE"] = newMatrix;
    return true;
  };
};