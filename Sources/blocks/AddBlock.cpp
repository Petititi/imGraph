#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <opencv2/imgproc.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
#include "Convertor.h"
using namespace charliesoft;
using std::vector;
using boost::lexical_cast;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(MergingBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(MergingBlock, AlgoType::mathOperator, BLOCK__ADD_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(MergingBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, AnyType, "BLOCK__ADD_IN_PARAM1", "BLOCK__ADD_IN_PARAM1_HELP");
  ADD_PARAMETER(true, AnyType, "BLOCK__ADD_IN_PARAM2", "BLOCK__ADD_IN_PARAM2_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(MergingBlock);
  ADD_PARAMETER(true, AnyType, "BLOCK__ADD_OUTPUT", "BLOCK__ADD_OUTPUT");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(MergingBlock);
  END_BLOCK_PARAMS();

  MergingBlock::MergingBlock() :Block("BLOCK__ADD_NAME", true){
    _myInputs["BLOCK__ADD_IN_PARAM1"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__ADD_IN_PARAM2"].addValidator({ new ValNeeded() });
  };

  bool MergingBlock::run(bool oneShot){
    if (_myInputs["BLOCK__ADD_IN_PARAM1"].isDefaultValue())
      return false;
    if (_myInputs["BLOCK__ADD_IN_PARAM2"].isDefaultValue())
      return false;
    ParamType type1 = _myInputs["BLOCK__ADD_IN_PARAM1"].getType(false),
      type2 = _myInputs["BLOCK__ADD_IN_PARAM2"].getType(false);
    //Boolean = 0, Int, Float, Color, Matrix, String, FilePath, ListBox, AnyType, typeError

    switch (type1)
    {
    case Int:
    {
      int param1 = _myInputs["BLOCK__ADD_IN_PARAM1"].get<int>();
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<int>();
        break;
      case Float:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<double>();
        break;
      case Color:
      {
        cv::Scalar tmp(param1, param1, param1, param1);
        _myOutputs["BLOCK__ADD_OUTPUT"] = tmp + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Scalar>();
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Mat>();
        break;
      case String:
      case FilePath:
        _myOutputs["BLOCK__ADD_OUTPUT"] = lexical_cast<std::string>(param1) + _myInputs["BLOCK__ADD_IN_PARAM2"].get<std::string>();
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Float:
    {
      double param1 = _myInputs["BLOCK__ADD_IN_PARAM1"].get<double>();
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<int>();
        break;
      case Float:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<double>();
        break;
      case Color:
      {
        cv::Scalar tmp(param1, param1, param1, param1);
        _myOutputs["BLOCK__ADD_OUTPUT"] = tmp + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Scalar>();
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Mat>();
        break;
      case String:
      case FilePath:
        _myOutputs["BLOCK__ADD_OUTPUT"] = lexical_cast<std::string>(param1)+_myInputs["BLOCK__ADD_IN_PARAM2"].get<std::string>();
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Color:
    {
      cv::Scalar param1 = _myInputs["BLOCK__ADD_IN_PARAM1"].get<cv::Scalar>();
      switch (type2)
      {
      case Int:
      {
        int param2 = _myInputs["BLOCK__ADD_IN_PARAM2"].get<int>();
        cv::Scalar tmp(param2, param2, param2, param2);
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + tmp;
        break;
      }
      case Float:
      {
        double param2 = _myInputs["BLOCK__ADD_IN_PARAM2"].get<double>();
        cv::Scalar tmp(param2, param2, param2, param2);
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + tmp;
        break;
      }
      case Color:
      {
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Scalar>();
        break;
      }
      case Matrix:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Mat>();
        break;
      case String:
      case FilePath:
        _myOutputs["BLOCK__ADD_OUTPUT"] = lexical_cast<std::string>(param1)+_myInputs["BLOCK__ADD_IN_PARAM2"].get<std::string>();
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case Matrix:
    {
      cv::Mat param1 = _myInputs["BLOCK__ADD_IN_PARAM1"].get<cv::Mat>();
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<int>();
        break;
      case Float:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<double>();
        break;
      case Color:
      {
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Scalar>();
        break;
      }
      case Matrix:
      {
        cv::Mat param2 = _myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Mat>();
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
        _myOutputs["BLOCK__ADD_OUTPUT"] = dest;
      }
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
    case String:
    case FilePath:
    {
      std::string param1 = _myInputs["BLOCK__ADD_IN_PARAM1"].get<std::string>();
      switch (type2)
      {
      case Int:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + lexical_cast<std::string>(_myInputs["BLOCK__ADD_IN_PARAM2"].get<int>());
        break;
      case Float:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + lexical_cast<std::string>(_myInputs["BLOCK__ADD_IN_PARAM2"].get<double>());
        break;
      case Color:
      {
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + lexical_cast<std::string>(_myInputs["BLOCK__ADD_IN_PARAM2"].get<cv::Scalar>());
        break;
      }
      case String:
      case FilePath:
        _myOutputs["BLOCK__ADD_OUTPUT"] = param1 + _myInputs["BLOCK__ADD_IN_PARAM2"].get<std::string>();
        break;
      default:
        return false;//nothing to do as we don't support this type of operation
        break;
      }
      break;
    }
      break;
    default:
      return false;//nothing to do as we don't support this type of operation
      break;
    }

    return true;
  };
};