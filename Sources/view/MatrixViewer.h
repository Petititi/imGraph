//IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.

// By downloading, copying, installing or using the software you agree to this license.
// If you do not agree to this license, do not download, install,
// copy or use the software.


//                          License Agreement
//               For Open Source Computer Vision Library

//Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
//Copyright (C) 2008-2010, Willow Garage Inc., all rights reserved.
//Third party copyrights are property of their respective owners.

//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:

//  * Redistribution's of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.

//  * Redistribution's in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.

//  * The name of the copyright holders may not be used to endorse or promote products
//  derived from this software without specific prior written permission.

//This software is provided by the copyright holders and contributors "as is" and
//any express or implied warranties, including, but not limited to, the implied
//warranties of merchantability and fitness for a particular purpose are disclaimed.
//In no event shall the Intel Corporation or contributors be liable for any direct,
//indirect, incidental, special, exemplary, or consequential damages
//(including, but not limited to, procurement of substitute goods or services;
//loss of use, data, or profits; or business interruption) however caused
//and on any theory of liability, whether in contract, strict liability,
//or tort (including negligence or otherwise) arising in any way out of
//the use of this software, even if advised of the possibility of such damage.

//--------------------Google Code 2010 -- Yannick Verdie--------------------//
#ifndef __OPENCV_HIGHGUI_QT_H__
#define __OPENCV_HIGHGUI_QT_H__

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc.hpp"

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QGraphicsView>
#include <QSizePolicy>
#include <QInputDialog>
#include <QBoxLayout>
#include <QSettings>
#include <qtimer.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QWaitCondition>
#include <QKeyEvent>
#include <QMetaObject>
#include <QPointer>
#include <QSlider>
#include <QLabel>
#include <QIODevice>
#include <QShortcut>
#include <QStatusBar>
#include <QVarLengthArray>
#include <QFileInfo>
#include <QDate>
#include <QFileDialog>
#include <QToolBar>
#include <QAction>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QDialog>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

class MatrixViewer;
class DefaultViewPort;

//start private enum
enum { CV_MODE_NORMAL = 0, CV_MODE_OPENGL = 1 };

//we can change the keyboard shortcuts from here !
enum {
  shortcut_zoom_normal = Qt::CTRL + Qt::Key_Z,
  shortcut_zoom_imgRegion = Qt::CTRL + Qt::Key_X,
  shortcut_save_img = Qt::CTRL + Qt::Key_S,
  shortcut_edit_pen_img = Qt::CTRL + Qt::Key_E,
  shortcut_properties_win = Qt::CTRL + Qt::Key_P,
  shortcut_zoom_in = Qt::CTRL + Qt::Key_Plus,//QKeySequence(QKeySequence::ZoomIn),
  shortcut_zoom_out = Qt::CTRL + Qt::Key_Minus,//QKeySequence(QKeySequence::ZoomOut),
  shortcut_panning_left = Qt::CTRL + Qt::Key_Left,
  shortcut_panning_right = Qt::CTRL + Qt::Key_Right,
  shortcut_panning_up = Qt::CTRL + Qt::Key_Up,
  shortcut_panning_down = Qt::CTRL + Qt::Key_Down,
  shortcut_always_top = Qt::CTRL + Qt::Key_T
};

//end enum
#define _WINDOW_MATRIX_CREATION_MODE  0x00010000

class ToolsWindow : public QWidget
{
  Q_OBJECT
public:
  ToolsWindow(QString name, MatrixViewer* parent);
  ~ToolsWindow();

  void addWidget(QWidget* obj);
  void linkWithMatrix(MatrixViewer* mView);

protected:
  void closeEvent(QCloseEvent * e);
  void showEvent(QShowEvent * event);
  void hideEvent(QHideEvent * event);

  MatrixViewer* _parent;

  QPushButton* _updateMatrix;
  QComboBox* _matrixTypes;
  QComboBox* _matrixVals;
  QLineEdit* _rows;
  QLineEdit* _cols;
  QLineEdit* _channels;

