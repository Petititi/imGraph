#ifndef _MATRIX_EDITOR_HEADER_
#define _MATRIX_EDITOR_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif

#include <opencv2/core.hpp>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace charliesoft
{
  class ScribbleArea : public QWidget
  {
    Q_OBJECT

  public:
    ScribbleArea(QWidget *parent = 0);

    bool openImage(cv::Mat &img);
    void setPenWidth(int newWidth);

    bool isModified() const { return modified; }
    int penWidth() const { return myPenWidth; }

    cv::Mat getMask();
  protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);

  private:
    void drawLineTo(const QPoint &endPoint);
    void resizeImage(QImage *image, const QSize &newSize);

    bool modified;
    bool scribbling;
    bool erasing;
    int myPenWidth;
    QColor myPenColor;
    QImage mask;
    QImage imageSrc;
    QPoint lastPoint;
  };


  class MatrixEditor :public QDialog
  {
    Q_OBJECT;

    QScrollArea *_scrollImg;
    ScribbleArea* _imgEditor;
    QSize _imgSize;

    cv::Mat _result;

    QVBoxLayout *_mainLayout;
    QPushButton* _OKbutton;
    QPushButton* _Cancelbutton;
  public:
    MatrixEditor();

    cv::Mat getResult() const { return _result; }

    public slots:
    void accept_button();
    void reject_button();
    void createScribble();

  signals:
    void askSynchro();
  };
}


#endif