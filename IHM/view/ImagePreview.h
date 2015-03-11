#ifndef _IMAGE_PREVIEW_HEADER_
#define _IMAGE_PREVIEW_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4244 4275 4800)
#endif
#include <QGraphicsView>
#include <QResizeEvent>
#include <QDialog >
#include <QPainter>
#include <QRect>
#include <QString>
#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QComboBox>
#include <QDial>
#include <QCheckBox>
#include <QPainterPath>
#include <QBasicTimer>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include "Convertor.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

namespace charliesoft
{
  class Converter : public QObject {
    Q_OBJECT;
    QBasicTimer _timer;
    cv::Mat _frame;
    bool _processAll;
    static void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }
    void queue(const cv::Mat & frame) {
      _frame = frame;
      if (!_timer.isActive()) _timer.start(0, this);
    }
    void process(cv::Mat frame) {
      //optimisation???
      //if (frame.cols>9 && frame.rows>9)
      //  cv::resize(frame, frame, cv::Size(), 0.3, 0.3, cv::INTER_AREA);

      frame = MatrixConvertor::convert(frame, CV_8UC3);

      cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
      const QImage image(frame.data, frame.cols, frame.rows, frame.step,
        QImage::Format_RGB888, &matDeleter, new cv::Mat(frame));
      emit imageReady(image);

      frame = cv::Mat();
    }
    void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() == _timer.timerId())
      {
        process(_frame);
        _frame.release();
        _timer.stop();
      }
    }
  public:
    explicit Converter(QObject * parent = 0) : QObject(parent), _processAll(false) {}
    void setProcessAll(bool all) { _processAll = all; }
    Q_SIGNAL void imageReady(const QImage &);
    void processFrame(const cv::Mat & frame) {
      if (_processAll) process(frame); else queue(frame);
    }
  };

  class ImageViewer : public QWidget {
    Q_OBJECT;
    QImage _img;
    Converter* _converter;
    void paintEvent(QPaintEvent *) {
      QPainter p(this);
      p.drawImage(QRect(0, 0, width(), height()), _img, QRect(0, 0, _img.width(), _img.height()));
    }
  public: 
    ImageViewer(QWidget * parent = 0) : QWidget(parent) {
      setAttribute(Qt::WA_OpaquePaintEvent);
      _converter = new Converter();
      connect(_converter, SIGNAL(imageReady(QImage)), SLOT(setImage(QImage)));
    }

    void setImage(cv::Mat img)
    {
      _converter->processFrame(img);
    }

    Q_SLOT void setImage(const QImage & img) {
      _img = img;
      update();
    }
    Q_SIGNAL void matReady(cv::Mat);

  protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
  };

}

#endif