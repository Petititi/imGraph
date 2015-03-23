
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503 4190)
#endif
#include <vector>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include <boost/algorithm/string.hpp>
#include "OpenCV_filter.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
using namespace lsis_org;
using std::vector;
using std::string;
using namespace cv;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(PointFinderBlock);
  //You can add methods, re implement needed functions... 
  cv::Ptr<cv::Algorithm> _detect;
  cv::Ptr<cv::Algorithm> _extract;
  std::vector<std::string> detectorList;
  std::vector<std::string> extractorList;

  void setParamOpencv(cv::Ptr<cv::Algorithm>& algo, string paramName);
  cv::Ptr<cv::Algorithm> createAlgo(string name, string prefix);
public:
  BLOCK_END_INSTANTIATION(PointFinderBlock, AlgoType::imgProcess, BLOCK__POINT_FINDER_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(PointFinderBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_IN_IMAGE", "BLOCK__POINT_FINDER_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__POINT_FINDER_IN_DETECTOR", "BLOCK__POINT_FINDER_IN_DETECTOR_HELP", 0);
  ADD_PARAMETER_FULL(false, ListBox, "BLOCK__POINT_FINDER_IN_EXTRACTOR", "BLOCK__POINT_FINDER_IN_EXTRACTOR_HELP", 0);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(PointFinderBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__POINT_FINDER_OUT_POINTS", "BLOCK__POINT_FINDER_OUT_POINTS_HELP");
  ADD_PARAMETER(false, Matrix, "BLOCK__POINT_FINDER_OUT_DESC", "BLOCK__POINT_FINDER_OUT_DESC_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(PointFinderBlock);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.BRISK.octaves", "octaves", 3);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.BRISK.thres", "thres", 30);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.BRISK.patternScale", "patternScale", 1.);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.FAST.nonmaxSuppression", "nonmaxSuppression", 1);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.FAST.threshold", "threshold", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.FAST.type", "type", 2);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.GFTT.k", "k", 0.04);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.GFTT.minDistance", "minDistance", 1);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.GFTT.MaxFeatures", "MaxFeatures", 1000);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.GFTT.qualityLevel", "qualityLevel", 0.01);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.GFTT.blockSize", "blockSize", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.HARRIS.k", "k", 0.04);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.HARRIS.minDistance", "minDistance", 1);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.HARRIS.MaxFeatures", "MaxFeatures", 1000);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.HARRIS.qualityLevel", "qualityLevel", 0.01);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.HARRIS.blockSize", "blockSize", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.areaThreshold", "areaThreshold", 1.01);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.delta", "delta", 5);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.edgeBlurSize", "edgeBlurSize", 5);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.maxArea", "maxArea", 14400);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.maxEvolution", "maxEvolution", 200);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.maxVariation", "maxVariation", 0.25);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.minArea", "minArea", 60);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.minDiversity", "minDiversity", 0.2);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.MSER.minMargin", "minMargin", 0.003);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.WTA_K", "WTA_K", 2);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.edgeThreshold", "edgeThreshold", 31);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.firstLevel", "firstLevel", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.nFeatures", "nFeatures", 500);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.nLevels", "nLevels", 8);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.patchSize", "patchSize", 31);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.scaleFactor", "scaleFactor", 1.2);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.scoreType", "scoreType", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.ORB.fastThreshold", "fastThreshold", 20);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SIFT.contrastThreshold", "contrastThreshold", 0.04);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SIFT.edgeThreshold", "edgeThreshold", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SIFT.nFeatures", "nFeatures", 500);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SIFT.nOctaveLayers", "nOctaveLayers", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SIFT.sigma", "sigma", 1.6);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.STAR.lineThresholdBinarized", "lineThresholdBinarized", 8);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.STAR.lineThresholdProjected", "lineThresholdProjected", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.STAR.maxSize", "maxSize", 45);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.STAR.responseThreshold", "responseThreshold", 30);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.STAR.suppressNonmaxSize", "suppressNonmaxSize", 5);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SURF.extended", "extended", 0);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SURF.hessianThreshold", "hessianThreshold", 100);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SURF.nOctaveLayers", "nOctaveLayers", 3);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SURF.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SURF.upright", "upright", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.blobColor", "blobColor", 0);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.filterByArea", "filterByArea", 1);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.filterByCircularity", "filterByCircularity", 0);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.filterByColor", "filterByColor", 1);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.filterByConvexity", "filterByConvexity", 1);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.filterByInertia", "filterByInertia", 1);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.maxArea", "maxArea", 5000);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.maxCircularity", "maxCircularity", 3.40282e+038);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.maxConvexity", "maxConvexity", 3.40282e+038);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.maxInertiaRatio", "maxInertiaRatio", 3.40282e+038);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.maxThreshold", "maxThreshold", 220);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minDistBetweenBlobs", "minDistBetweenBlobs", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minRepeatability", "minRepeatability", 2);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minArea", "minArea", 5);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minCircularity", "minCircularity", 5);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minInertiaRatio", "minInertiaRatio", 5);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minConvexity", "minConvexity", 5);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.minThreshold", "minThreshold", 50);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.SimpleBlob.thresholdStep", "thresholdStep", 10);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.extended", "extended", false);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.upright", "upright", false);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.threshold", "threshold", 0.001);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.nOctaveLayers", "nOctaveLayers", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.KAZE.diffusivity", "diffusivity", 1);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.descriptor_type", "descriptor_type", 5);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.upright", "upright", false);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.descriptor_size", "descriptor_size", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.descriptor_channels", "descriptor_channels", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.threshold", "threshold", 0.001);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.nOctaveLayers", "nOctaveLayers", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_DETECTOR.AKAZE.diffusivity", "diffusivity", 1);

  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SIFT.contrastThreshold", "contrastThreshold", 0.04);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SIFT.edgeThreshold", "edgeThreshold", 10);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SIFT.nFeatures", "nFeatures", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SIFT.nOctaveLayers", "nOctaveLayers", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SIFT.sigma", "sigma", 1.6);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SURF.extended", "extended", 0);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SURF.hessianThreshold", "hessianThreshold", 100);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SURF.nOctaveLayers", "nOctaveLayers", 3);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SURF.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.SURF.upright", "upright", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.WTA_K", "WTA_K", 2);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.edgeThreshold", "edgeThreshold", 31);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.firstLevel", "firstLevel", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.nFeatures", "nFeatures", 500);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.nLevels", "nLevels", 8);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.patchSize", "patchSize", 31);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.scaleFactor", "scaleFactor", 1.2);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.scoreType", "scoreType", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.ORB.fastThreshold", "fastThreshold", 20); 
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.BRIEF.bytes", "bytes", 32);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.BRISK.octaves", "octaves", 3);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.BRISK.thres", "thres", 30);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.FREAK.orientationNormalized", "orientationNormalized", true);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.FREAK.scaleNormalized", "scaleNormalized", true);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.FREAK.patternScale", "patternScale", 22.);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.FREAK.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.extended", "extended", false);
  ADD_PARAMETER_FULL(notUsed, Boolean, "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.upright", "upright", false);
  ADD_PARAMETER_FULL(notUsed, Float,  "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.threshold", "threshold", 0.001);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.nOctaveLayers", "nOctaveLayers", 4);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.KAZE.diffusivity", "diffusivity", 1);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.descriptor_type", "descriptor_type", 5);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.upright", "upright", false);
  ADD_PARAMETER_FULL(notUsed, Int,    "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.descriptor_size", "descriptor_size", 0);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.descriptor_channels", "descriptor_channels", 3);
  ADD_PARAMETER_FULL(notUsed, Float, "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.threshold", "threshold", 0.001);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.nOctaves", "nOctaves", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.nOctaveLayers", "nOctaveLayers", 4);
  ADD_PARAMETER_FULL(notUsed, Int, "BLOCK__POINT_FINDER_IN_EXTRACTOR.AKAZE.diffusivity", "diffusivity", 1);
  END_BLOCK_PARAMS();

  PointFinderBlock::PointFinderBlock() :Block("BLOCK__POINT_FINDER_NAME", true){
    _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].addValidator({ new ValNeeded() });

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
  };
  
  void PointFinderBlock::setParamOpencv(cv::Ptr<cv::Algorithm>& algo, string paramName)
  {
    size_t pos = paramName.find_first_of('.');
    if (pos != std::string::npos)
    {
      std::string nomAlgo = paramName.substr(pos + 1);
      if (nomAlgo.compare("BRISK") == 0)
      {
        if (_mySubParams[paramName + ".thres"].isNew() ||
          _mySubParams[paramName + ".octaves"].isNew() ||
          _mySubParams[paramName + ".patternScale"].isNew())
          algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
      else if (nomAlgo.compare("FAST") == 0)
      {
        FastFeatureDetector* myAlgo = dynamic_cast<FastFeatureDetector*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".nonmaxSuppression"].isNew() || 
          _mySubParams[paramName + ".threshold"].isNew() || 
          _mySubParams[paramName + ".type"].isNew() ))
        {
          myAlgo->setNonmaxSuppression(_mySubParams[paramName + ".nonmaxSuppression"].get<bool>());
          myAlgo->setThreshold(_mySubParams[paramName + ".threshold"].get<int>());
          myAlgo->setType(_mySubParams[paramName + ".type"].get<int>());
        }
      }
      else if (nomAlgo.compare("GFTT") == 0 || nomAlgo.compare("HARRIS") == 0)
      {
        GFTTDetector* myAlgo = dynamic_cast<GFTTDetector*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".k"].isNew() ||
          _mySubParams[paramName + ".minDistance"].isNew() ||
          _mySubParams[paramName + ".MaxFeatures"].isNew() ||
          _mySubParams[paramName + ".qualityLevel"].isNew()))
        {
          myAlgo->setK(_mySubParams[paramName + ".k"].get<double>());
          myAlgo->setMinDistance(_mySubParams[paramName + ".minDistance"].get<double>());
          myAlgo->setMaxFeatures(_mySubParams[paramName + ".MaxFeatures"].get<int>());
          myAlgo->setQualityLevel(_mySubParams[paramName + ".qualityLevel"].get<double>());
          myAlgo->setHarrisDetector(nomAlgo.compare("HARRIS") == 0);
        }
      }
      else if (nomAlgo.compare("MSER") == 0)
      {
        MSER* myAlgo = dynamic_cast<MSER*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".delta"].isNew() ||
          _mySubParams[paramName + ".minArea"].isNew() ||
          _mySubParams[paramName + ".maxArea"].isNew()))
        {
          myAlgo->setDelta(_mySubParams[paramName + ".delta"].get<int>());
          myAlgo->setMinArea(_mySubParams[paramName + ".minArea"].get<int>());
          myAlgo->setMaxArea(_mySubParams[paramName + ".maxArea"].get<int>());
        }
      }
      else if (nomAlgo.compare("ORB") == 0)
      {
        ORB* myAlgo = dynamic_cast<ORB*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".WTA_K"].isNew() ||
          _mySubParams[paramName + ".edgeThreshold"].isNew() ||
          _mySubParams[paramName + ".firstLevel"].isNew() ||
          _mySubParams[paramName + ".nFeatures"].isNew() ||
          _mySubParams[paramName + ".nLevels"].isNew() ||
          _mySubParams[paramName + ".patchSize"].isNew() ||
          _mySubParams[paramName + ".scaleFactor"].isNew() ||
          _mySubParams[paramName + ".scoreType"].isNew() ||
          _mySubParams[paramName + ".fastThreshold"].isNew()))
        {
          myAlgo->setWTA_K(_mySubParams[paramName + ".WTA_K"].get<int>());
          myAlgo->setEdgeThreshold(_mySubParams[paramName + ".edgeThreshold"].get<int>());
          myAlgo->setFirstLevel(_mySubParams[paramName + ".firstLevel"].get<int>());
          myAlgo->setMaxFeatures(_mySubParams[paramName + ".nFeatures"].get<int>());
          myAlgo->setNLevels(_mySubParams[paramName + ".nLevels"].get<int>());
          myAlgo->setPatchSize(_mySubParams[paramName + ".patchSize"].get<int>());
          myAlgo->setScaleFactor(_mySubParams[paramName + ".scaleFactor"].get<double>());
          myAlgo->setScoreType(_mySubParams[paramName + ".scoreType"].get<int>());
          myAlgo->setFastThreshold(_mySubParams[paramName + ".fastThreshold"].get<int>());
        }
      }
      else if (nomAlgo.compare("SIFT") == 0)
      {
        if (_mySubParams[paramName + ".nFeatures"].isNew() ||
          _mySubParams[paramName + ".nOctaveLayers"].isNew() ||
          _mySubParams[paramName + ".contrastThreshold"].isNew() ||
          _mySubParams[paramName + ".edgeThreshold"].isNew() ||
          _mySubParams[paramName + ".sigma"].isNew())
        algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
      else if (nomAlgo.compare("SURF") == 0)
      {
        xfeatures2d::SURF* myAlgo = dynamic_cast<xfeatures2d::SURF*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".extended"].isNew() ||
          _mySubParams[paramName + ".hessianThreshold"].isNew() ||
          _mySubParams[paramName + ".nOctaveLayers"].isNew() ||
          _mySubParams[paramName + ".nOctaves"].isNew() ||
          _mySubParams[paramName + ".upright"].isNew()))
        {
          myAlgo->setExtended(_mySubParams[paramName + ".extended"].get<bool>());
          myAlgo->setHessianThreshold(_mySubParams[paramName + ".hessianThreshold"].get<double>());
          myAlgo->setNOctaveLayers(_mySubParams[paramName + ".nOctaveLayers"].get<int>());
          myAlgo->setNOctaves(_mySubParams[paramName + ".nOctaves"].get<int>());
          myAlgo->setUpright(_mySubParams[paramName + ".upright"].get<bool>());
        }
      }
      else if (nomAlgo.compare("KAZE") == 0)
      {
        KAZE* myAlgo = dynamic_cast<KAZE*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".extended"].isNew() ||
          _mySubParams[paramName + ".upright"].isNew() ||
          _mySubParams[paramName + ".threshold"].isNew() ||
          _mySubParams[paramName + ".nOctaves"].isNew() ||
          _mySubParams[paramName + ".nOctaveLayers"].isNew() ||
          _mySubParams[paramName + ".diffusivity"].isNew()))
        {
          myAlgo->setExtended(_mySubParams[paramName + ".extended"].get<bool>());
          myAlgo->setUpright(_mySubParams[paramName + ".upright"].get<bool>());
          myAlgo->setThreshold(_mySubParams[paramName + ".threshold"].get<float>());
          myAlgo->setNOctaves(_mySubParams[paramName + ".nOctaves"].get<int>());
          myAlgo->setNOctaveLayers(_mySubParams[paramName + ".nOctaveLayers"].get<int>());
          myAlgo->setDiffusivity(_mySubParams[paramName + ".diffusivity"].get<int>());
        }
      }
      else if (nomAlgo.compare("AKAZE") == 0)
      {
        AKAZE* myAlgo = dynamic_cast<AKAZE*>(algo.get());
        if (myAlgo != NULL &&
          (_mySubParams[paramName + ".descriptor_type"].isNew() ||
          _mySubParams[paramName + ".descriptor_size"].isNew() ||
          _mySubParams[paramName + ".descriptor_channels"].isNew() ||
          _mySubParams[paramName + ".threshold"].isNew() ||
          _mySubParams[paramName + ".nOctaves"].isNew() ||
          _mySubParams[paramName + ".nOctaveLayers"].isNew() ||
          _mySubParams[paramName + ".diffusivity"].isNew()))
        {
          myAlgo->setDescriptorType(_mySubParams[paramName + ".descriptor_type"].get<int>());
          myAlgo->setDescriptorSize(_mySubParams[paramName + ".descriptor_size"].get<int>());
          myAlgo->setDescriptorChannels(_mySubParams[paramName + ".descriptor_channels"].get<int>());
          myAlgo->setThreshold(_mySubParams[paramName + ".threshold"].get<float>());
          myAlgo->setNOctaves(_mySubParams[paramName + ".nOctaves"].get<int>());
          myAlgo->setNOctaveLayers(_mySubParams[paramName + ".nOctaveLayers"].get<int>());
          myAlgo->setDiffusivity(_mySubParams[paramName + ".diffusivity"].get<int>());
        }
      }
      else if (nomAlgo.compare("STAR") == 0)
      {
        if (_mySubParams[paramName + ".maxSize"].isNew() ||
          _mySubParams[paramName + ".responseThreshold"].isNew() ||
          _mySubParams[paramName + ".lineThresholdProjected"].isNew() ||
          _mySubParams[paramName + ".lineThresholdBinarized"].isNew() ||
          _mySubParams[paramName + ".suppressNonmaxSize"].isNew())
          algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
      else if (nomAlgo.compare("STAR") == 0)
      {
        if (_mySubParams[paramName + ".orientationNormalized"].isNew() ||
          _mySubParams[paramName + ".scaleNormalized"].isNew() ||
          _mySubParams[paramName + ".patternScale"].isNew() ||
          _mySubParams[paramName + ".nOctaves"].isNew())
          algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
      else if (nomAlgo.compare("SimpleBlob") == 0)
      {
        if (_mySubParams[paramName + ".thresholdStep"].isNew() ||
          _mySubParams[paramName + ".minThreshold"].isNew() ||
          _mySubParams[paramName + ".maxThreshold"].isNew() ||
          _mySubParams[paramName + ".minRepeatability"].isNew() ||
          _mySubParams[paramName + ".minDistBetweenBlobs"].isNew() ||
          _mySubParams[paramName + ".suppressNonmaxSize"].isNew() ||
          _mySubParams[paramName + ".filterByColor"].isNew() ||
          _mySubParams[paramName + ".blobColor"].isNew() ||
          _mySubParams[paramName + ".filterByArea"].isNew() ||
          _mySubParams[paramName + ".maxArea"].isNew() ||
          _mySubParams[paramName + ".minArea"].isNew() ||
          _mySubParams[paramName + ".filterByCircularity"].isNew() ||
          _mySubParams[paramName + ".maxCircularity"].isNew() ||
          _mySubParams[paramName + ".minCircularity"].isNew() ||
          _mySubParams[paramName + ".filterByInertia"].isNew() ||
          _mySubParams[paramName + ".maxInertiaRatio"].isNew() ||
          _mySubParams[paramName + ".minInertiaRatio"].isNew() ||
          _mySubParams[paramName + ".filterByConvexity"].isNew() ||
          _mySubParams[paramName + ".maxConvexity"].isNew() ||
          _mySubParams[paramName + ".minConvexity"].isNew())
          algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
      else if (nomAlgo.compare("BRIEF") == 0)
      {
        if (_mySubParams[paramName + ".bytes"].isNew())
          algo = createAlgo(nomAlgo, paramName.substr(0, pos));
      }
    };
    //mark each subparam as not new:
    for (auto& val : _mySubParams)
      val.second.setNew(false);
  }

  cv::Ptr<cv::Algorithm> PointFinderBlock::createAlgo(string nomAlgo, string prefix)
  {
    _myInputs[prefix].setNew(false);
    cv::Ptr<cv::Algorithm> output;
    string fullName = prefix + "." + nomAlgo;
    if (nomAlgo.compare("BRISK") == 0)
    {
      output = BRISK::create(_mySubParams[fullName + ".thres"].get<int>(),
        _mySubParams[fullName + ".octaves"].get<int>(),
        _mySubParams[fullName + ".patternScale"].get<float>());
    }
    else if (nomAlgo.compare("FAST") == 0)
    {
      output = FastFeatureDetector::create(_mySubParams[fullName + ".threshold"].get<int>(),
        _mySubParams[fullName + ".nonmaxSuppression"].get<bool>(),
        _mySubParams[fullName + ".type"].get<int>());
    }
    else if (nomAlgo.compare("GFTT") == 0 || nomAlgo.compare("HARRIS") == 0)
    {
      output = GFTTDetector::create(_mySubParams[fullName + ".MaxFeatures"].get<int>(),
        _mySubParams[fullName + ".qualityLevel"].get<double>(),
        _mySubParams[fullName + ".minDistance"].get<double>(),
        _mySubParams[fullName + ".blockSize"].get<int>(),
        nomAlgo.compare("HARRIS") == 0,
        _mySubParams[fullName + ".k"].get<double>());
    }
    else if (nomAlgo.compare("MSER") == 0)
    {
      output = MSER::create(_mySubParams[fullName + ".delta"].get<int>(),
        _mySubParams[fullName + ".minArea"].get<int>(),
        _mySubParams[fullName + ".maxArea"].get<int>(),
        _mySubParams[fullName + ".maxVariation"].get<double>(),
        _mySubParams[fullName + ".minDiversity"].get<double>(),
        _mySubParams[fullName + ".maxEvolution"].get<int>(),
        _mySubParams[fullName + ".areaThreshold"].get<double>(),
        _mySubParams[fullName + ".minMargin"].get<double>(),
        _mySubParams[fullName + ".edgeBlurSize"].get<int>());
    }
    else if (nomAlgo.compare("ORB") == 0)
    {
      output = ORB::create(_mySubParams[fullName + ".nFeatures"].get<int>(),
        _mySubParams[fullName + ".scaleFactor"].get<float>(),
        _mySubParams[fullName + ".nLevels"].get<int>(),
        _mySubParams[fullName + ".edgeThreshold"].get<int>(),
        _mySubParams[fullName + ".firstLevel"].get<int>(),
        _mySubParams[fullName + ".WTA_K"].get<int>(),
        _mySubParams[fullName + ".scoreType"].get<int>(),
        _mySubParams[fullName + ".patchSize"].get<int>(),
        _mySubParams[fullName + ".fastThreshold"].get<int>());
    }
    else if (nomAlgo.compare("SIFT") == 0)
    {
      output = xfeatures2d::SIFT::create(_mySubParams[fullName + ".nFeatures"].get<int>(),
        _mySubParams[fullName + ".nOctaveLayers"].get<int>(),
        _mySubParams[fullName + ".contrastThreshold"].get<double>(),
        _mySubParams[fullName + ".edgeThreshold"].get<double>(),
        _mySubParams[fullName + ".sigma"].get<double>());
    }
    else if (nomAlgo.compare("SURF") == 0)
    {
      output = xfeatures2d::SURF::create(_mySubParams[fullName + ".hessianThreshold"].get<double>(),
        _mySubParams[fullName + ".nOctaves"].get<int>(),
        _mySubParams[fullName + ".nOctaveLayers"].get<int>(),
        _mySubParams[fullName + ".extended"].get<bool>(),
        _mySubParams[fullName + ".upright"].get<bool>());
    }
    else if (nomAlgo.compare("KAZE") == 0)
    {
      output = KAZE::create(_mySubParams[fullName + ".extended"].get<bool>(),
        _mySubParams[fullName + ".upright"].get<bool>(),
        _mySubParams[fullName + ".threshold"].get<float>(),
        _mySubParams[fullName + ".nOctaves"].get<int>(),
        _mySubParams[fullName + ".nOctaveLayers"].get<int>(),
        _mySubParams[fullName + ".diffusivity"].get<int>());
    }
    else if (nomAlgo.compare("AKAZE") == 0)
    {
      output = AKAZE::create(_mySubParams[fullName + ".descriptor_type"].get<int>(),
        _mySubParams[fullName + ".descriptor_size"].get<int>(),
        _mySubParams[fullName + ".descriptor_channels"].get<int>(),
        _mySubParams[fullName + ".threshold"].get<float>(),
        _mySubParams[fullName + ".nOctaves"].get<int>(),
        _mySubParams[fullName + ".nOctaveLayers"].get<int>(),
        _mySubParams[fullName + ".diffusivity"].get<int>());
    }
    else if (nomAlgo.compare("STAR") == 0)
    {
      output = xfeatures2d::StarDetector::create(_mySubParams[fullName + ".maxSize"].get<int>(),
        _mySubParams[fullName + ".responseThreshold"].get<int>(),
        _mySubParams[fullName + ".lineThresholdProjected"].get<int>(),
        _mySubParams[fullName + ".lineThresholdBinarized"].get<int>(),
        _mySubParams[fullName + ".suppressNonmaxSize"].get<int>());
    }
    else if (nomAlgo.compare("FREAK") == 0)
    {
      output = xfeatures2d::FREAK::create(_mySubParams[fullName + ".orientationNormalized"].get<bool>(),
        _mySubParams[fullName + ".scaleNormalized"].get<bool>(),
        _mySubParams[fullName + ".patternScale"].get<float>(),
        _mySubParams[fullName + ".nOctaves"].get<int>());
    }
    else if (nomAlgo.compare("SimpleBlob") == 0)
    {
      SimpleBlobDetector::Params params;
      params.thresholdStep = _mySubParams[fullName + ".thresholdStep"].get<float>();
      params.minThreshold = _mySubParams[fullName + ".minThreshold"].get<float>();
      params.maxThreshold = _mySubParams[fullName + ".maxThreshold"].get<float>();
      params.minRepeatability = _mySubParams[fullName + ".minRepeatability"].get<int>();
      params.minDistBetweenBlobs = _mySubParams[fullName + ".minDistBetweenBlobs"].get<float>();

      params.filterByArea = _mySubParams[fullName + ".filterByColor"].get<bool>();
      params.blobColor = _mySubParams[fullName + ".blobColor"].get<int>();

      params.filterByArea = _mySubParams[fullName + ".filterByArea"].get<bool>();
      params.maxArea = _mySubParams[fullName + ".maxArea"].get<float>();
      params.minArea = _mySubParams[fullName + ".minArea"].get<float>();

      params.filterByCircularity = _mySubParams[fullName + ".filterByCircularity"].get<bool>();
      params.maxCircularity = _mySubParams[fullName + ".maxCircularity"].get<float>();
      params.minCircularity = _mySubParams[fullName + ".minCircularity"].get<float>();

      params.filterByInertia = _mySubParams[fullName + ".filterByInertia"].get<bool>();
      params.maxInertiaRatio = _mySubParams[fullName + ".maxInertiaRatio"].get<float>();
      params.minInertiaRatio = _mySubParams[fullName + ".minInertiaRatio"].get<float>();

      params.filterByConvexity = _mySubParams[fullName + ".filterByConvexity"].get<bool>();
      params.maxConvexity = _mySubParams[fullName + ".maxConvexity"].get<float>();
      params.minConvexity = _mySubParams[fullName + ".minConvexity"].get<float>();


      output = SimpleBlobDetector::create(params);
    }
    else if (nomAlgo.compare("BRIEF") == 0)
    {
      output = xfeatures2d::BriefDescriptorExtractor::create(_mySubParams[fullName + ".bytes"].get<int>());
    };
    //mark each subparam as not new:
    for (auto& val : _mySubParams)
      val.second.setNew(false);
    return output;
  }

  bool PointFinderBlock::run(bool oneShot){
    cv::Mat mat = _myInputs["BLOCK__POINT_FINDER_IN_IMAGE"].get<cv::Mat>(),
      desc;

    int methodDetect = _myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].get<int>();

    if (_myInputs["BLOCK__POINT_FINDER_IN_DETECTOR"].isNew() || _detect.empty())
      _detect = createAlgo(detectorList[methodDetect], "BLOCK__POINT_FINDER_IN_DETECTOR");

    setParamOpencv(_detect, "BLOCK__POINT_FINDER_IN_DETECTOR." + detectorList[methodDetect]);

    Feature2D* algo = dynamic_cast<Feature2D*>(_detect.get());
    if (algo == NULL)
      return false;//error...

    std::vector<cv::KeyPoint> points;
    algo->detect(mat, points);

    _myOutputs["BLOCK__POINT_FINDER_OUT_POINTS"] = ((cv::Mat)points).clone();
    if (_myOutputs["BLOCK__POINT_FINDER_OUT_DESC"].isNeeded())
    {
      int methodExtract = _myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].get<int>();
      if (_myInputs["BLOCK__POINT_FINDER_IN_EXTRACTOR"].isNew() || _extract.empty())
        _extract = createAlgo(extractorList[methodExtract], "BLOCK__POINT_FINDER_IN_EXTRACTOR");

      setParamOpencv(_extract, "BLOCK__POINT_FINDER_IN_EXTRACTOR." + extractorList[methodExtract]);

      algo = dynamic_cast<Feature2D*>(_extract.get());
      if (algo == NULL)
        return false;//error...

      algo->compute(mat, points, desc);

      _myOutputs["BLOCK__POINT_FINDER_OUT_DESC"] = desc;
    }

    return true;
  };
};