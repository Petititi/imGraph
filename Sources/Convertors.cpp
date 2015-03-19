#include "Convertor.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4190)
#endif

#include <opencv2/imgproc.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace cv;
using namespace boost;

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
        Mat *tmp = new Mat[nbChannelsSrc];
        split(src, tmp);

        Mat *tmp1 = new Mat[nbChannels];
        for (int i = 0; i < nbChannelsSrc && i<nbChannels; i++)
          tmp1[i] = tmp[i].clone();

        if (duplicate)
          newChannel = tmp[nbChannelsSrc-1];
        else
          newChannel = Mat::zeros(tmp[0].size(), tmp[0].type());


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

  namespace StringConvertor
  {
    std::string regExExpend(std::string input, std::initializer_list<ParamValue*> list)
    {
      std::vector<ParamValue*> tmpList;

      for (auto elem : list) tmpList.push_back(elem);

      return regExExpend(input, tmpList);
    }

    std::string regExExpend(std::string input, std::vector<ParamValue*> tmpList,
      std::map<std::string, ParamValue*> valuesToMatch)
    {
      //first replace each %1%,¨%2%, ... %n% with corresponding value:
      size_t p = input.find_first_of('%');
      size_t prevPos = 0;
      std::string finalString = "";
      while (p != std::string::npos)
      {
        finalString += input.substr(prevPos, (p - prevPos));
        bool isParameter = true;
        if (p > 0)
          isParameter = input[p - 1] != '\\';

        prevPos = p + 1;
        if (isParameter)
        {
          p = input.find_first_of('%', prevPos);
          if (p != std::string::npos)
          {
            std::string number = input.substr(prevPos, (p - prevPos));
            try
            {
              size_t convNum = lexical_cast<int>(number);
              if (tmpList.size() >= convNum && convNum > 0)
              {
                std::string newValue = tmpList[convNum - 1]->toString();
                finalString += newValue;
              }
            }
            catch (bad_lexical_cast &)
            {
              finalString += "%" + number + "%";//we keep the bad val
            }
            prevPos = p + 1;
          }
        }
        else
          finalString += input[p];
        p = input.find_first_of('%', prevPos);
      }
      finalString += input.substr(prevPos);

      //now replace every valuesToMatch found:
      for (auto& valToMatch : valuesToMatch)
      {
        std::string toFind = valToMatch.first;
        std::string toReplace = "";
        if (valToMatch.second != NULL)
          toReplace = valToMatch.second->toString();
        boost::algorithm::replace_all(finalString, toFind, toReplace);
      }
      return finalString;
    }
  };
}