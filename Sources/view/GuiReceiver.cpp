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


#include <memory>

#include "GuiReceiver.h"
#include "GraphViewer.h"
#include "MatrixViewer.h"
#include "MatrixConvertor.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <math.h>
#include <QColorDialog>
#include <boost/lexical_cast.hpp>
#include <boost/thread/lock_guard.hpp>
#include <opencv2/highgui.hpp>
#include <QString>
#include <QDialogButtonBox>
#include <view/Window.h>

#ifdef _WIN32
#pragma warning(disable:4503)
#include <windows.h>
#pragma warning(pop)
#else
#include <unistd.h>
#endif

using boost::lexical_cast;
using boost::lock_guard;
using namespace cv;
using namespace charliesoft;

//Static and global first
static bool multiThreads = false;
GuiReceiver* GuiReceiver::guiMainThread = NULL;

static MatrixViewer* icvFindWindowByName(QString name)
{
  MatrixViewer* window = 0;

  QWidgetList winList = QApplication::topLevelWidgets();
  for (auto* widget : winList)
  {
    if (widget->isWindow() && !widget->parentWidget())//is a window without parent
    {
      if (MatrixViewer* w = dynamic_cast<MatrixViewer*>(widget))
      {
        if (w->windowTitle() == name)
        {
          window = w;
          break;
        }
      }
    }
  }

  return window;
}

static GraphViewer* icvFindGraphViewByName(QString name)
{
  GraphViewer* window = 0;

  //This is not a very clean way to do the stuff. Indeed, QAction automatically generate toolTil (QLabel)
  //that can be grabbed here and crash the code at 'w->param_name==name'.
  foreach(QWidget* widget, QApplication::topLevelWidgets())
  {
    if (widget->isWindow() && !widget->parentWidget())//is a window without parent
    {
      if (GraphViewer* w = dynamic_cast<GraphViewer*>(widget))
      {
        if (w->windowTitle() == name)
        {
          window = w;
          break;
        }
      }
    }
  }

  return window;
}

void imshow(cv::String name, cv::Mat im)
{
  GuiReceiver::getInstance()->showImage(QString(name.c_str()), im);
}

GraphViewer* createGraphView(cv::String name)
{
  GraphViewer* gv = icvFindGraphViewByName(name.c_str());
  if (gv == NULL)
  {
    QMetaObject::invokeMethod(GuiReceiver::getInstance(),
      "createGraph",
      Qt::AutoConnection,
      Q_ARG(QString, QString(name.c_str())));

    boost::this_thread::sleep(boost::posix_time::milliseconds(100));//wait for the window creation
    gv = icvFindGraphViewByName(name.c_str());
  }

  return gv;
}

MatrixViewer* createWindow(cv::String name, int params)
{
  MatrixViewer* mv = icvFindWindowByName(name.c_str());
  if (mv == NULL)
  {
    QMetaObject::invokeMethod(GuiReceiver::getInstance(),
      "createWindow",
      Qt::AutoConnection,
      Q_ARG(QString, QString(name.c_str())),
      Q_ARG(int, params));

    boost::this_thread::sleep(boost::posix_time::milliseconds(100));//wait for the window creation
    mv = icvFindWindowByName(name.c_str());
  }

  return mv;
}

//////////////////////////////////////////////////////
// GuiReceiver
boost::recursive_mutex mtx_gui;
GuiReceiver* GuiReceiver::getInstance()
{
  boost::unique_lock<boost::recursive_mutex> guard(mtx_gui);
  if (guiMainThread==NULL)
    guiMainThread = new GuiReceiver();
  return guiMainThread;
}

GuiReceiver::GuiReceiver() : bTimeOut(false), nb_windows(0)
{
  guiMainThread = this;
  doesExternalQAppExist = (QApplication::instance() != 0);

  timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeOut()));
  timer->setSingleShot(true);
}


void GuiReceiver::saveWindowParameters(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
    w->writeSettings();
}


void GuiReceiver::loadWindowParameters(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
    w->readSettings();
}



double GuiReceiver::getPropWindow(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w)
    return -1;

  return (double)w->getPropWindow();
}


void GuiReceiver::setPropWindow(QString name, double arg2)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w)
    return;

  int flags = (int)arg2;

  w->setPropWindow(flags);
}


double GuiReceiver::isFullScreen(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w)
    return -1;

  return w->isFullScreen() ? CV_WINDOW_FULLSCREEN : CV_WINDOW_NORMAL;
}


void GuiReceiver::toggleFullScreen(QString name, double arg2)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w)
    return;

  int flags = (int)arg2;

  w->toggleFullScreen(flags);
}

void GuiReceiver::createGraph(QString name)
{
  if (!qApp)
    CV_Error(CV_StsNullPtr, "NULL session handler");

  // Check the name in the storage
  if (icvFindGraphViewByName(name.toLatin1().data()))
    return;


  nb_windows++;
  GraphViewer* w = new GraphViewer();
  w->setWindowTitle(name);
}

void GuiReceiver::createWindow(QString name, int flags)
{
  if (!qApp)
    CV_Error(CV_StsNullPtr, "NULL session handler");

  // Check the name in the storage
  if (icvFindWindowByName(name.toLatin1().data()))
    return;


  nb_windows++;
  MatrixViewer* w = new MatrixViewer(name, flags);

  w->updateImage(Mat::zeros(3,3,CV_8UC1));

}


void GuiReceiver::timeOut()
{
  bTimeOut = true;
}


void GuiReceiver::displayInfo(QString name, QString text, int delayms)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
    w->displayInfo(text, delayms);
}

void GuiReceiver::showImage(QString name, cv::Mat arr)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w) //as observed in the previous implementation (W32, GTK or Carbon), create a new window is the pointer returned is null
  {
    QMetaObject::invokeMethod(guiMainThread,
      "createWindow",
      //Qt::BlockingQueuedConnection,
      Qt::AutoConnection,
      Q_ARG(QString, QString(name)),
      Q_ARG(int, 0));
    w = icvFindWindowByName(name);
  }

  if (!w || arr.empty())
    return; // keep silence here.

  w->updateImage(arr);

  if (w->isHidden())
  {
    QMetaObject::invokeMethod(w,
      "show",
      Qt::AutoConnection);
  }
}


void GuiReceiver::destroyWindow(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
  {
    w->close();

    //in not-multiThreads mode, looks like the window is hidden but not deleted
    //so I do it manually
    //otherwise QApplication do it for me if the exec command was executed (in multiThread mode)
    if (!multiThreads)
      delete w;
  }
}


void GuiReceiver::moveWindow(QString name, int x, int y)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
    w->move(x, y);
}


void GuiReceiver::resizeWindow(QString name, int width, int height)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (w)
  {
    w->showNormal();
    w->setViewportSize(QSize(width, height));
  }
}


void GuiReceiver::enablePropertiesButtonEachWindow()
{
  //For each window, enable window property button
  foreach(QWidget* widget, QApplication::topLevelWidgets())
  {
    if (widget->isWindow() && !widget->parentWidget()) //is a window without parent
    {
      if (MatrixViewer* w = dynamic_cast<MatrixViewer*>(widget))
      {
        //active window properties button
        w->enablePropertiesButton();
      }
    }
  }
}
