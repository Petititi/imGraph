
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190 4244)
#endif
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"

#ifdef _MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt
#endif

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
  BLOCK_BEGIN_INSTANTIATION(CORE_filter);
  inline int hamming_distance(char* vals1, char* vals2, int nbVals)
  {
    int output = 0;
    int i;
    int nbValsPacked = nbVals - 3;
    for (i = 0; i < nbValsPacked; i += 4) {
      output += __builtin_popcount(
        *((uint32_t *)(&vals1[i])) ^ *((uint32_t *)(&vals2[i])));
    }
    //if nbVals is not 4bytes packed
    for (; i < nbVals; i++)
      output += __builtin_popcount(vals1[i] ^ vals2[i]);
    return output;
  };

  template<typename T>
  inline float euclidean2_distance(T* vals1, T* vals2, int nbVals)
  {
    float output = 0;
    int i;
    for (i = 0; i < nbVals; i += 4) {
      T tmp = vals1[nbVals] - vals2[nbVals];
      output += static_cast<float>(tmp*tmp);
    }
    return output;
  };

  template<typename T>
  cv::Mat get_distances_float(cv::Mat features, float sigma)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_32FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_32FC1);

    float sigma_carre = 2 * sigma*sigma;

    float* ptr = output.ptr<float>();
    T* val = features.ptr<T>();
    int nbFeatures = features.cols;
    for (int i = 0; i < features.rows; i++)
    {
      int realAdress = i*features.rows;
      ptr[realAdress + i] = 0;//diagonal distance is null...
      T* val1 = &val[i*features.cols];
      for (int j = i + 1; j < features.rows; j++)
      {
        ptr[realAdress + j] = ptr[j*features.rows + i] = exp(
          - euclidean2_distance(val1, &val[j*features.cols], nbFeatures) / sigma_carre);
      }
    }
    return output;
  }

  cv::Mat get_distances_binary(cv::Mat features)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_32FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_32FC1);

    float* ptr = output.ptr<float>();
    char* val = features.ptr<char>();
    int nbFeatures = features.cols;
    for (int i = 0; i < features.rows; i++)
    {
      int realAdress = i*features.rows;
      ptr[realAdress + i] = 0;//diagonal distance is null...
      char* val1 = &val[i*features.cols];
      for (int j = i + 1; j < features.rows; j++)
      {
        ptr[realAdress + j] = ptr[j*features.rows + i] = (float)hamming_distance(
          val1, &val[j*features.cols], nbFeatures);
      }
    }
    return output;
  }

  std::vector<double> get_criterions_float(cv::Mat distances, double sigma, int D)
  {
    std::vector<double> output;
    double divisor = (1. / ((distances.rows - 1.) * pow((sigma*sqrt(2 * CV_PI)), D)));

    for (int i = 0; i < distances.cols; i++)
    {
      float* dist_i = distances.ptr<float>(i);
      double crit_i = 0, divisor = (1. / (distances.rows - 1.));
      for (int j = 0; j < distances.rows; j++)
      {
        if (i != j)
          crit_i += dist_i[j];
      }

      output.push_back(divisor * crit_i);
    }
    return output;
  };

  std::vector<double> get_criterions_binary(cv::Mat distances, double mu, int nbBits)
  {
    std::vector<double> output;
    double divisor = (1. / (distances.rows - 1.));
    for (int i = 0; i < distances.cols; i++)
    {
      float* dist_i = distances.ptr<float>(i);
      double crit_i = 0;
      for (int j = 0; j < distances.rows; j++)
      {
        if (i != j)
          crit_i += (pow(mu, (int)dist_i[j]) * pow(1. - mu, (int)(nbBits - dist_i[j])));
      }

      output.push_back(divisor * crit_i);
    }
    return output;
  };

  BLOCK_END_INSTANTIATION(CORE_filter, AlgoType::imgProcess, BLOCK__CORE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(CORE_filter);
  //Add parameters, with following parameters:
  //Based on opencv::Canny( InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize = 3, bool L2gradient = false );
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_IN_POINTS", "BLOCK__CORE_IN_POINTS_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_IN_DESC", "BLOCK__CORE_IN_DESC_HELP");
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CORE_IN_THRESHOLD", "BLOCK__CORE_IN_THRESHOLD_HELP", 90.f);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(CORE_filter);
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_OUT_POINTS", "BLOCK__CORE_OUT_POINTS_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_OUT_DESC", "BLOCK__CORE_OUT_DESC_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(CORE_filter);
  END_BLOCK_PARAMS();

  CORE_filter::CORE_filter() :Block("BLOCK__CORE_NAME", true){
    _myInputs["BLOCK__CORE_IN_POINTS"].addValidator({ new ValNeeded() });
    _myInputs["BLOCK__CORE_IN_THRESHOLD"].addValidator({ new ValRange(0,100) });
  };

  template <typename T>
  vector<size_t> sort_indexes(const vector<T> &v) {

    // initialize original index locations
    vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
      [&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

    return idx;
  }

  bool CORE_filter::run(bool oneShot){
    double percent = _myInputs["BLOCK__CORE_IN_THRESHOLD"].get<double>()/100.;
    cv::Mat points = _myInputs["BLOCK__CORE_IN_POINTS"].get<cv::Mat>();
    cv::Mat desc = _myInputs["BLOCK__CORE_IN_DESC"].get<cv::Mat>();
    if (points.empty() || desc.empty())
    {
      _myOutputs["BLOCK__CORE_OUT_POINTS"] = points;
      _myOutputs["BLOCK__CORE_OUT_DESC"] = desc;
      return true;//not a problem, just nothing to produce...
    }
    int nbChanels = points.channels();
    if (nbChanels != 1)
      points = points.reshape(1, points.rows);
    if (points.depth() != CV_32F)
      points.convertTo(points, CV_32F);

    int dataSize = 8 * desc.cols;
    bool isBinary = true, isFloat = false;
    if (desc.depth() == CV_16U || desc.depth() == CV_16S)
      dataSize = 16 * desc.cols;
    if (desc.depth() == CV_32S)
      dataSize = 32 * desc.cols;
    if (desc.depth() == CV_32F)
    {
      dataSize = desc.cols;//we just need the number of values, not the number of bytes...
      isBinary = false;
      isFloat = true;
    }
    if (desc.depth() == CV_64F)
    {
      dataSize = desc.cols;//we just need the number of values, not the number of bytes...
      isBinary = false;
      isFloat = false;
    }
    std::vector<double> crit;
    if (isBinary)
    {
      cv::Mat distances = get_distances_binary(desc);
      crit = get_criterions_binary(distances, 0.1, dataSize);
    }
    else
    {
      cv::Mat distances;
      if (isFloat)
        distances = get_distances_float<float>(desc, 32.135f);
      else
        distances = get_distances_float<double>(desc, 32.135f);
      crit = get_criterions_float(distances, 32.135f, dataSize);
    }

    std::vector<size_t> indices = sort_indexes(crit);

    //now filter points:
    int newRows = (int)(points.rows*percent);
    cv::Mat outPoints(cv::Size(points.cols, newRows), points.type());
    cv::Mat outDesc(cv::Size(desc.cols, newRows), desc.type());
    for (int i = 0; i < newRows; i++) {
      int bestIndice = indices[i];
      memcpy(outPoints.ptr<float>(i), points.ptr<float>(bestIndice), points.cols*sizeof(float));

      if (desc.depth() == CV_8U || desc.depth() == CV_8S)
        memcpy(outDesc.ptr<char>(i), desc.ptr<char>(bestIndice), desc.cols*sizeof(char));

      if (desc.depth() == CV_16U || desc.depth() == CV_16S)
        memcpy(outDesc.ptr<short>(i), desc.ptr<short>(bestIndice), desc.cols*sizeof(short));

      if (desc.depth() == CV_32S || desc.depth() == CV_32F)
        memcpy(outDesc.ptr<float>(i), desc.ptr<float>(bestIndice), desc.cols*sizeof(float));

      if (desc.depth() == CV_64F)
        memcpy(outDesc.ptr<double>(i), desc.ptr<double>(bestIndice), desc.cols*sizeof(double));
    }

    _myOutputs["BLOCK__CORE_OUT_POINTS"] = outPoints;
    _myOutputs["BLOCK__CORE_OUT_DESC"] = outDesc;
    return true;
  };
};