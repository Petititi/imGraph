#include "MatrixConvertor.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif

#include <opencv2/imgproc.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace cv;

namespace charliesoft
{
  namespace MatrixConvertor
  {
    cv::Mat convert(cv::Mat src, int outputType)
    {
      Mat output = adjustChannels(src, CV_MAT_CN(outputType));
      if (output.depth() != CV_MAT_DEPTH(outputType))
      {
        if (outputType == CV_8UC3 && output.channels() == 3)
        {
          normalize(output, output, 0, 255, NORM_MINMAX);
          output.convertTo(output, outputType);
          cvtColor(output, output, COLOR_HSV2RGB);
        }
        else
          output.convertTo(output, outputType);
      }
      return output;
    }

    cv::Mat adjustChannels(cv::Mat src, int nbChannels, bool duplicate)
    {
      int nbChannelsSrc = src.channels();
      if (nbChannelsSrc == nbChannels)
        return src.clone();
      Mat output, newChannel;
      //particular conversions:
      switch (nbChannelsSrc)
      {
      case 1:
      {
        if (nbChannels == 3 && src.type() == CV_8UC1)
        {
          cvtColor(src, output, COLOR_GRAY2RGB);
        }
        else
        {
          if (duplicate)
            newChannel = src;
          else
            newChannel = Mat::zeros(src.size(), src.type());
          Mat *tmp = new Mat[nbChannels];
          for (int i = 0; i < nbChannels; i++)
            tmp[i] = newChannel.clone();
          cv::merge(tmp, nbChannels, output);
          delete[] tmp;
        }
        break;
      }
      case 2:
      {
        Mat *tmp = new Mat[2];
        split(src, tmp);
        if (nbChannels == 1)
        {
          output = tmp[0].clone();
        }
        else
        {
          Mat *tmp1 = new Mat[nbChannels];
          tmp1[0] = tmp[0].clone();
          tmp1[1] = tmp[1].clone();
          if (nbChannels == 3)
          {
            cv::Scalar tmpMean = mean(src);
            double meanVal = (tmpMean[0] + tmpMean[1]) / 2.;
            newChannel = Mat::ones(tmp[1].size(), tmp[1].type())*meanVal;
          }
          else
          {
            if (duplicate)
              newChannel = tmp[1];
            else
              newChannel = Mat::zeros(tmp[1].size(), tmp[1].type());
          }
          for (int i = 2; i < nbChannels; i++)
            tmp1[i] = newChannel.clone();
          cv::merge(tmp1, nbChannels, output);
          delete[] tmp1;
        }
        delete[] tmp;
        break;
      }
      case 3:
      {
        if (nbChannels == 1)
        {
          cvtColor(src, output, COLOR_RGB2GRAY);
          break;
        }
      }
      default:
      {
        Mat *tmp = new Mat[3];
        split(src, tmp);

        if (duplicate)
          newChannel = tmp[2];
        else
          newChannel = Mat::zeros(tmp[2].size(), tmp[2].type());

        Mat *tmp1 = new Mat[nbChannels];
        for (int i = 0; i < nbChannelsSrc; i++)
          tmp1[i] = tmp[i].clone();
        for (int i = nbChannelsSrc; i < nbChannels; i++)
          tmp1[i] = newChannel.clone();
        cv::merge(tmp1, nbChannels, output);
        delete[] tmp1;
        delete[] tmp;
        break;
      }
      }
      return output;
    };

    std::string printElem(cv::Mat src, int x, int y, int channel)
    {
      std::stringstream output;
      switch (src.depth())
      {
      case CV_8U:
        output << (int)src.ptr<uchar>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_8S:
        output << (int)src.ptr<char>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_16U:
        output << (int)src.ptr<ushort>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_16S:
        output << (int)src.ptr<short>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_32S:
        output << src.ptr<int>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_32F:
        output << src.ptr<float>()[y*src.step1() + x*src.channels() + channel];
        break;
      case CV_64F:
        output << src.ptr<double>()[y*src.step1() + x*src.channels() + channel];
        break;
      default:
        break;
      }
      std::string r_output = output.str();
      return r_output;
    }

    double getElem(cv::Mat src, int x, int y, int channel)
    {
      switch (src.depth())
      {
      case CV_8U:
        return src.ptr<uchar>()[y*src.step1() + x*src.channels() + channel];
      case CV_8S:
        return src.ptr<char>()[y*src.step1() + x*src.channels() + channel];
      case CV_16U:
        return src.ptr<ushort>()[y*src.step1() + x*src.channels() + channel];
      case CV_16S:
        return src.ptr<short>()[y*src.step1() + x*src.channels() + channel];
      case CV_32S:
        return src.ptr<int>()[y*src.step1() + x*src.channels() + channel];
      case CV_32F:
        return src.ptr<float>()[y*src.step1() + x*src.channels() + channel];
      case CV_64F:
        return src.ptr<double>()[y*src.step1() + x*src.channels() + channel];
      default:
        break;
      }
      return 0;
    }
  }
}