  QPushButton* color_choose;
  QLineEdit* pencilSize;

  QPointer<QBoxLayout> myLayout;

  public slots:
  void updateMatrix();
};

#define __ACT_IMGRAPH_LOAD       0
#define __ACT_IMGRAPH_SAVE       __ACT_IMGRAPH_LOAD     +1
#define __ACT_IMGRAPH_LEFT       __ACT_IMGRAPH_SAVE     +1
#define __ACT_IMGRAPH_RIGHT      __ACT_IMGRAPH_LEFT     +1
#define __ACT_IMGRAPH_UP         __ACT_IMGRAPH_RIGHT    +1
#define __ACT_IMGRAPH_DOWN       __ACT_IMGRAPH_UP       +1
#define __ACT_IMGRAPH_ZOOM_X1    __ACT_IMGRAPH_DOWN     +1
#define __ACT_IMGRAPH_ZOOM_IN    __ACT_IMGRAPH_ZOOM_X1  +1
#define __ACT_IMGRAPH_ZOOM_OUT   __ACT_IMGRAPH_ZOOM_IN  +1
#define __ACT_IMGRAPH_PEN_EDIT   __ACT_IMGRAPH_ZOOM_OUT +1
#define __ACT_IMGRAPH_ONTOP      __ACT_IMGRAPH_PEN_EDIT +1

class MatrixViewer : public QDialog
{
  Q_OBJECT;

public:
  MatrixViewer(QString arg2, int flag = CV_WINDOW_NORMAL);
  ~MatrixViewer();

  void writeSettings();
  void readSettings();

  int getPropWindow();
  void setPropWindow(int flags);
  
  bool isPencil_mode() const { return pencil_mode; };

  void toggleFullScreen(int flags);

  void updateImage(cv::Mat arr);
  cv::Mat getMatrix();

  void displayInfo(QString text, int delayms);
  void displayStatusBar(QString text, int delayms);

  void enablePropertiesButton();

  void setViewportSize(QSize size);

  DefaultViewPort* view(){ return myView; };

  //parameters (will be save/load)
  int param_flags;
  int param_gui_mode;
  int param_creation_mode;

  QPointer<QBoxLayout> myGlobalLayout; //All the widget (toolbar, view, LayoutBar, ...) are attached to it

  QVector<QAction*> vect_QActions;

  QPointer<QStatusBar> myStatusBar;
  QPointer<QToolBar> myToolBar;
  QPointer<QLabel> myStatusBar_msg;

  public slots:
  void changePenSize();
protected:
  virtual void keyPressEvent(QKeyEvent* event);

  ToolsWindow* myTools;
private:
  bool isOnTop;
  bool pencil_mode;
  int mode_display; //opengl or native
  DefaultViewPort* myView;

  QVector<QShortcut*> vect_QShortcuts;

  void createActions();
  void createShortcuts();
  void createView();
  void createGlobalLayout();
  ToolsWindow* createParameterWindow();

  void hideTools();
  void showTools();
  QSize getAvailableSize();

  private slots:
  void displayPropertiesWin();
  void switchEditingImg();
  void chooseColor();
  void switchOnTop();
};


enum _typemouse_event { mouse_up = 0, mouse_down = 1, mouse_dbclick = 2, mouse_move = 3 };
static const int tableMouseButtons[][3] = {
    { CV_EVENT_LBUTTONUP, CV_EVENT_RBUTTONUP, CV_EVENT_MBUTTONUP },               //mouse_up
    { CV_EVENT_LBUTTONDOWN, CV_EVENT_RBUTTONDOWN, CV_EVENT_MBUTTONDOWN },         //mouse_down
    { CV_EVENT_LBUTTONDBLCLK, CV_EVENT_RBUTTONDBLCLK, CV_EVENT_MBUTTONDBLCLK },   //mouse_dbclick
    { CV_EVENT_MOUSEMOVE, CV_EVENT_MOUSEMOVE, CV_EVENT_MOUSEMOVE }                //mouse_move
};


