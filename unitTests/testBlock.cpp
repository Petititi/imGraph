
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

using namespace charliesoft;
using namespace std;
using namespace boost;
using namespace cv;

struct MyConfig {
  MyConfig()   {}
  ~MyConfig()  { ProcessManager::releaseInstance(); Internationalizator::releaseInstance(); }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE(MyConfig);

BOOST_AUTO_TEST_SUITE( Test_Block )

BOOST_AUTO_TEST_CASE(Accumulator)
{
  cv::Mat img1 = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/0001.jpg", IMREAD_GRAYSCALE);
  cv::Mat img2 = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/0002.jpg", IMREAD_GRAYSCALE);
  BOOST_REQUIRE_MESSAGE(!img1.empty() && !img2.empty(), "img loading");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__ACCUMULATOR_NAME");
  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Accumulator block creation");

  ParamValue* outImg = myBlock->getParam("BLOCK__ACCUMULATOR_OUT_IMAGE", false);

  myBlock->setParam("BLOCK__ACCUMULATOR_IN_IMAGE", img1);
  myBlock->setParam("BLOCK__ACCUMULATOR_IN_NB_HISTORY", 2);
  myBlock->setExecuteOnlyOnce(true);

  (*myBlock)();
  myBlock->run(true);//as release is called, the history is emptied... Run it the classic way.

  Mat imgFirst = outImg->get<cv::Mat>().clone();
  BOOST_CHECK_MESSAGE(img1.size() == imgFirst.size(), "First accumulation");
  bool isSame = true;
  for (int i = 0; i < img1.rows; i++)
  {
    uchar* line1 = img1.ptr<uchar>(i);
    uchar* line2 = imgFirst.ptr<uchar>(i);
    for (int j = 0; j < img1.cols; j++)
      isSame &= line1[j] == line2[j];
  }
  BOOST_CHECK_MESSAGE(isSame, "First accumulation don't corrupt img");

  myBlock->setParam("BLOCK__ACCUMULATOR_IN_IMAGE", img2);
  myBlock->run(true);

  Mat imgMerge = outImg->get<cv::Mat>().clone();

  myBlock->setParam("BLOCK__ACCUMULATOR_IN_IMAGE", img1);
  myBlock->run(true);//as we only have 2 img to accumulate, this will not change the output:

  Mat imgMergeFinal = outImg->get<cv::Mat>().clone();

  bool diff1 = false, diff2 = false;
  isSame = true;
  for (int i = 0; i < imgMerge.rows; i++)
  {
    uchar* lineI1 = img1.ptr<uchar>(i);
    uchar* lineI2 = img2.ptr<uchar>(i);
    uchar* line1 = imgMerge.ptr<uchar>(i);
    uchar* line2 = imgMergeFinal.ptr<uchar>(i);
    for (int j = 0; j < imgMerge.cols; j++)
    {
      diff1 |= line1[j] != lineI1[j];
      diff2 |= line1[j] != lineI2[j];
      isSame &= line1[j] == line2[j];
    }
  }
  BOOST_CHECK_MESSAGE(isSame && diff1 && diff2, "Accumulation loop");

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(AddBlock)
{
  cv::Mat img1 = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/0001.jpg", IMREAD_GRAYSCALE);
  cv::Mat img2 = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/0002.jpg", IMREAD_GRAYSCALE);
  BOOST_REQUIRE_MESSAGE(!img1.empty() && !img2.empty(), "img loading");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__ADD_NAME");
  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Accumulator block creation");

  myBlock->setExecuteOnlyOnce(true);

  ParamValue* param1 = myBlock->getParam("BLOCK__ADD_IN_PARAM1", true);
  ParamValue* param2 = myBlock->getParam("BLOCK__ADD_IN_PARAM2", true);
  ParamValue* result = myBlock->getParam("BLOCK__ADD_OUTPUT", false);
  Mat imgSum;

  *param1 = 12;
  *param2 = 10;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<int>() == 22, "simple sum (int)");
  *param2 = Scalar(64, 128, 64);
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<Scalar>() == Scalar(12 + 64, 128 + 12, 12 + 64, 12), "simple sum (Scalar)");
  *param2 = 10.5;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<double>() == 22.5, "simple sum (int*double)");
  *param2 = "This is a test";
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "12This is a test", "simple sum (String)");
  *param2 = img1;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();
  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<uchar>(51, 51) == 12 && imgSum.at<uchar>(0, 0) == 255, "simple sum (Mat)");


  *param1 = 12.5;
  *param2 = 10.5;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<double>() == 23., "double sum (double)");
  *param2 = 10;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<double>() == 22.5, "double sum (int)");
  *param2 = Scalar(64, 128, 64);
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<Scalar>() == Scalar(12.5 + 64, 128 + 12.5, 12.5 + 64, 12.5), "double sum (Scalar)");
  *param2 = "This is a test";
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "12.5This is a test", "double sum (String)");
  *param2 = img1;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();
  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<uchar>(51, 51) == 12 && imgSum.at<uchar>(0, 0) == 255, "double sum (Mat)");


  *param1 = Scalar(128, 64, 128.5, 5);
  *param2 = 10;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<Scalar>() == Scalar(10 + 128, 10 + 64, 10 + 128.5, 15), "Scalar sum (int)");
  *param2 = 10.5;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<Scalar>() == Scalar(10.5 + 128, 10.5 + 64, 10.5 + 128.5, 10.5 + 5), "Scalar sum (double)");
  *param2 = Scalar(64, 128);
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<Scalar>() == Scalar(128 + 64, 64 + 128, 128.5, 5), "Scalar sum (Scalar)");
  *param2 = "This is a test";
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "[128, 64, 128.5, 5]This is a test", "Scalar sum (String)");
  *param2 = img1;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();
  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<uchar>(51, 51) == 128 && imgSum.at<uchar>(0, 0) == 255, "Scalar sum (Mat)");


  *param1 = "AnotherTest";
  *param2 = 10;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "AnotherTest10", "String sum (int)");
  *param2 = 10.5;
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "AnotherTest10.5", "String sum (double)");
  *param2 = Scalar(64, 128);
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "AnotherTest[64, 128, 0, 0]", "String sum (Scalar)");
  *param2 = " This is a test";
  (*myBlock)();
  BOOST_CHECK_MESSAGE(result->get<std::string>() == "AnotherTest This is a test", "String sum (String)");

  *param1 = img1;
  *param2 = 12;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();
  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<uchar>(51, 51) == 12 && imgSum.at<uchar>(0, 0) == 255, "Image sum (int)");

  *param1 = MatrixConvertor::convert(img1, CV_64F);
  *param2 = 12.5;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();

  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<double>(51, 51) == 12.5 && imgSum.at<double>(0, 0) == 267.5, "Image sum (double)");

  *param1 = MatrixConvertor::convert(img1, CV_8UC3);
  *param2 = Scalar(128,255,128);
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();

  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<Vec3b>(51, 51)[0] == 128 && imgSum.at<Vec3b>(51, 51)[1] == 255 &&
    imgSum.at<Vec3b>(0, 0)[0] == 255 && imgSum.at<Vec3b>(0, 0)[1] == 255, "Image sum (color)");

  *param1 = img1;
  *param2 = img2;
  (*myBlock)();
  imgSum = result->get<cv::Mat>().clone();

  BOOST_CHECK_MESSAGE(img1.size() == imgSum.size() &&
    imgSum.at<uchar>(115, 127) == 152 && imgSum.at<uchar>(0, 0) == 255, "Image sum (img)");

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(Cropping)
{
  cv::Mat img = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/color.jpg");
  BOOST_REQUIRE_MESSAGE(!img.empty(), "img loading");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__CROP_NAME");
  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Crop block creation");

  myBlock->setExecuteOnlyOnce(true);
  ParamValue* imgParam = myBlock->getParam("BLOCK__CROP_IN_IMAGE", true);

  ParamValue* width = myBlock->getParam("BLOCK__CROP_WIDTH", true);
  ParamValue* height = myBlock->getParam("BLOCK__CROP_HEIGHT", true);

  ParamValue* posX = myBlock->getParam("BLOCK__CROP_IN_X", true);
  ParamValue* posY = myBlock->getParam("BLOCK__CROP_IN_Y", true);

  ParamValue* output = myBlock->getParam("BLOCK__CROP_OUT_IMAGE", false);

  *imgParam = img;

  *width = 128;
  *height = 256;

  (*myBlock)();//run block...

  cv::Mat outMat = output->get<cv::Mat>();
  BOOST_CHECK_MESSAGE(outMat.cols == 128 && outMat.rows == 256, "Cropping image");

  *posX = 10;
  *posY = 30;

  (*myBlock)();//run block...

  outMat = output->get<cv::Mat>();
  Vec3b color = *outMat.ptr<Vec3b>();
  BOOST_CHECK_MESSAGE(outMat.cols == 128 && outMat.rows == 256 &&
    color[0] == 0 && color[1] == 250 && color[2] == 5, "Cropping and origin change image");

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(InputLoader)
{
  string inputDirectory = string(SOURCE_DIRECTORY) + "/unitTests/data/";

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__INPUT_NAME");

  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "input loader creation");

  ParamValue* typeInput = myBlock->getParam("BLOCK__INPUT_IN_INPUT_TYPE", true);
  ParamValue* inFolder = myBlock->getParam("BLOCK__INPUT_IN_INPUT_TYPE.Folder.input folder", true);
  ParamValue* isColor = myBlock->getParam("BLOCK__INPUT_IN_COLOR", true);
  ParamValue* isGrey = myBlock->getParam("BLOCK__INPUT_IN_GREY", true);
  ParamValue* width = myBlock->getParam("BLOCK__INPUT_INOUT_WIDTH", true);
  ParamValue* height = myBlock->getParam("BLOCK__INPUT_INOUT_HEIGHT", true);

  ParamValue* output = myBlock->getParam("BLOCK__INPUT_OUT_IMAGE", false);

  *typeInput = 2;//folder...
  *inFolder = inputDirectory;

  myBlock->setExecuteOnlyOnce(true);
  (*myBlock)();//run block...

  cv::Mat img = output->get<cv::Mat>();
  BOOST_REQUIRE_MESSAGE(!img.empty(), "img loading");

  //try to get image in b&w:
  *isColor = false;
  *isGrey = true;
  (*myBlock)();//run block...
  img = output->get<cv::Mat>();
  BOOST_CHECK_MESSAGE(img.channels() == 1, "B&W img");

  //try to get image in color:
  *isGrey = false;
  *isColor = true;
  (*myBlock)();//run block...
  img = output->get<cv::Mat>();
  BOOST_CHECK_MESSAGE(img.channels() == 3, "Color img");

  //change the size of image:
  *width = 512;
  *height = 254;

  (*myBlock)();//run block...
  img = output->get<cv::Mat>();
  BOOST_CHECK_MESSAGE(img.rows == 254 && img.cols == 512, "Resized img");

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(OutputBlock)
{
  cv::Mat img = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/0001.jpg");
  BOOST_REQUIRE_MESSAGE(!img.empty(), "img loading");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__OUTPUT_NAME");

  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Display image creation");

  ParamValue* imgInput = myBlock->getParam("BLOCK__OUTPUT_IN_IMAGE", true);
  ParamValue* winName = myBlock->getParam("BLOCK__OUTPUT_IN_WIN_NAME", true);

  QString winNameStr = "This is a test!";
  *imgInput = img;//img to show...
  *winName = winNameStr.toStdString();

  myBlock->setExecuteOnlyOnce(true);
  (*myBlock)();//run block...

  //try to find window:
  MatrixViewer* window = NULL;

  QWidgetList winList = QApplication::topLevelWidgets();
  for (auto* widget : winList)
  {
    if (widget->isWindow() && !widget->parentWidget())//is a window without parent
    {
      if (MatrixViewer* w = dynamic_cast<MatrixViewer*>(widget))
      {
        if (w->windowTitle() == winNameStr)
        {
          window = w;
          break;
        }
      }
    }
  }
  BOOST_CHECK_MESSAGE(window != NULL, "Find window");
  delete window;

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(StringCreation)
{
  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__STRING_CREATION_NAME");

  BOOST_CHECK_MESSAGE(myBlock != NULL, "String manipulation block creation");

  ParamValue* input1 = myBlock->addNewInput(new ParamDefinition(false, charliesoft::String, "firstParam", "firstParam"));
  ParamValue* input2 = myBlock->addNewInput(new ParamDefinition(false, Int, "secondParam", "secondParam"));
  ParamValue* input3 = myBlock->addNewInput(new ParamDefinition(false, Float, "thirdParam", "thirdParam"));
  ParamValue* input4 = myBlock->addNewInput(new ParamDefinition(false, FilePath, "fourthParam", "fourthParam"));

  *input1 = "param1";
  *input2 = 128;
  *input3 = 0.5;
  *input4 = "c:/test.jpg";

  ParamValue* regex = myBlock->getParam("BLOCK__STRING_CREATION_IN_REGEX", true);
  ParamValue* output = myBlock->getParam("BLOCK__STRING_CREATION_OUT", false);

  myBlock->setExecuteOnlyOnce(true);

  *regex = "This is a simple test";
  (*myBlock)();//run block...
  BOOST_CHECK_MESSAGE(output->toString() == "This is a simple test", "Simple test");

  *regex = "This is a bit\\%more complicated é\"'€";
  (*myBlock)();//run block...
  BOOST_CHECK_MESSAGE(output->toString() == "This is a bit\\%more complicated é\"'€", "Simple test1");

  *regex = "Remplacing %1%";
  (*myBlock)();//run block...
  BOOST_CHECK_MESSAGE(output->toString() == "Remplacing param1", "Test remplacing string");

  *regex = "Full substitution \\%2%2%%3%\\";
  (*myBlock)();//run block...
  string tet = output->toString();
  BOOST_CHECK_MESSAGE(output->toString() == "Full substitution \\%21280.5\\", "Substitution");

  *regex = "%4% is a file path";
  (*myBlock)();//run block...
  BOOST_CHECK_MESSAGE(output->toString() == "c:/test.jpg is a file path", "Test filepath");

  *regex = "A special value : %n%";
  (*myBlock)();//run block...
  BOOST_CHECK_MESSAGE(output->toString() == "A special value : 1", "Test nb rendering");

  delete myBlock;
}

void testImgSave(Block* myBlock, Mat img, string imName, string ext)
{
  ParamValue* imgToSave = myBlock->getParam("BLOCK__IMWRITE_IN_IMAGE", true);
  ParamValue* fileName = myBlock->getParam("BLOCK__IMWRITE_IN_FILENAME", true);
  ParamValue* quality = myBlock->getParam("BLOCK__IMWRITE_IN_QUALITY", true);

  *imgToSave = img;
  vector<uintmax_t> sizes;

  for (int i = 0; i <= 100; i += 25)
  {
    *fileName = imName + lexical_cast<std::string>(i)+ext;
    *quality = i;
    (*myBlock)();//run block...
    BOOST_CHECK_MESSAGE(boost::filesystem::exists(fileName->toString()), "image writing: " + lexical_cast<std::string>(i)+ext);
    sizes.push_back(boost::filesystem::file_size(fileName->toString()));
    if (sizes.size()>1)
      BOOST_CHECK_MESSAGE(sizes[sizes.size() - 2] <= sizes[sizes.size() - 1], "image size: " + lexical_cast<std::string>(i)+ext);
    boost::filesystem::path imgPath(fileName->toString());
    boost::filesystem::remove(fileName->toString());
  }
}

BOOST_AUTO_TEST_CASE(ImgWriter)
{
  cv::Mat img = imread(string(SOURCE_DIRECTORY) + "/unitTests/data/color.jpg");
  BOOST_REQUIRE_MESSAGE(!img.empty(), "img loading");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__IMWRITE_NAME");
  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Img writer block creation");

  myBlock->setExecuteOnlyOnce(true);

  testImgSave(myBlock, img, "./test", ".jpg");
  testImgSave(myBlock, img, "./test", ".png");
  testImgSave(myBlock, img, "./test", ".bmp");
  testImgSave(myBlock, img, "./test", ".ppm");

  delete myBlock;
}

BOOST_AUTO_TEST_CASE(DrawLine)
{
  Block* matrixCreator = _PROCESS_MANAGER->createAlgoInstance("BLOCK__CREATEMATRIX_NAME");
  BOOST_REQUIRE_MESSAGE(matrixCreator != NULL, "Matrix creator block creation");

  Block* myBlock = _PROCESS_MANAGER->createAlgoInstance("BLOCK__LINEDRAWER_NAME");
  BOOST_REQUIRE_MESSAGE(myBlock != NULL, "Line drawer creation");

  ParamValue* outLine = myBlock->getParam("BLOCK__LINEDRAWER_OUT_IMAGE", false);

  myBlock->setExecuteOnlyOnce(true);
  matrixCreator->setExecuteOnlyOnce(true);

  ParamValue* imgCreated = matrixCreator->getParam("BLOCK__CROP_IN_IMAGE", false);
  *matrixCreator->getParam("BLOCK__CREATEMATRIX_IN_NBCHANNEL", true) = 3;//Color image
  *matrixCreator->getParam("BLOCK__CREATEMATRIX_IN_INIT", true) = 6;//random uniform
  (*matrixCreator)();//create random img...

  Mat imgRandom = matrixCreator->getParam("BLOCK__CREATEMATRIX_OUT_IMAGE", false)->get<cv::Mat>();
  BOOST_CHECK_MESSAGE(imgRandom.cols == 640 && imgRandom.rows == 480 && imgRandom.channels() == 3, "Random image creation");

  vector<Vec4i> points;
  points.push_back(Vec4i(0, 0, 15, 15));//First line...
  points.push_back(Vec4i(30, 0, 30, 30));//second line...

  myBlock->setParam("BLOCK__LINEDRAWER_IN_LINES", Mat(points));
  myBlock->setParam("BLOCK__LINEDRAWER_IN_COLOR", Scalar(255, 0, 0));
  myBlock->setParam("BLOCK__LINEDRAWER_IN_IMAGE", imgRandom);
  myBlock->setParam("BLOCK__LINEDRAWER_IN_SIZE", 1);

  (*myBlock)();//run block...

  Mat withLines = outLine->get<cv::Mat>();

  //verify img:
  bool isBlue = true, isRandom = false;
  for (int i = 0; i < 15; i++)
  {
    Vec3b& color = withLines.at<Vec3b>(i, i);
    isBlue &= color[0] == 255 && color[1] == 0 && color[2] == 0;
    color = withLines.at<Vec3b>(i * 2, 30);
    isBlue &= color[0] == 255 && color[1] == 0 && color[2] == 0;
    color = withLines.at<Vec3b>(i, i + 1);
    isRandom |= color[0] != 255 && color[1] != 0 && color[2] != 0;
    color = withLines.at<Vec3b>(i * 2, 31);
    isRandom |= color[0] != 255 && color[1] != 0 && color[2] != 0;
  }
  BOOST_CHECK_MESSAGE(isRandom, "Test random pixels");
  BOOST_CHECK_MESSAGE(isBlue, "Test blue lines (1px large)");

  points.clear();
  points.push_back(Vec4i(100, 100, 115, 115));//First line...
  points.push_back(Vec4i(130, 0, 130, 30));//second line...

  myBlock->setParam("BLOCK__LINEDRAWER_IN_SIZE", 3);//line 3 pix large
  myBlock->setParam("BLOCK__LINEDRAWER_IN_COLOR", Scalar(0, 0, 255));

  (*myBlock)();//run block...

  withLines = outLine->get<cv::Mat>();

  //verify img:
  bool isRed = true;
  isRandom = false;
  for (int i = 0; i < 15; i++)
  {
    Vec3b& color = withLines.at<Vec3b>(100 + i + 1, 100 + i);
    isRed &= color[0] == 0 && color[1] == 0 && color[2] == 255;
    color = withLines.at<Vec3b>(100 + i, 100 + i + 1);
    isRed &= color[0] == 0 && color[1] == 0 && color[2] == 255;

    color = withLines.at<Vec3b>(i * 2, 131);
    isRed &= color[0] == 0 && color[1] == 0 && color[2] == 255;
    color = withLines.at<Vec3b>(i * 2, 129);
    isRed &= color[0] == 0 && color[1] == 0 && color[2] == 255;

    color = withLines.at<Vec3b>(100 + i + 3, 100 + i);
    isRandom |= color[0] != 255 && color[1] != 0 && color[2] != 0;
    color = withLines.at<Vec3b>(i * 2, 126);
    isRandom |= color[0] != 255 && color[1] != 0 && color[2] != 0;
  }
  BOOST_CHECK_MESSAGE(isRandom, "Test random pixels");
  BOOST_CHECK_MESSAGE(isRed, "Test red lines (3px large)");

  delete myBlock;
  delete matrixCreator;
}

BOOST_AUTO_TEST_SUITE_END()