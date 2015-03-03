
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
using namespace lsis_org;
using std::vector;
using std::string;
using namespace cv;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(PointFinderBlock);
  //You can add methods, re implement needed functions... 
  LineFinder filter;
  std::vector<std::string> detectorList;
  std::vector<std::string> modificatorList;
  std::vector<std::string> extractorList;

  void setParamOpencv(cv::Algorithm* algo, string paramName, string subParamName);
public:
  BLOCK_END_INSTANTIATION(PointFinderBlock, AlgoType::imgProcess, BLOCK__POINT_FINDER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(PointFinderBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_IN_IMAGE", "BLOCK__POINT_FINDER_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__POINT_FINDER_IN_DETECTOR", "BLOCK__POINT_FINDER_IN_DETECTOR_HELP", 0);
  ADD_PARAMETER(false, ListBox, "BLOCK__POINT_FINDER_IN_MODIFICATOR", "BLOCK__POINT_FINDER_IN_MODIFICATOR_HELP"); 
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__POINT_FINDER_IN_EXTRACTOR", "BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(PointFinderBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_OUT_POINTS", "BLOCK__POINT_FINDER_OUT_POINTS_HELP");
  ADD_PARAMETER(false, Matrix, "BLOCK__POINT_FINDER_OUT_DESC", "BLOCK__POINT_FINDER_OUT_DESC_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(PointFinderBlock);
  size_t pos = _STR("BLOCK__POINT_FINDER_IN_DETECTOR_HELP").find_first_of('|');
  if (pos != std::string::npos)
  {
    std::string params = _STR("BLOCK__POINT_FINDER_IN_DETECTOR_HELP").substr(pos + 1);
    std::vector<std::string> detectorList;
    boost::split(detectorList, params, boost::is_any_of("^"));
    //now create all subparameters:
    for (string param : detectorList)
    {
      vector<cv::String> out;
      cv::Ptr<FeatureDetector> detect = FeatureDetector::create(param);
      if (!detect.empty())
      {
        detect->getParams(out);
        for (cv::String subParam : out)
          ADD_SUBPARAM_FROM_OPENCV_ALGO(detect, "BLOCK__POINT_FINDER_IN_DETECTOR", param, subParam);
      }
    }
  };
  pos = _STR("BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP").find_first_of('|');
  if (pos != std::string::npos)
  {
    std::string params = _STR("BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP").substr(pos + 1);
    std::vector<std::string> extractorList;
    boost::split(extractorList, params, boost::is_any_of("^"));

    //now create all subparameters:
    for (string param : extractorList)
    {
      vector<cv::String> out;
      cv::Ptr<DescriptorExtractor> detect = DescriptorExtractor::create(param);
      if (!detect.empty())
      {
        detect->getParams(out);
        for (cv::String subParam : out)
          ADD_SUBPARAM_FROM_OPENCV_ALGO(detect, "BLOCK__POINT_FINDER_IN_EXTRACTOR", param, subParam);
      }
    }
  };
  pos = _STR("BLOCK__POINT_FINDER_IN_MODIFICATOR_HELP").find_first_of('|');
  if (pos != std::string::npos)
  {
    std::string params = _STR("BLOCK__POINT_FINDER_IN_MODIFICATOR_HELP").substr(pos + 1);
    std::vector<std::string> modificatorList;
    boost::split(modificatorList, params, boost::is_any_of("^"));
    //now create all subparameters:
    for (string param : modificatorList)
    {
      if (param == "Grid")
      {
        vector<cv::String> out;
        cv::Ptr<FeatureDetector> detect = FeatureDetector::create(param);
        if (!detect.empty())
        {
          detect->getParams(out);
          for (cv::String subParam : out)
            ADD_SUBPARAM_FROM_OPENCV_ALGO(detect, "BLOCK__POINT_FINDER_IN_MODIFICATOR", param, subParam);
        }
      }
    }
  }
  END_BLOCK_PARAMS();

  PointFinderBlock::PointFinderBlock() :Block("BLOCK__POINT_FINDER_NAME", true){
    _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].addValidator({ new ValNeeded() });

    size_t pos = _STR("BLOCK__POINT_FINDER_IN_DETECTOR_HELP").find_first_of('|');
    if (pos != std::string::npos)
    {
      std::string params = _STR("BLOCK__POINT_FINDER_IN_DETECTOR_HELP").substr(pos + 1);
      boost::split(detectorList, params, boost::is_any_of("^"));
    };
    pos = _STR("BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP").find_first_of('|');
    if (pos != std::string::npos)
    {
      std::string params = _STR("BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP").substr(pos + 1);
      boost::split(extractorList, params, boost::is_any_of("^"));
    }
    pos = _STR("BLOCK__POINT_FINDER_IN_MODIFICATOR_HELP").find_first_of('|');
    if (pos != std::string::npos)
    {
      std::string params = _STR("BLOCK__POINT_FINDER_IN_MODIFICATOR_HELP").substr(pos + 1);
      boost::split(modificatorList, params, boost::is_any_of("^"));
    }
  };
  
  void PointFinderBlock::setParamOpencv(cv::Algorithm* algo, string paramName, string subParamName)
  {

    switch (algo->paramType(subParamName)){
    case cv::Param::BOOLEAN:
      algo->set(subParamName, _mySubParams[paramName + "." + subParamName].get<bool>());
      break;
    case cv::Param::UNSIGNED_INT: 
    case cv::Param::UINT64: 
    case cv::Param::UCHAR: 
    case cv::Param::INT:
      algo->set(subParamName, _mySubParams[paramName + "." + subParamName].get<int>());
      break;
    case cv::Param::REAL: 
    case cv::Param::FLOAT:
      algo->set(subParamName, _mySubParams[paramName + "." + subParamName].get<double>());
      break;
    case cv::Param::STRING:
      algo->set(subParamName, _mySubParams[paramName + "." + subParamName].get<std::string>());
      break;
    case cv::Param::MAT:
      algo->set(subParamName, _mySubParams[paramName + "." + subParamName].get<cv::Mat>());
      break;
    } 
  }

  bool PointFinderBlock::run(bool oneShot){
    cv::Mat mat = _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].get<cv::Mat>(),
      desc;

    std::vector<cv::KeyPoint> points;
    int methodDetect = _myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].get<int>();

    cv::Ptr<FeatureDetector> detect = FeatureDetector::create(detectorList[methodDetect]);

    //set sub params values:
    
    vector<cv::String> subValuesDetector;
    detect->getParams(subValuesDetector);
    for (cv::String subParam : subValuesDetector)
      setParamOpencv(detect, "BLOCK__POINT_FINDER_IN_DETECTOR." + detectorList[methodDetect], subParam);
    
    string modificator = "";
    if (!_myInputs["BLOCK__POINT_FINDER_IN_MODIFICATOR"].isDefaultValue())
    {
      int idxModificator = _myInputs["BLOCK__POINT_FINDER_IN_MODIFICATOR"].get<int>();
      modificator = modificatorList[idxModificator];
      if (modificator.compare("Grid")==0)
      {
        cv::Ptr<FeatureDetector> subdetect = FeatureDetector::create(modificator);
        subdetect->set("detector", detect);
        //set sub params values:
        vector<cv::String> out;
        subdetect->getParams(out);
        for (cv::String subParam : out)
          setParamOpencv(subdetect, "BLOCK__POINT_FINDER_IN_MODIFICATOR." + modificator, subParam);

        detect = subdetect;//detector are now chained, take the parent
      }
    }
    
    if (!detect.empty())
      detect->detect(mat, points);

    _myOutputs["BLOCK__POINT_FINDER_OUT_POINTS"] = ((cv::Mat)points).clone();
    if (_myOutputs["BLOCK__POINT_FINDER_OUT_DESC"].isNeeded())
    {
      int methodExtract = _myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].get<int>();
      cv::Ptr<DescriptorExtractor> extract = DescriptorExtractor::create(extractorList[methodExtract].c_str());

      //set sub params values:
      vector<cv::String> subValuesExtractor;
      extract->getParams(subValuesExtractor);
      for (cv::String subParam : subValuesExtractor)
        setParamOpencv(extract, "BLOCK__POINT_FINDER_IN_EXTRACTOR." + extractorList[methodExtract], subParam);

      if (!extract.empty())
        extract->compute(mat, points, desc);

      _myOutputs["BLOCK__POINT_FINDER_OUT_DESC"] = desc;
    }

    return true;
  };
};