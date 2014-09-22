
#include "MatrixEditor.h"
#include "GraphicView.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif

#ifdef _WIN32
#pragma warning(pop)
#endif

using cv::Vec3b;

namespace charliesoft
{
  ScribbleArea::ScribbleArea(QWidget *parent)
    : QWidget(parent)
  {
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    myPenWidth = 1;
    myPenColor = Qt::white;
  }
  bool ScribbleArea::openImage(cv::Mat &src)
  {
    QImage loadedImage(src.cols, src.rows, QImage::Format_ARGB32);
    for (int y = 0; y < src.rows; ++y) {
      const cv::Vec3b *srcrow = src.ptr<Vec3b>(y);
      QRgb *destrow = (QRgb*)loadedImage.scanLine(y);
      for (int x = 0; x < src.cols; ++x) {
        destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
      }
    }

    QSize newSize = size();
    resizeImage(&loadedImage, newSize);
    mask = loadedImage;
    mask.fill(qRgb(0, 0, 0));
    imageSrc = loadedImage;
    modified = false;
    update();
    return true;
  }
  void ScribbleArea::setPenWidth(int newWidth)
  {
    myPenWidth = newWidth;
  }
  void ScribbleArea::mousePressEvent(QMouseEvent *event)
  {
    scribbling = erasing = false;
    if (event->button() == Qt::LeftButton) {
      lastPoint = event->pos();
      scribbling = true;
    }
    if (event->button() == Qt::RightButton) {
      lastPoint = event->pos();
      erasing = true;
    }
  }
  void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
  {
    if ((event->buttons() & Qt::LeftButton) && scribbling)
      drawLineTo(event->pos());
    if ((event->buttons() & Qt::RightButton) && erasing)
      drawLineTo(event->pos());
  }
  void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
  {
    if (event->button() == Qt::LeftButton && scribbling) {
      drawLineTo(event->pos());
      scribbling = false;
    }
  }
  void ScribbleArea::paintEvent(QPaintEvent *event)
  {
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    QImage imageTmp = imageSrc;

    for (int y = 0; y < mask.height(); ++y) {
      QRgb *srcrow = (QRgb*)mask.scanLine(y);
      QRgb *destrow = (QRgb*)imageTmp.scanLine(y);
      for (int x = 0; x < mask.width(); ++x) {
        if (srcrow[x] == qRgb(255, 255, 255))
          destrow[x] = qRgb(0, 0, 255);
      }
    }


    painter.drawImage(dirtyRect, imageTmp, dirtyRect);
  }
  void ScribbleArea::wheelEvent(QWheelEvent *event)
  {
    if (event->delta() > 0)
      myPenWidth++;
    else
      myPenWidth--;
    if (myPenWidth < 2)
      myPenWidth = 1;
  }
  void ScribbleArea::resizeEvent(QResizeEvent *event)
  {
    if (width() > mask.width() || height() > mask.height())
    {
      int newWidth = qMax(width() + 128, mask.width());
      int newHeight = qMax(height() + 128, mask.height());
      resizeImage(&mask, QSize(newWidth, newHeight));
      update();
    }
    QWidget::resizeEvent(event);
  }
  void ScribbleArea::drawLineTo(const QPoint &endPoint)
  {
    QPainter painter(&mask);
    QColor tmpColor = myPenColor;
    if (erasing)
      tmpColor = Qt::black;
    painter.setPen(QPen(tmpColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
      Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);
    modified = true;

    int rad = (myPenWidth / 2) + 2;
    update(QRect(lastPoint, endPoint).normalized()
      .adjusted(-rad, -rad, +rad, +rad));
    lastPoint = endPoint;
  }
  void ScribbleArea::resizeImage(QImage *image, const QSize &newSize)
  {
    if (image->size() == newSize)
      return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
  }
  cv::Mat ScribbleArea::getMask()
  {
    cv::Mat output(mask.height(), mask.width(), CV_8UC3);
    for (int y = 0; y < mask.height(); ++y) {
      QRgb *srcrow = (QRgb*)mask.scanLine(y);
      cv::Vec3b *out = output.ptr<Vec3b>(y);
      for (int x = 0; x < mask.width(); ++x) {
        uchar outVal = 0;
        if (srcrow[x] == qRgb(255, 255, 255))
          outVal = 255;
        out[x][0] = out[x][1] = out[x][2] = outVal;
      }
    }
    return output;
  };

  MatrixEditor::MatrixEditor()
  {
    setWindowTitle(_QT("MATRIX_EDITOR_TITLE"));
    _imgEditor = NULL;
    _mainLayout = new QVBoxLayout();
    setLayout(_mainLayout);

    QHBoxLayout *titleLayout;

    QLabel* tmpLabel = new QLabel("Repertoire du ground truth pour la classification:");
    _mainLayout->addWidget(tmpLabel);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignLeft);
    QWidget* tmp = new QWidget();
    tmp->setLayout(layout);

  }

  void MatrixEditor::accept_button()
  {

  };
  void MatrixEditor::reject_button()
  {

  };

  void MatrixEditor::createScribble()
  {
    if (_imgEditor != NULL)
    {
      delete _imgEditor;
      delete _scrollImg;
    }
    _imgEditor = new ScribbleArea(this);
    _imgEditor->setFixedSize(_imgSize);
    //_imgEditor->openImage(imgVis);
    _imgEditor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    _scrollImg = new QScrollArea;
    _scrollImg->setBackgroundRole(QPalette::Dark);
    _scrollImg->setWidget(_imgEditor);
    _mainLayout->addWidget(_scrollImg);
  };
}