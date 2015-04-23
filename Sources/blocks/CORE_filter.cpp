
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
  inline double hamming_distance(char* vals1, char* vals2, int nbVals)
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
  inline double euclidean2_distance(T* vals1, T* vals2, int nbVals)
  {
    double output = 0;
    int i;
    for (i = 0; i < nbVals; i ++) {
      T tmp = vals1[i] - vals2[i];
      output += static_cast<double>(tmp*tmp);
    }
    return output;
  };

  template<typename T>
  cv::Mat get_distances_float_opt(cv::Mat features, float sigma, cv::Mat points, double boostThreshold)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_64FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_64FC1);

    double* tmp = new double[features.cols];
    float* tmpP = new float[points.cols];
    float* ptrP = points.ptr<float>();

    double sigma_carre = 2 * sigma*sigma;

    double* ptr = output.ptr<double>();
    T* val = features.ptr<T>();
    int nbFeatures = features.cols;
    int totalSwitch = 0;
    for (int i = 0; i < output.rows; i++)
    {
      int realAdress = i*output.cols;
      ptr[realAdress + i] = 1;

      T* val1 = &val[i*features.cols];
      int nbCopy = 1;
      for (int j = i + 1; j < output.cols; j++)
      {
        double test = ptr[realAdress + j] = exp(
          -euclidean2_distance(val1, &val[j*nbFeatures], nbFeatures)
          / sigma_carre);
        if (test > boostThreshold)//points are the same!
        {
          if (i + nbCopy < j)
          {
            //switch (i+nbCopy)th line with jth feature:
            memcpy(tmp, &val[j*nbFeatures], sizeof(T)*nbFeatures);
            memcpy(&val[j*nbFeatures], &val[(i + nbCopy)*nbFeatures], sizeof(T)*nbFeatures);
            memcpy(&val[(i + nbCopy)*nbFeatures], tmp, sizeof(T)*nbFeatures);

            //switch also the points
            float* pt1 = &ptrP[j*points.cols];
            float* pt2 = &ptrP[(i + nbCopy)*points.cols];
            memcpy(tmpP, pt1, sizeof(float)*points.cols);
            memcpy(pt1, pt2, sizeof(float)*points.cols);
            memcpy(pt2, tmpP, sizeof(float)*points.cols);
            
            //also switch the value of previously computed distance:
            for (int cpt = 0; cpt <= i; cpt++)
              std::swap(ptr[cpt*output.cols + j], ptr[cpt*output.cols + i + nbCopy]);

          }
          nbCopy++;
        }
      }
      //copy the distances of ith point:
      for (int cpt = 1; cpt < nbCopy; cpt++)
        memcpy(&ptr[realAdress + cpt*output.cols + i + cpt], &ptr[realAdress + i + cpt], sizeof(double)*(output.cols - (i + cpt)));
      
      nbCopy--;
      i += nbCopy;//skip the same points...
      totalSwitch += nbCopy;
    }
    delete[] tmp;
    delete[] tmpP;
    return output;
  }

  cv::Mat get_distances_binary_opt(cv::Mat features, double mu, int nbBits, cv::Mat points, double boostThreshold)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_64FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_64FC1);

    double* tmp = new double[features.cols];
    float* tmpP = new float[points.cols];
    float* ptrP = points.ptr<float>();

    double* ptr = output.ptr<double>();
    char* val = features.ptr<char>();

    int nbFeatures = features.cols;
    int sizeFeatures = features.cols * 8;
    int totalSwitch = 0;
    for (int i = 0; i < output.rows; i++)
    {
      int realAdress = i*output.cols;
      ptr[realAdress + i] = 1;

      char* val1 = &val[i*features.cols];
      int nbCopy = 1;
      for (int j = i + 1; j < output.cols; j++)
      {
        double distTmp = hamming_distance(
          val1, &val[j*features.cols], nbFeatures);

        double test = ptr[realAdress + j] = (pow(mu, (int)distTmp) * pow(1. - mu, (int)(nbBits - distTmp)));

        if (test > boostThreshold)//points are the same!
        {
          if (i + nbCopy < j)
          {
            //switch (i+nbCopy)th line with jth feature:
            memcpy(tmp, &val[j*nbFeatures], nbFeatures);
            memcpy(&val[j*nbFeatures], &val[(i + nbCopy)*nbFeatures], nbFeatures);
            memcpy(&val[(i + nbCopy)*nbFeatures], tmp, nbFeatures);

            //switch also the points
            float* pt1 = &ptrP[j*points.cols];
            float* pt2 = &ptrP[(i + nbCopy)*points.cols];
            memcpy(tmpP, pt1, sizeof(float)*points.cols);
            memcpy(pt1, pt2, sizeof(float)*points.cols);
            memcpy(pt2, tmpP, sizeof(float)*points.cols);

            //also switch the value of previously computed distance:
            for (int cpt = 0; cpt <= i; cpt++)
              std::swap(ptr[cpt*output.cols + j], ptr[cpt*output.cols + i + nbCopy]);
          }
          nbCopy++;
        }
      }
      //copy the distances of ith point:
      for (int cpt = 1; cpt < nbCopy; cpt++)
        memcpy(&ptr[realAdress + cpt*output.cols + i + cpt], &ptr[realAdress + i + cpt], sizeof(double)*(output.cols - (i + cpt)));

      nbCopy--;
      i += nbCopy;//skip the same points...
      totalSwitch += nbCopy;
    }
    delete[] tmp;
    delete[] tmpP;
    return output;
  }

  template<typename T>
  cv::Mat get_distances_float(cv::Mat features, float sigma)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_64FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_64FC1);

    float sigma_carre = 2 * sigma*sigma;

    double* ptr = output.ptr<double>();
    T* val = features.ptr<T>();
    int nbFeatures = features.cols;
    for (int i = 0; i < features.rows; i++)
    {
      int realAdress = i*features.rows;
      ptr[realAdress + i] = 0;//diagonal distance is null...
      T* val1 = &val[i*features.cols];
      for (int j = i + 1; j < features.rows; j++)
      {
        ptr[realAdress + j] = exp(
          -euclidean2_distance(val1, &val[j*features.cols], nbFeatures)
          / sigma_carre);
      }
    }
    return output;
  }

  cv::Mat get_distances_binary(cv::Mat features, double mu, int nbBits)
  {
    static cv::Mat output(cv::Size(features.rows, features.rows), CV_64FC1);
    if (output.rows != features.rows)
      output = Mat(cv::Size(features.rows, features.rows), CV_64FC1);
    output.setTo(0);

    double* ptr = output.ptr<double>();
    char* val = features.ptr<char>();
    int nbFeatures = features.cols;
    for (int i = 0; i < features.rows; i++)
    {
      int realAdress = i*features.rows;
      ptr[realAdress + i] = 0;//diagonal distance is null...
      char* val1 = &val[i*features.cols];
      for (int j = i + 1; j < features.rows; j++)
      {
        double dist = hamming_distance(
          val1, &val[j*features.cols], nbFeatures);

        ptr[realAdress + j] = -log(pow(mu, (int)dist) * pow(1. - mu, (int)(nbBits - dist)) + 1e-256);
      }
    }
    return output;
  }


  std::vector<double> get_criterions_float(cv::Mat distances, double sigma, int D)
  {
    std::vector<double> output;
    for (int i = 0; i < distances.cols; i++)
      output.push_back(0);
    //double divisor = 1.;// (1. / ((distances.rows - 1.) * pow((sigma*sqrt(2 * CV_PI)), D)));

    for (int i = 0; i < distances.cols; i++)
    {
      double* dist_i = distances.ptr<double>(i);
      double crit_i = 0;
      for (int j = i + 1; j < distances.rows; j++)
      {
        output[i] += dist_i[j];
        output[j] += dist_i[j];
      }
    }

    return output;
  };

  std::vector<double> get_criterions_binary(cv::Mat distances)
  {
    std::vector<double> output;
    for (int i = 0; i < distances.rows; i++)
      output.push_back(0);

    for (int i = 0; i < distances.rows; i++)
    {
      double* dist_i = distances.ptr<double>(i);
      for (int j = i + 1; j < distances.rows; j++)
      {
        double crit_i = dist_i[j];
        output[i] += crit_i;
        output[j] += crit_i;
      }
    }
    return output;
  };

  BLOCK_END_INSTANTIATION(CORE_filter, AlgoType::imgProcess, BLOCK__CORE_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(CORE_filter);
  //Add parameters, with following parameters:
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_IN_POINTS", "BLOCK__CORE_IN_POINTS_HELP");
  ADD_PARAMETER(true, Matrix, "BLOCK__CORE_IN_DESC", "BLOCK__CORE_IN_DESC_HELP");
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CORE_IN_THRESHOLD", "BLOCK__CORE_IN_THRESHOLD_HELP", 90.f);
  ADD_PARAMETER_FULL(false, Float, "BLOCK__CORE_IN_OPTIM_THRESHOLD", "BLOCK__CORE_IN_OPTIM_THRESHOLD_HELP", .75f);
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
    double percent = _myInputs["BLOCK__CORE_IN_THRESHOLD"].get<double>() / 100.;
    double percentOpt = _myInputs["BLOCK__CORE_IN_OPTIM_THRESHOLD"].get<double>();
    cv::Mat points = _myInputs["BLOCK__CORE_IN_POINTS"].get<cv::Mat>().clone();
    cv::Mat desc = _myInputs["BLOCK__CORE_IN_DESC"].get<cv::Mat>().clone();
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
      cv::Mat distances;
      if (percentOpt >= 1)
        distances = get_distances_binary(desc, 0.1, dataSize);
      else
        distances = get_distances_binary_opt(desc, 0.1, dataSize, points, percentOpt);
      crit = get_criterions_binary(distances);
    }
    else
    {
      cv::Mat distances;
      if (percentOpt >= 1)
      {
        if (isFloat)
          distances = get_distances_float<float>(desc, 32.135f);
        else
          distances = get_distances_float<double>(desc, 32.135f);
      }
      else
      {
        if (isFloat)
        {
          auto tick = boost::posix_time::microsec_clock::local_time();
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          distances = get_distances_float_opt<float>(desc, 32.135f, points, percentOpt);
          auto now = boost::posix_time::microsec_clock::local_time();
          auto elapsed = (now - tick).total_microseconds();

          tick = boost::posix_time::microsec_clock::local_time();
          cv::Mat distances1 = get_distances_float<float>(desc, 32.135f);
          distances1 = get_distances_float<float>(desc, 32.135f);
          distances1 = get_distances_float<float>(desc, 32.135f);
          distances1 = get_distances_float<float>(desc, 32.135f);
          distances1 = get_distances_float<float>(desc, 32.135f);
          distances1 = get_distances_float<float>(desc, 32.135f);
          now = boost::posix_time::microsec_clock::local_time();
          auto elapsed1 = (now - tick).total_microseconds();
          std::cout << elapsed << " sans optim : " << elapsed1 << " gain : " << elapsed * 100 / elapsed1 << std::endl;
          distances1.ptr<double>();
        }
        else
          distances = get_distances_float_opt<double>(desc, 32.135f, points, percentOpt);
      }
      crit = get_criterions_float(distances, 32.135f, dataSize);
    }

    std::vector<size_t> indices = sort_indexes(crit);

    //now filter points:
    int newRows = (int)(points.rows*percent);
    cv::Mat outPoints(cv::Size(points.cols, newRows), points.type());
    cv::Mat outDesc(cv::Size(desc.cols, newRows), desc.type());
    for (int i = 0; i <newRows; i++) {
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