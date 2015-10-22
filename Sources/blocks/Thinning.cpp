#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <opencv2/imgproc.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"
#include "ParamValidator.h"
#include "OpenCV_filter.h"
#include "Convertor.h"
using namespace charliesoft;
using std::vector;
using boost::lexical_cast;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(ThinningBlock);
  //You can add methods, re implement needed functions... 
  BLOCK_END_INSTANTIATION(ThinningBlock, AlgoType::mathOperator, BLOCK__THINNING_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(ThinningBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(toBeLinked, Matrix, "BLOCK__THINNING_IN_IMAGE", "BLOCK__THINNING_IN_IMAGE_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(ThinningBlock);
  ADD_PARAMETER(toBeLinked, AnyType, "BLOCK__THINNING_OUT_IMAGE", "BLOCK__THINNING_OUT_IMAGE_HELP");//output type is defined by inputs
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(ThinningBlock);
  END_BLOCK_PARAMS();

  ThinningBlock::ThinningBlock() :Block("BLOCK__THINNING_NAME", true){
    _myInputs["BLOCK__THINNING_IN_IMAGE"].addValidator({ new ValNeeded() });
  };


  using namespace std;
  using namespace cv;

  void nonMaximaSuppression(const Mat& src, const int sz, Mat& dst, const Mat mask = Mat()) {

    // initialise the block mask and destination
    const int M = src.rows;
    const int N = src.cols;
    const bool masked = !mask.empty();
    Mat block = 255 * Mat_<uint8_t>::ones(Size(2 * sz + 1, 2 * sz + 1));
    dst = Mat_<uint8_t>::zeros(src.size());

    // iterate over image blocks
    for (int m = 0; m < M; m += sz + 1) {
      for (int n = 0; n < N; n += sz + 1) {
        Point  ijmax;
        double vcmax, vnmax;

        // get the maximal candidate within the block
        Range ic(m, min(m + sz + 1, M));
        Range jc(n, min(n + sz + 1, N));
        minMaxLoc(src(ic, jc), NULL, &vcmax, NULL, &ijmax, masked ? mask(ic, jc) : noArray());
        Point cc = ijmax + Point(jc.start, ic.start);

        // search the neighbours centered around the candidate for the true maxima
        Range in(max(cc.y - sz, 0), min(cc.y + sz + 1, M));
        Range jn(max(cc.x - sz, 0), min(cc.x + sz + 1, N));

        // mask out the block whose maxima we already know
        Mat_<uint8_t> blockmask;
        block(Range(0, in.size()), Range(0, jn.size())).copyTo(blockmask);
        Range iis(ic.start - in.start, min(ic.start - in.start + sz + 1, in.size()));
        Range jis(jc.start - jn.start, min(jc.start - jn.start + sz + 1, jn.size()));
        blockmask(iis, jis) = Mat_<uint8_t>::zeros(Size(jis.size(), iis.size()));

        minMaxLoc(src(in, jn), NULL, &vnmax, NULL, &ijmax, masked ? mask(in, jn).mul(blockmask) : blockmask);
        Point cn = ijmax + Point(jn.start, in.start);

        // if the block centre is also the neighbour centre, then it's a local maxima
        if (vcmax > vnmax) {
          dst.at<uint8_t>(cc.y, cc.x) = 255;
        }
      }
    }
  }

  bool isEndOfLine(uchar *pixel, int lineSize, uchar valOfEmpty = (uchar) 1)
  {
    if (*pixel == (uchar)valOfEmpty)
      return false;
    if (pixel[-1] != valOfEmpty)
    {
      return (pixel[1] == valOfEmpty) && (pixel[lineSize + 1] == valOfEmpty) && (pixel[-lineSize + 1] == valOfEmpty);
    }
    if (pixel[-lineSize - 1] != valOfEmpty)
    {
      return (pixel[1] == valOfEmpty) && (pixel[lineSize + 1] == valOfEmpty) && (pixel[lineSize] == valOfEmpty);
    }
    if (pixel[-lineSize] != valOfEmpty)
    {
      return (pixel[lineSize - 1] == valOfEmpty) && (pixel[lineSize] == valOfEmpty) && (pixel[lineSize + 1] == valOfEmpty);
    }
    if (pixel[-lineSize + 1] != valOfEmpty)
    {
      return (pixel[-1] == valOfEmpty) && (pixel[lineSize - 1] == valOfEmpty) && (pixel[lineSize] == valOfEmpty);
    }
    if (pixel[1] != valOfEmpty)
    {
      return (pixel[-1] == valOfEmpty) && (pixel[lineSize - 1] == valOfEmpty) && (pixel[-lineSize - 1] == valOfEmpty);
    }
    if (pixel[lineSize + 1] != valOfEmpty)
    {
      return (pixel[-1] == valOfEmpty) && (pixel[-lineSize - 1] == valOfEmpty) && (pixel[-lineSize] == valOfEmpty);
    }
    if (pixel[lineSize] != valOfEmpty)
    {
      return (pixel[-lineSize - 1] == valOfEmpty) && (pixel[-lineSize] == valOfEmpty) && (pixel[-lineSize + 1] == valOfEmpty);
    }
    if (pixel[lineSize - 1] != valOfEmpty)
    {
      return (pixel[1] == valOfEmpty) && (pixel[-lineSize + 1] == valOfEmpty) && (pixel[-lineSize] == valOfEmpty);
    }
    return true;//isolated pixel... Considered as end of line...
  }

  void rewindMaxima(uchar* pixel, int nbMax)
  {
    for (int i = 0; i < nbMax; i++)
    {
      //if (((*pixel) != 0) && ((*pixel) != 2))
      if (((*pixel) == 1))
        return;
      else
      {
        (*pixel) = 1;
      }
      pixel--;
    }
  }

  cv::Mat nonMaximaSupression(cv::Mat imgFloat)
  {
    cv::Mat dst = Mat::zeros(imgFloat.size(), CV_8UC1);
    cv::Mat img;
    normalize(imgFloat, img, 0, 255, NORM_MINMAX, CV_8UC1);

    IplImage srctmp = img;
    IplImage* srcarr = &srctmp;
    IplImage *gradImg = cvCreateImage(cvSize(srcarr->width, srcarr->height), 8, 1);

    int aperture_size = 5;
    float moy = 0, vari = 0;
    static CvMat *dx = cvCreateMat(srcarr->height, srcarr->width, CV_16SC1);
    static CvMat *dy = cvCreateMat(srcarr->height, srcarr->width, CV_16SC1);

    if (dx->cols != srcarr->width || dx->rows != srcarr->height)
    {
      cvReleaseMat(&dx);
      cvReleaseMat(&dy);
      dx = cvCreateMat(srcarr->height, srcarr->width, CV_16SC1);
      dy = cvCreateMat(srcarr->height, srcarr->width, CV_16SC1);
    }

    cvSobel(srcarr, dx, 1, 0, aperture_size);
    cvSobel(srcarr, dy, 0, 1, aperture_size);

    //----------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------Détection des contours avec canny :---------------------------------------------
    //----------------------------------------------------------------------------------------------------------------------------

    void *buffer = 0;
    uchar **stack_top, **stack_bottom = 0;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub;
    CvSize size;
    int flags = aperture_size;
    int* mag_buf[3];
    uchar* map;
    int mapstep, maxsize;
    CvMat mag_row;

    src = cvGetMat(src, &srcstub);

    aperture_size &= INT_MAX;
    if ((aperture_size & 1) == 0 || aperture_size < 3 || aperture_size > 7)
      return Mat();

    size = CvSize(src->cols, src->rows);

    buffer = cvAlloc((size.width + 2)*(size.height + 2) + (size.width + 2) * 3 * sizeof(int));

    mag_buf[0] = (int*)buffer;
    mag_buf[1] = mag_buf[0] + size.width + 2;
    mag_buf[2] = mag_buf[1] + size.width + 2;
    map = (uchar*)(mag_buf[2] + size.width + 2);
    mapstep = size.width + 2;

    maxsize = MAX(1 << 10, size.width*size.height / 10);
    stack_top = stack_bottom = (uchar**)cvAlloc(maxsize*sizeof(stack_top[0]));

    memset(mag_buf[0], 0, (size.width + 2)*sizeof(int));
    memset(map, 1, mapstep);
    memset(map + mapstep*(size.height + 1), 1, mapstep);

    //sector numbers 
    //  (Top-Left Origin)
    //
    //       1   2   3
    //         *  *  * 
    //          * * *  
    //        0*******0
    //          * * *  
    //         *  *  * 
    //        3   2   1


#define CANNY_PUSH(d)    *(d) = (uchar)2, *stack_top++ = (d)
#define CANNY_POP(d)     (d) = *--stack_top

    mag_row = cvMat(1, size.width, CV_32F);

    int i, j;


    // calculate magnitude and angle of gradient, perform non-maxima supression.
    // fill the map with one of the following values:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge
    for (i = 0; i < size.height; i++)
    {
      uchar* _map;
      int prev_flag = 0;

      if (i == 0)
        continue;

      _map = map + mapstep*i + 1;
      _map[-1] = _map[size.width] = 1;
      uchar * _img = img.ptr<uchar>(i);
      uchar * _img_prev = img.ptr<uchar>(i - 1);
      uchar * _img_next = i<size.height - 1 ? img.ptr<uchar>(i + 1) : _img;

      if ((stack_top - stack_bottom) + size.width > maxsize)
      {
        uchar** new_stack_bottom;
        maxsize = MAX(maxsize * 3 / 2, maxsize + size.width);
        new_stack_bottom = (uchar**)cvAlloc(maxsize * sizeof(stack_top[0]));
        memcpy(new_stack_bottom, stack_bottom, (stack_top - stack_bottom)*sizeof(stack_top[0]));
        stack_top = new_stack_bottom + (stack_top - stack_bottom);
        cvFree(&stack_bottom);
        stack_bottom = new_stack_bottom;
      }
      bool riskyRun = false;
      for (j = 0; j < size.width; j++)
      {
        uchar m = _img[j];
        uchar mPrev = j>0 ? _img[j - 1] : m;
        uchar m_prev_ = _img_prev[j];
        uchar mNext = j < size.width - 1 ? _img[j + 1] : m;
        uchar m_next_ = _img_next[j];
        if (mPrev>0)
        {
          if (m > mPrev && m >= mNext)
          {
            if (m > mNext)
              riskyRun = false;
            if (!prev_flag && _map[j - mapstep] == 1)
            {
              CANNY_PUSH(_map + j);
              prev_flag = 1;
            }
            else
              _map[j] = (uchar)0;
            continue;
          }
          if (m > m_prev_ && m >= m_next_)
          {
            if (m >= mPrev)
              _map[j] = (uchar)0;
            else
              _map[j] = (uchar)3;
            continue;
          }
        }
        else
        {
          if (m > mPrev && m >= mNext)
          {
            if (m > mNext)
              riskyRun = false;
            else
              riskyRun = true;
            if (!prev_flag && _map[j - mapstep] == 1)
            {
              CANNY_PUSH(_map + j);
              prev_flag = 1;
            }
            else
              _map[j] = (uchar)0;
            continue;
          }
        }
        if (riskyRun)
        {
          rewindMaxima(_map + j, j);
          uchar* unusedVal;
          if (prev_flag == 1)
            CANNY_POP(unusedVal);
        }
        riskyRun = false;
        prev_flag = 0;
        _map[j] = (uchar)1;
      }
    }
    
    // now track the edges (hysteresis thresholding)
    while (stack_top > stack_bottom)
    {
      uchar* m;
      if ((stack_top - stack_bottom) + 8 > maxsize)
      {
        uchar** new_stack_bottom;
        maxsize = MAX(maxsize * 3 / 2, maxsize + 8);
        new_stack_bottom = (uchar**)cvAlloc(maxsize * sizeof(stack_top[0]));
        memcpy(new_stack_bottom, stack_bottom, (stack_top - stack_bottom)*sizeof(stack_top[0]));
        stack_top = new_stack_bottom + (stack_top - stack_bottom);
        cvFree(&stack_bottom);
        stack_bottom = new_stack_bottom;
      }

      CANNY_POP(m);

      if (m[-1] == 0)
        CANNY_PUSH(m - 1);
      if (m[1] == 0)
        CANNY_PUSH(m + 1);
      if (m[-mapstep - 1] == 0)
        CANNY_PUSH(m - mapstep - 1);
      if (m[-mapstep] == 0)
        CANNY_PUSH(m - mapstep);
      if (m[-mapstep + 1] == 0)
        CANNY_PUSH(m - mapstep + 1);
      if (m[mapstep - 1] == 0)
        CANNY_PUSH(m + mapstep - 1);
      if (m[mapstep] == 0)
        CANNY_PUSH(m + mapstep);
      if (m[mapstep + 1] == 0)
        CANNY_PUSH(m + mapstep + 1);
    }

    //now track bad ending lines:

#define CANNY_PUSH_BADS(d)    *(d) = (uchar)4, *stack_top++ = (d)
#define CANNY_POP(d)     (d) = *--stack_top
    for (i = 0; i < size.height; i++)
    {
      uchar* _map;

      if (i == 0)
        continue;

      _map = map + mapstep*i + 1;
      _map[-1] = _map[size.width] = 1;
      uchar * _img = img.ptr<uchar>(i);

      for (j = 0; j < size.width; j++)
      {
        if (_map[j] == 3 && isEndOfLine(&_map[j], mapstep))
          CANNY_PUSH_BADS(_map + j);
      }
    }
    // now track the edges (hysteresis thresholding)
    while (stack_top > stack_bottom)
    {
      uchar* m;
      if ((stack_top - stack_bottom) + 8 > maxsize)
      {
        uchar** new_stack_bottom;
        maxsize = MAX(maxsize * 3 / 2, maxsize + 8);
        new_stack_bottom = (uchar**)cvAlloc(maxsize * sizeof(stack_top[0]));
        memcpy(new_stack_bottom, stack_bottom, (stack_top - stack_bottom)*sizeof(stack_top[0]));
        stack_top = new_stack_bottom + (stack_top - stack_bottom);
        cvFree(&stack_bottom);
        stack_bottom = new_stack_bottom;
      }

      CANNY_POP(m);

      if (m[-1] == 3)
        CANNY_PUSH_BADS(m - 1);
      if (m[1] == 3)
        CANNY_PUSH_BADS(m + 1);
      if (m[-mapstep - 1] == 3)
        CANNY_PUSH_BADS(m - mapstep - 1);
      if (m[-mapstep] == 3)
        CANNY_PUSH_BADS(m - mapstep);
      if (m[-mapstep + 1] == 3)
        CANNY_PUSH_BADS(m - mapstep + 1);
      if (m[mapstep - 1] == 3)
        CANNY_PUSH_BADS(m + mapstep - 1);
      if (m[mapstep] == 3)
        CANNY_PUSH_BADS(m + mapstep);
      if (m[mapstep + 1] == 3)
        CANNY_PUSH_BADS(m + mapstep + 1);
    }
    
    // the final pass, form the final image
    uchar* _dst, *_src;
    for (i = 5; i < size.height - 6; i++)
    {
      const uchar* _map = map + mapstep*(i + 1) + 1;
      _dst = dst.ptr<uchar>(i);
      //_dst = (uchar*)dstarr->imageData + dstarr->widthStep*i;
      _src = src->data.ptr + src->step*i;

      //        _dx = (short*)(dx->data.ptr + dx->step*(i));
      //        _dy = (short*)(dy->data.ptr + dy->step*(i));

      //pc->shouldResize(size.width);


      for (j = 5; j < size.width - 6; j++){
        if (_map[j] == 0)
          _dst[j] = 255;
        else{
          uchar test = _map[j];
          if (_map[j] == (uchar)1)
            _dst[j] = 0;
          else{
            if (_map[j] == (uchar)3)
              _dst[j] = 255;
            else{
              if (_map[j] == (uchar)4)
                _dst[j] = 0;
              else{
                _dst[j] = 255;
              }
            }
          }
        }
        //PILE_CANNY_PUSH(&_src[j]);
      }
      //_dst[j] = (uchar)-(_map[j] >> 1);
    }
    cvFree(&buffer);
    cvFree(&stack_bottom);

    return dst;
  }

  //from a binary image, find pen size
  float findBiggestPenSize(cv::Mat binaryImg)
  {
    cv::Mat distImg;

    cv::distanceTransform(binaryImg,
      distImg,
      cv::DIST_L2,
      cv::DIST_MASK_3);

    int image_width = binaryImg.cols;
    int image_height = binaryImg.rows;
    double meanSize = 0;
    int nbPixel = 0;

    for (int i = 0; i < image_width; i++){
      for (int j = 0; j < image_height; j++){
        float tmp = distImg.at<float>(j, i);
        if (tmp > 0)
        {
          meanSize += tmp;
          nbPixel++;
        }
      }
    }

    return (float)meanSize/nbPixel;
  }

  /**
  * Perform one thinning iteration.
  * Normally you wouldn't call this function directly from your code.
  *
  * @param  im    Binary image with range = 0-1
  * @param  iter  0=even, 1=odd
  */
  void thinningIteration(cv::Mat& im, int iter)
  {
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    uchar *line, *prevLine, *nextLine;
    nextLine = im.ptr<uchar>(1);
    line = im.ptr<uchar>(0);
    for (int i = 1; i < im.rows - 1; i++)
    {
      prevLine = line;
      line = nextLine;
      nextLine = im.ptr<uchar>(i + 1);
      for (int j = 1; j < im.cols - 1; j++)
      {
        const uchar& p2 = prevLine[j];
        const uchar& p3 = prevLine[j + 1];
        const uchar& p4 = line[j + 1];
        const uchar& p5 = nextLine[j + 1];
        const uchar& p6 = nextLine[j];
        const uchar& p7 = nextLine[j - 1];
        const uchar& p8 = line[j - 1];
        const uchar& p9 = prevLine[j - 1];

        int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
          (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
          (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
          (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
        int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
        int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
        int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

        if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
          marker.at<uchar>(i, j) = 1;
      }
    }

    im &= ~marker;
  }

  void thinning(cv::Mat& im, int iter)
  {
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
      iter--;
      thinningIteration(im, 0);
      thinningIteration(im, 1);
      cv::absdiff(im, prev, diff);
      im.copyTo(prev);
    } while (cv::countNonZero(diff) > 0 && iter>=0);

    im *= 255;
  }

  bool ThinningBlock::run(bool oneShot){
    if (_myInputs["BLOCK__THINNING_IN_IMAGE"].isDefaultValue())
      return false;
    
    cv::Mat mat = _myInputs["BLOCK__THINNING_IN_IMAGE"].get<cv::Mat>();
    if (!mat.empty())
    {
      cv::Mat output = MatrixConvertor::adjustChannels(mat.clone(), 1);

      _myOutputs["BLOCK__THINNING_OUT_IMAGE"] = nonMaximaSupression(output);
    }
    return !mat.empty();
  };
};