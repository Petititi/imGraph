
#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4244 4275 4800)
#endif
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/test/unit_test.hpp>
#include "QApplication"
#include "boost/filesystem/path_traits.hpp"
#include "boost/filesystem/operations.hpp"

#include <iostream>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "listOfBlocks.h"
#include "Block.h"
#include "ProcessManager.h"
#include "view/MatrixViewer.h"
#include "Convertor.h"
#include "Graph.h"

using namespace charliesoft;
using namespace std;
using namespace boost;
using namespace cv;

struct MyConfig {
  MyConfig()   {}
  ~MyConfig()  { ProcessManager::releaseInstance(); Internationalizator::releaseInstance();}
};

namespace cv{
  CVAPI(int) cvHaveImageReader(const char* filename);
}
//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE(MyConfig);

BOOST_AUTO_TEST_SUITE( Test_Graph )

BOOST_AUTO_TEST_CASE(SimpleGraph)
{
  boost::filesystem::path dir("tempdir");
  BOOST_CHECK_MESSAGE(boost::filesystem::create_directory(dir), "Temporary folder creation");
  string srcFiles = string(SOURCE_DIRECTORY) + "/unitTests/data/";

  //open file -> outputName generation -> save output

  GraphOfProcess graph;
  Block* inputBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__INPUT_NAME");
  BOOST_CHECK_MESSAGE(inputBlock != NULL, "input block creation");
  graph.addNewProcess(inputBlock);
  inputBlock->setParam("BLOCK__INPUT_IN_LOOP", 1);//only 1 iteration...
  inputBlock->setParam("BLOCK__INPUT_IN_INPUT_TYPE", 2);//folder
  inputBlock->setParam("BLOCK__INPUT_IN_INPUT_TYPE.Folder.input folder", srcFiles);

  Block* stringCreation = _PROCESS_MANAGER->createAlgoInstance("BLOCK__STRING_CREATION_NAME");
  BOOST_CHECK_MESSAGE(stringCreation != NULL, "String manipulation block creation");
  graph.addNewProcess(stringCreation);
  stringCreation->setParam("BLOCK__STRING_CREATION_IN_REGEX", "./tempdir/img%n%.jpg");//folder

  Block* imgWriter = _PROCESS_MANAGER->createAlgoInstance("BLOCK__IMWRITE_NAME");
  BOOST_CHECK_MESSAGE(imgWriter != NULL, "Image writer block creation");
  graph.addNewProcess(imgWriter);

  //now make links:
  graph.createLink(inputBlock, "BLOCK__INPUT_OUT_IMAGE", imgWriter, "BLOCK__IMWRITE_IN_IMAGE");
  graph.createLink(stringCreation, "BLOCK__STRING_CREATION_OUT", imgWriter, "BLOCK__IMWRITE_IN_FILENAME");

  graph.run();

  graph.waitUntilEnd(15000);

  boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(dir);
  int nbFiles = 0;
  while (iter != boost::filesystem::directory_iterator())
  {
    string name_of_file = iter->path().string();
    if (cv::cvHaveImageReader((const char*)name_of_file.c_str()))
    { 
      boost::filesystem::remove(name_of_file);
      nbFiles++;
    }
    iter++;
  }
  boost::filesystem::remove("tempdir");
  boost::filesystem::directory_iterator iterSrc = boost::filesystem::directory_iterator(srcFiles);
  int nbFilesSrc = 0;
  while (iterSrc != boost::filesystem::directory_iterator())
  {
    string name_of_file = iterSrc->path().string();
    if (cv::cvHaveImageReader((const char*)name_of_file.c_str()))
      nbFilesSrc++;
    iterSrc++;
  }
  BOOST_CHECK_MESSAGE(nbFilesSrc == nbFiles, "Files created");
}

BOOST_AUTO_TEST_SUITE_END()