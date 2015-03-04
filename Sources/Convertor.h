#ifndef _MY_CONVERTOR_HEADER_
#define _MY_CONVERTOR_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif

#include <opencv2/core.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "blocks/ParamValue.h"

namespace charliesoft
{
  namespace MatrixConvertor
  {
    cv::Mat convert(cv::Mat src, int outputType);
    cv::Mat adjustChannels(cv::Mat src, int nbChannels, bool duplicate = true);
    std::string printElem(cv::Mat src, int x, int y, int channel = 0);
    double getElem(cv::Mat src, int x, int y, int channel);
  };
  namespace StringConvertor
  {
    std::string regExExpend(std::string input, std::initializer_list<ParamValue*> list = {});
    std::string regExExpend(std::string input, std::vector<ParamValue*> list = {},
      std::map<std::string, ParamValue*> valuesToMatch = std::map<std::string, ParamValue*>());
  };
}

#endif