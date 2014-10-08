
#include <vector>
#include <opencv2/nonfree.hpp>
#include <opencv2/features2d.hpp>

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
  BLOCK_END_INSTANTIATION(PointFinderBlock, AlgoType::imgProcess, BLOCK__POINT_FINDER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(PointFinderBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_IN_IMAGE", "BLOCK__POINT_FINDER_IN_IMAGE_HELP");
  ADD_PARAMETER(false, ListBox, "BLOCK__POINT_FINDER_IN_DETECTOR", "BLOCK__POINT_FINDER_IN_DETECTOR_HELP");
  ADD_PARAMETER(false, ListBox, "BLOCK__POINT_FINDER_IN_EXTRACTOR", "BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(PointFinderBlock); 
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_OUT_POINTS", "BLOCK__POINT_FINDER_OUT_POINTS_HELP");
  ADD_PARAMETER(false, Matrix, "BLOCK__POINT_FINDER_OUT_DESC", "BLOCK__POINT_FINDER_OUT_DESC_HELP");
  END_BLOCK_PARAMS();

  PointFinderBlock::PointFinderBlock() :Block("BLOCK__POINT_FINDER_NAME"){
    _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].addValidator({ new ValNeeded() });
  };
  
  bool PointFinderBlock::run(){
    cv::Mat mat = _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].get<cv::Mat>(),
      desc;

    std::vector<cv::KeyPoint> points;
    switch (_myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].get<int>())
    {
    case 1:
      FeatureDetector::create("BRISK")->detect(mat, points);
      break;
    case 2:
      FeatureDetector::create("FAST")->detect(mat, points);
      break;
    case 3:
      FeatureDetector::create("FREAK")->detect(mat, points);
      break;
    case 4:
      FeatureDetector::create("GFTT")->detect(mat, points);
      break;
    case 5:
      FeatureDetector::create("HARRIS")->detect(mat, points);
      break;
    case 6:
      FeatureDetector::create("MSER")->detect(mat, points);
      break;
    case 7:
      FeatureDetector::create("ORB")->detect(mat, points);
      break;
    case 8:
      FeatureDetector::create("SIFT")->detect(mat, points);
      break;
    case 9:
      FeatureDetector::create("STAR")->detect(mat, points);
      break;
    case 10:
      FeatureDetector::create("SURF")->detect(mat, points);
      break;
    case 11:
      FeatureDetector::create("SimpleBlob")->detect(mat, points);
      break;
    default:
      FeatureDetector::create("BRIEF")->detect(mat, points);
      break;
    }
    _myOutputs["BLOCK__POINT_FINDER_OUT_IMAGE"] = ((cv::Mat)points).clone();
    if (_myOutputs["BLOCK__POINT_FINDER_OUT_DESC"].isNeeded())
    {
      switch (_myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].get<int>())
      {
      case 1://SIFT^SURF^ORB^BRIEF^BRISK^MSER
        DescriptorExtractor::create("SURF")->compute(mat, points, desc);
        break;
      case 2:
        DescriptorExtractor::create("ORB")->compute(mat, points, desc);
        break;
      case 3:
        DescriptorExtractor::create("BRIEF")->compute(mat, points, desc);
        break;
      case 4:
        DescriptorExtractor::create("BRISK")->compute(mat, points, desc);
        break;
      case 5:
        DescriptorExtractor::create("MSER")->compute(mat, points, desc);
        break;
      default:
        DescriptorExtractor::create("SIFT")->compute(mat, points, desc);
        break;
      }
      _myOutputs["BLOCK__POINT_FINDER_OUT_DESC"] = desc;
    }
    renderingDone();
    return true;
  };
};