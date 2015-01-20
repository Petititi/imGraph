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
#ifndef __GUI_RECEIVER_QT_H__
#define __GUI_RECEIVER_QT_H__

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4244 4251 4275 4800)
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
class GraphViewer;

void imshow(cv::String name, cv::Mat im);
MatrixViewer* createWindow(cv::String name, int params = 0);
GraphViewer* createGraphView(cv::String name);

class GuiReceiver : public QObject
{
  Q_OBJECT

public:
  GuiReceiver();

  bool bTimeOut;
  QTimer* timer;

  public slots:
  void createWindow(QString name, int flags = 0);
  void createGraph(QString name);
  void destroyWindow(QString name);
  void destroyAllWindow();
  void moveWindow(QString name, int x, int y);
  void resizeWindow(QString name, int width, int height);
  void showImage(QString name, cv::Mat arr);
  void displayInfo(QString name, QString text, int delayms);
  void timeOut();
  void toggleFullScreen(QString name, double flags);
  double isFullScreen(QString name);
  double getPropWindow(QString name);
  void setPropWindow(QString name, double flags);
  void saveWindowParameters(QString name);
  void loadWindowParameters(QString name);
  void enablePropertiesButtonEachWindow();

private:
  GuiReceiver* guiMainThread;
  int nb_windows;
  bool doesExternalQAppExist;
};

#endif
