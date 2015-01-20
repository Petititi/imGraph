#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <opencv2/imgproc.hpp>
#include <vector>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
#include "MatrixConvertor.h"
using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(MergingBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(MergingBlock, AlgoType::mathOperator, BLOCK__MERGING_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(MergingBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, AnyType, "BLOCK__MERGING_IN_IMAGE1", "BLOCK__MERGING_IN_IMAGE1_HELP");
  ADD_PARAMETER(true, AnyType, "BLOCK__MERGING_IN_IMAGE2", "BLOCK__MERGING_IN_IMAGE2_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(MergingBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__MERGING_OUT_IMAGE", "BLOCK__MERGING_OUT_IMAGE");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(MergingBlock);
  END_BLOCK_PARAMS();

  MergingBlock::MergingBlock() :Block("BLOCK__MERGING_NAME"){
    _myInputs["BLOCK__MERGING_IN_IMAGE1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__MERGING_IN_IMAGE2"].addValidator({ new ValNeeded() });
  };

  bool MergingBlock::run(bool oneShot){
    if (_myInputs["BLOCK__MERGING_IN_IMAGE1"].isDefaultValue())
      return false;
    if (_myInputs["BLOCK__MERGING_IN_IMAGE2"].isDefaultValue())
      return false;
    ParamType type1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].getType(false),
      type2 = _myInputs["BLOCK__MERGING_IN_IMAGE2"].getType(false);
    //Boolean = 0, Int, Float, Color, Matrix, String, FilePath, ListBox, AnyType, typeError

    switch (type1)
    {
    case Int:
    {
      int param1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].get<int>(true);
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<int>(true);
        break;
      case Float:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<double>(true);
        break;
      case Color:
      {
        cv::Scalar tmp(param1, param1, param1, param1);
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = tmp + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Scalar>(true);
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Mat>(true);
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Float:
    {
      double param1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].get<double>(true);
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<int>(true);
        break;
      case Float:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<double>(true);
        break;
      case Color:
      {
        cv::Scalar tmp(param1, param1, param1, param1);
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = tmp + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Scalar>(true);
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Mat>(true);
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Color:
    {
      cv::Scalar param1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].get<cv::Scalar>(true);
      switch (type2)
      {
      case Int:
      {
        int param2 = _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<int>(true);
        cv::Scalar tmp(param2, param2, param2, param2);
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + tmp;
        break;
      }
      case Float:
      {
        double param2 = _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<double>(true);
        cv::Scalar tmp(param2, param2, param2, param2);
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + tmp;
        break;
      }
      case Color:
      {
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Scalar>(true);
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Mat>(true);
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Matrix:
    {
      cv::Mat param1 = _myInputs["BLOCK__MERGING_IN_IMAGE1"].get<cv::Mat>(true);
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<int>(true);
        break;
      case Float:
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<double>(true);
        break;
      case Color:
      {
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = param1 + _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Scalar>(true);
        break;
      }
      case Matrix:
      {
        cv::Mat param2 = _myInputs["BLOCK__MERGING_IN_IMAGE2"].get<cv::Mat>(true);
        int outputType = param1.type();
        if (param1.channels() != param2.channels())
        {
          if (param1.channels() < param2.channels())
          {
            param1 = MatrixConvertor::adjustChannels(param1, param2.channels());
            outputType = param2.type();
          }
          else
            param2 = MatrixConvertor::adjustChannels(param2, param1.channels());
        }
        Mat dest;
        cv::add(param1, param2, dest, cv::Mat(), outputType);
        _myOutputs["BLOCK__MERGING_OUT_IMAGE"] = dest;
      }
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    default:
      return false;//nothing to do as we don't support this type of operation
      break;
    }

    return true;
  };
};