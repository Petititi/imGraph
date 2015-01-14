#ifndef _BLOCK_ITERATION_DEF_H
#define _BLOCK_ITERATION_DEF_H
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <vector>
#include <opencv2/opencv.hpp>
//#include <opencv2/nonfree.hpp>
//#include <opencv2/features2d.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "SubBlock.h"

namespace charliesoft
{

  class ForBlock :public SubBlock
  {
    friend class charliesoft::ProcessManager;

    static bool addedToList;

    virtual bool run(bool oneShot = false);
  public:
    ForBlock();

    virtual boost::property_tree::ptree getXML();
    virtual void initFromXML(boost::property_tree::ptree* tree,
      std::vector < std::pair<ParamValue*, unsigned int> >& toUpdate,
      std::map<unsigned int, ParamValue*>& addressesMap,
      std::vector<ConditionOfRendering*>& condToUpdate);
  };

};

#endif