class ViewPort
{
public:
  virtual ~ViewPort() {}

  virtual QWidget* getWidget() = 0;

  virtual void writeSettings(QSettings& settings) = 0;
  virtual void readSettings(QSettings& settings) = 0;

  virtual void updateImage(const cv::Mat arr) = 0;

  virtual void startDisplayInfo(QString text, int delayms) = 0;

  virtual void setSize(QSize size_) = 0;
};




class DefaultViewPort : public QGraphicsView, public ViewPort
{
  Q_OBJECT

public:
  boost::recursive_mutex _mtx;    // explicit mutex declaration

  DefaultViewPort(MatrixViewer* centralWidget);
  ~DefaultViewPort();

  QWidget* getWidget();

  void writeSettings(QSettings& settings);
  void readSettings(QSettings& settings);

  double getRatio();
  void setRatio(int flags);

  void updateImage(const cv::Mat arr);

  void startDisplayInfo(QString text, int delayms);

  void setSize(QSize size_);

  void updateViewport(){ viewport()->update(); };

  public slots:
  //reference:
  //http://www.qtcentre.org/wiki/index.php?title=QGraphicsView:_Smooth_Panning_and_Zooming
  //http://doc.qt.nokia.com/4.6/gestures-imagegestures-imagewidget-cpp.html

  void siftWindowOnLeft();
  void siftWindowOnRight();
  void siftWindowOnUp();
  void siftWindowOnDown();

  void resetZoom();
  void imgRegion();
  void ZoomIn();
  void ZoomOut();

  void loadMatrix();
  void saveView();
  QColor getPenColor() const { return myPenColor; }
  void setPenColor(QColor val) { myPenColor = val; }
  float getPenSize() const { return myPenWidth; }
  void setPenSize(float newSize) { myPenWidth = newSize; }

  void updateImage();
  cv::Mat getMatrix();
protected:
  void contextMenuEvent(QContextMenuEvent* event);
  void resizeEvent(QResizeEvent* event);
  void paintEvent(QPaintEvent* paintEventInfo);
  void wheelEvent(QWheelEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);

  void drawLineTo(const QPointF &endPoint);
  QPoint toImgCoord(QPointF src);

private:
  QPointF lastPoint;

  QColor myPenColor;
  float myPenWidth;

  //parameters (will be save/load)
  QTransform param_matrixWorld;

  cv::Mat image2Draw_mat;
  cv::Mat image_copy;
  QImage image2Draw_qt;
  int nbChannelOriginImage;

  void scaleView(qreal scaleFactor, QPointF center);
  void moveView(QPointF delta);

  QPoint mouseCoordinate;
  QPoint positionGrabbing;
  QRect  positionCorners;
  QTransform matrixWorld_inv;
  float ratioX, ratioY;

  bool isSameSize(IplImage* img1, IplImage* img2);

  QSize sizeHint() const;
  QPointer<MatrixViewer> centralWidget;
  QPointer<QTimer> timerDisplay;
  bool drawInfo;
  QString infoText;
  QRectF target;
  bool labelsShown;

  QPoint pixelEdit;
  int canalEdit;
  bool updateEdits;
  QLineEdit *imgEditPixel_R;
  QLineEdit *imgEditPixel_G;
  QLineEdit *imgEditPixel_B;
  QLineEdit *imgEditPixel_A;

  void drawInstructions(QPainter *painter);
  void drawViewOverview(QPainter *painter);
  void drawImgRegion(QPainter *painter);
  void draw2D(QPainter *painter);
  void drawStatusBar();
  void controlImagePosition();
  void icvmouseProcessing(QPointF pt, int cv_event, int flags);

  private slots:
  void stopDisplayInfo();
};

#endif
