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
using namespace cv;
using namespace charliesoft;

//Static and global first
static GuiReceiver *guiMainThread = NULL;
static bool multiThreads = false;
static int last_key = -1;
QWaitCondition key_pressed;
QMutex mutexKey;
static const unsigned int threshold_zoom_img_region = 30;
//the minimum zoom value to start displaying the values in the grid
//that is also the number of pixel per grid

//end static and global

static MatrixViewer* icvFindWindowByName(QString name)
{
  MatrixViewer* window = 0;

  //This is not a very clean way to do the stuff. Indeed, QAction automatically generate toolTil (QLabel)
  //that can be grabbed here and crash the code at 'w->param_name==name'.
  foreach(QWidget* widget, QApplication::topLevelWidgets())
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

void imshow(cv::String name, cv::Mat im)
{
  if (!guiMainThread)
    guiMainThread = new GuiReceiver;
  guiMainThread->showImage(QString(name.c_str()), im);
}

MatrixViewer* createWindow(cv::String name, int params)
{
  if (!guiMainThread)
    guiMainThread = new GuiReceiver;

  if (icvFindWindowByName(name.c_str()) == NULL)
  {
    QMetaObject::invokeMethod(guiMainThread,
      "createWindow",
      Qt::AutoConnection,
      Q_ARG(QString, QString(name.c_str())),
      Q_ARG(int, params));
  }

  return icvFindWindowByName(name.c_str());
}


//////////////////////////////////////////////////////
// GuiReceiver


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


double GuiReceiver::getRatioWindow(QString name)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name);

  if (!w)
    return -1;

  return w->getRatio();
}


void GuiReceiver::setRatioWindow(QString name, double arg2)
{
  QPointer<MatrixViewer> w = icvFindWindowByName(name.toLatin1().data());

  if (!w)
    return;

  int flags = (int)arg2;

  w->setRatio(flags);
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
      Qt::BlockingQueuedConnection,
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


void GuiReceiver::destroyAllWindow()
{
  if (!qApp)
    CV_Error(CV_StsNullPtr, "NULL session handler");

  if (multiThreads)
  {
    // WARNING: this could even close windows from an external parent app
    //#TODO check externalQAppExists and in case it does, close windows carefully,
    //      i.e. apply the className-check from below...
    qApp->closeAllWindows();
  }
  else
  {
    bool isWidgetDeleted = true;
    while (isWidgetDeleted)
    {
      isWidgetDeleted = false;
      QWidgetList list = QApplication::topLevelWidgets();
      for (int i = 0; i < list.count(); i++)
      {
        QObject *obj = list.at(i);
        if (obj->metaObject()->className() == QString("CvWindow"))
        {
          delete obj;
          isWidgetDeleted = true;
          break;
        }
      }
    }
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

//////////////////////////////////////////////////////
// CvWinProperties


//here CvWinProperties class
ToolsWindow::ToolsWindow(QString name_paraWindow, QObject* /*parent*/)
{
  //setParent(parent);
  setWindowFlags(Qt::Tool);
  setContentsMargins(0, 0, 0, 0);
  setWindowTitle(name_paraWindow);
  setObjectName(name_paraWindow);
  resize(100, 50);

  myLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  myLayout->setObjectName(QString::fromUtf8("boxLayout"));
  myLayout->setContentsMargins(0, 0, 0, 0);
  myLayout->setSpacing(0);
  myLayout->setMargin(0);
  myLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(myLayout);

  hide();
}

void ToolsWindow::addWidget(QWidget* obj)
{
  myLayout->addWidget(obj);
}

void ToolsWindow::closeEvent(QCloseEvent* e)
{
  e->accept(); //intersept the close event (not sure I really need it)
  //an hide event is also sent. I will intercept it and do some processing
}


void ToolsWindow::showEvent(QShowEvent* evnt)
{
  //why -1,-1 ?: do this trick because the first time the code is run,
  //no value pos was saved so we let Qt move the window in the middle of its parent (event ignored).
  //then hide will save the last position and thus, we want to retreive it (event accepted).
  QPoint mypos(-1, -1);
  QSettings settings("OpenCV2", windowTitle());
  mypos = settings.value("pos", mypos).toPoint();

  if (mypos.x() >= 0)
  {
    move(mypos);
    evnt->accept();
  }
  else
  {
    evnt->ignore();
  }
}


void ToolsWindow::hideEvent(QHideEvent* evnt)
{
  QSettings settings("OpenCV2", windowTitle());
  settings.setValue("pos", pos()); //there is an offset of 6 pixels (so the window's position is wrong -- why ?)
  evnt->accept();
}


ToolsWindow::~ToolsWindow()
{
  //clear the setting pos
  QSettings settings("OpenCV2", windowTitle());
  settings.remove("pos");
}



//////////////////////////////////////////////////////
// CvWindow


MatrixViewer::MatrixViewer(QString name, int arg2)
{
  color_choose = NULL;
  myTools = NULL;
  pencilSize = NULL;
  pencil_mode = false;
  moveToThread(qApp->instance()->thread());

  param_flags = arg2 & 0x0000000F;
  param_gui_mode = arg2 & 0x000000F0;
  param_ratio_mode = arg2 & 0x00000F00;
  param_creation_mode = arg2 & _WINDOW_MATRIX_CREATION_MODE;

  setContentsMargins(0, 0, 0, 0);
  setWindowTitle(name);
  setObjectName(name);

  setFocus(Qt::PopupFocusReason); //#1695 arrow keys are not recieved without the explicit focus

  resize(400, 300);
  setMinimumSize(1, 1);

  //1: create control panel
  //if (!myTools)
  //  myTools = createParameterWindow();

  //2: Layouts
  myBarLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  myBarLayout->setObjectName(QString::fromUtf8("barLayout"));
  myBarLayout->setContentsMargins(0, 0, 0, 0);
  myBarLayout->setSpacing(0);
  myBarLayout->setMargin(0);
  createGlobalLayout();

  //3: my view
  if (arg2 & CV_WINDOW_OPENGL)
    CV_Error(CV_OpenGlNotSupported, "Library was built without OpenGL support");
  mode_display = CV_MODE_NORMAL;
  createView();

  setAttribute(Qt::WA_DeleteOnClose, param_creation_mode==0); //in other case, does not release memory

  //4: shortcuts and actions
  //5: toolBar and statusbar
  if (param_gui_mode == CV_GUI_EXPANDED)
  {
    createActions();
    createShortcuts();

    myStatusBar = new QStatusBar(this);
    myStatusBar->setSizeGripEnabled(false);
    myStatusBar->setFixedHeight(20);
    myStatusBar->setMinimumWidth(1);
    myStatusBar_msg = new QLabel;

    myStatusBar_msg->setAlignment(Qt::AlignHCenter);
    myStatusBar->addWidget(myStatusBar_msg);

    myToolBar = new QToolBar(this);
    myToolBar->setFloatable(false); //is not a window
    myToolBar->setFixedHeight(28);
    myToolBar->setMinimumWidth(1);

    foreach(QAction *a, vect_QActions)
      myToolBar->addAction(a);

    if (param_creation_mode == 0)
    {
      //hide load and edit buttons:
      vect_QActions[__ACT_IMGRAPH_LOAD]->setVisible(false);
      vect_QActions[__ACT_IMGRAPH_PEN_EDIT]->setVisible(false);
    }

    myToolBar->setStyleSheet("QPushButton#ColorPick { background: none; color: black; }");
  }

  //Now attach everything
  if (myToolBar)
    myGlobalLayout->addWidget(myToolBar, Qt::AlignCenter);

  myGlobalLayout->addWidget(myView->getWidget(), Qt::AlignCenter);
  
  myGlobalLayout->addLayout(myBarLayout, Qt::AlignCenter);

  if (param_creation_mode != 0)
  {
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); 
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    myGlobalLayout->addWidget(buttonBox);
  }

  if (myStatusBar)
    myGlobalLayout->addWidget(myStatusBar, Qt::AlignCenter);

  setLayout(myGlobalLayout);
  show();
}


MatrixViewer::~MatrixViewer()
{
  if (myTools != NULL)
  {
    delete myTools;
    myTools = NULL;
  }
  _cond_waitEnd.notify_all();
}


void MatrixViewer::waitEnd()
{
  boost::unique_lock<boost::mutex> lock(_mtx);
  _cond_waitEnd.wait(lock);//wait for end...
}

void MatrixViewer::writeSettings()
{
  //organisation and application's name
  QSettings settings("OpenCV2", QFileInfo(QApplication::applicationFilePath()).fileName());

  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("mode_resize", param_flags);
  settings.setValue("mode_gui", param_gui_mode);

  myView->writeSettings(settings);

}



//TODO: load CV_GUI flag (done) and act accordingly (create win property if needed and attach trackbars)
void MatrixViewer::readSettings()
{
  //organisation and application's name
  QSettings settings("OpenCV2", QFileInfo(QApplication::applicationFilePath()).fileName());

  QPoint _pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize _size = settings.value("size", QSize(400, 400)).toSize();

  param_flags = settings.value("mode_resize", param_flags).toInt();
  param_gui_mode = settings.value("mode_gui", param_gui_mode).toInt();

  param_flags = settings.value("mode_resize", param_flags).toInt();

  myView->readSettings(settings);

  resize(_size);
  move(_pos);

}


double MatrixViewer::getRatio()
{
  return myView->getRatio();
}


void MatrixViewer::setRatio(int flags)
{
  myView->setRatio(flags);
}


int MatrixViewer::getPropWindow()
{
  return param_flags;
}


void MatrixViewer::setPropWindow(int flags)
{
  if (param_flags == flags) //nothing to do
    return;

  switch (flags)
  {
  case CV_WINDOW_NORMAL:
    myGlobalLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    param_flags = flags;

    break;

  case CV_WINDOW_AUTOSIZE:
    myGlobalLayout->setSizeConstraint(QLayout::SetFixedSize);
    param_flags = flags;

    break;

  default:
    ;
  }
}


void MatrixViewer::toggleFullScreen(int flags)
{
  if (isFullScreen() && flags == CV_WINDOW_NORMAL)
  {
    showTools();
    showNormal();
    return;
  }

  if (!isFullScreen() && flags == CV_WINDOW_FULLSCREEN)
  {
    hideTools();
    showFullScreen();
    return;
  }
}


void MatrixViewer::updateImage(cv::Mat arr)
{
  myView->updateImage(arr);
}


void MatrixViewer::displayInfo(QString text, int delayms)
{
  myView->startDisplayInfo(text, delayms);
}


void MatrixViewer::displayStatusBar(QString text, int delayms)
{
  if (myStatusBar)
    myStatusBar->showMessage(text, delayms);
}


void MatrixViewer::enablePropertiesButton()
{
  if (!vect_QActions.empty())
    vect_QActions[9]->setDisabled(false);
}


void MatrixViewer::setViewportSize(QSize _size)
{
  myView->getWidget()->resize(_size);
  myView->setSize(_size);
}


void MatrixViewer::createGlobalLayout()
{
  myGlobalLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  myGlobalLayout->setObjectName(QString::fromUtf8("boxLayout"));
  myGlobalLayout->setContentsMargins(0, 0, 0, 0);
  myGlobalLayout->setSpacing(0);
  myGlobalLayout->setMargin(0);
  setMinimumSize(1, 1);

  if (param_flags == CV_WINDOW_AUTOSIZE)
    myGlobalLayout->setSizeConstraint(QLayout::SetFixedSize);
  else if (param_flags == CV_WINDOW_NORMAL)
    myGlobalLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
}


void MatrixViewer::createView()
{
    myView = new DefaultViewPort(this, param_ratio_mode);
}


void MatrixViewer::createActions()
{
  vect_QActions.resize(10);

  QWidget* view = myView->getWidget();

  //if the shortcuts are changed in window_QT.h, we need to update the tooltip manually
  vect_QActions[__ACT_IMGRAPH_LEFT] = new QAction(QIcon(":/left-icon"), "Panning left (CTRL+arrowLEFT)", this);
  vect_QActions[__ACT_IMGRAPH_LEFT]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_LEFT], SIGNAL(triggered()), view, SLOT(siftWindowOnLeft()));

  vect_QActions[__ACT_IMGRAPH_RIGHT] = new QAction(QIcon(":/right-icon"), "Panning right (CTRL+arrowRIGHT)", this);
  vect_QActions[__ACT_IMGRAPH_RIGHT]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_RIGHT], SIGNAL(triggered()), view, SLOT(siftWindowOnRight()));

  vect_QActions[__ACT_IMGRAPH_UP] = new QAction(QIcon(":/up-icon"), "Panning up (CTRL+arrowUP)", this);
  vect_QActions[__ACT_IMGRAPH_UP]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_UP], SIGNAL(triggered()), view, SLOT(siftWindowOnUp()));

  vect_QActions[__ACT_IMGRAPH_DOWN] = new QAction(QIcon(":/down-icon"), "Panning down (CTRL+arrowDOWN)", this);
  vect_QActions[__ACT_IMGRAPH_DOWN]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_DOWN], SIGNAL(triggered()), view, SLOT(siftWindowOnDown()));

  vect_QActions[__ACT_IMGRAPH_ZOOM_X1] = new QAction(QIcon(":/zoom_x1-icon"), "Zoom x1 (CTRL+P)", this);
  vect_QActions[__ACT_IMGRAPH_ZOOM_X1]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_ZOOM_X1], SIGNAL(triggered()), view, SLOT(resetZoom()));

  vect_QActions[__ACT_IMGRAPH_ZOOM_IN] = new QAction(QIcon(":/zoom_in-icon"), "Zoom in (CTRL++)", this);
  vect_QActions[__ACT_IMGRAPH_ZOOM_IN]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_ZOOM_IN], SIGNAL(triggered()), view, SLOT(ZoomIn()));

  vect_QActions[__ACT_IMGRAPH_ZOOM_OUT] = new QAction(QIcon(":/zoom_out-icon"), "Zoom out (CTRL+-)", this);
  vect_QActions[__ACT_IMGRAPH_ZOOM_OUT]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_ZOOM_OUT], SIGNAL(triggered()), view, SLOT(ZoomOut()));

  vect_QActions[__ACT_IMGRAPH_SAVE] = new QAction(QIcon(":/save-icon"), "Save current image (CTRL+S)", this);
  vect_QActions[__ACT_IMGRAPH_SAVE]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_SAVE], SIGNAL(triggered()), view, SLOT(saveView()));

  vect_QActions[__ACT_IMGRAPH_LOAD] = new QAction(QIcon(":/load-icon"), "Load new image (CTRL+O)", this);
  vect_QActions[__ACT_IMGRAPH_LOAD]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_LOAD], SIGNAL(triggered()), view, SLOT(loadMatrix()));

  vect_QActions[__ACT_IMGRAPH_PEN_EDIT] = new QAction(QIcon(":/edit_pen-icon"), "Edit image (CTRL+E)", this);
  vect_QActions[__ACT_IMGRAPH_PEN_EDIT]->setIconVisibleInMenu(true);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_PEN_EDIT], SIGNAL(triggered()), this, SLOT(switchEditingImg()));
  /*
  vect_QActions[__ACT_IMGRAPH_PEN_COLOR] = new QAction("Color", this);
  vect_QActions[__ACT_IMGRAPH_PEN_COLOR]->setIconVisibleInMenu(false);
  QObject::connect(vect_QActions[__ACT_IMGRAPH_PEN_COLOR], SIGNAL(triggered()), this, SLOT(chooseColor()));*/
}


void MatrixViewer::createShortcuts()
{
  vect_QShortcuts.resize(11);

  QWidget* view = myView->getWidget();

  vect_QShortcuts[0] = new QShortcut(shortcut_panning_left, this);
  QObject::connect(vect_QShortcuts[0], SIGNAL(activated()), view, SLOT(siftWindowOnLeft()));

  vect_QShortcuts[1] = new QShortcut(shortcut_panning_right, this);
  QObject::connect(vect_QShortcuts[1], SIGNAL(activated()), view, SLOT(siftWindowOnRight()));

  vect_QShortcuts[2] = new QShortcut(shortcut_panning_up, this);
  QObject::connect(vect_QShortcuts[2], SIGNAL(activated()), view, SLOT(siftWindowOnUp()));

  vect_QShortcuts[3] = new QShortcut(shortcut_panning_down, this);
  QObject::connect(vect_QShortcuts[3], SIGNAL(activated()), view, SLOT(siftWindowOnDown()));

  vect_QShortcuts[4] = new QShortcut(shortcut_zoom_normal, this);
  QObject::connect(vect_QShortcuts[4], SIGNAL(activated()), view, SLOT(resetZoom()));

  vect_QShortcuts[5] = new QShortcut(shortcut_zoom_imgRegion, this);
  QObject::connect(vect_QShortcuts[5], SIGNAL(activated()), view, SLOT(imgRegion()));

  vect_QShortcuts[6] = new QShortcut(shortcut_zoom_in, this);
  QObject::connect(vect_QShortcuts[6], SIGNAL(activated()), view, SLOT(ZoomIn()));

  vect_QShortcuts[7] = new QShortcut(shortcut_zoom_out, this);
  QObject::connect(vect_QShortcuts[7], SIGNAL(activated()), view, SLOT(ZoomOut()));

  vect_QShortcuts[8] = new QShortcut(shortcut_save_img, this);
  QObject::connect(vect_QShortcuts[8], SIGNAL(activated()), view, SLOT(saveView()));

  vect_QShortcuts[9] = new QShortcut(shortcut_properties_win, this);
  QObject::connect(vect_QShortcuts[9], SIGNAL(activated()), this, SLOT(displayPropertiesWin()));

  vect_QShortcuts[10] = new QShortcut(shortcut_edit_pen_img, this);
  QObject::connect(vect_QShortcuts[10], SIGNAL(activated()), this, SLOT(switchEditingImg()));
}


void MatrixViewer::hideTools()
{
  if (myToolBar)
    myToolBar->hide();

  if (myStatusBar)
    myStatusBar->hide();

  if (myTools)
    myTools->hide();
}


void MatrixViewer::showTools()
{
  if (myToolBar)
    myToolBar->show();

  if (myStatusBar)
    myStatusBar->show();
}


ToolsWindow* MatrixViewer::createParameterWindow()
{
  if (myTools != NULL)
    return myTools;
  QString name_paraWindow = QFileInfo(QApplication::applicationFilePath()).fileName() + " settings";

  myTools = new ToolsWindow(name_paraWindow, guiMainThread);
  displayPropertiesWin();

  QFrame* mySeparator = new QFrame();
  mySeparator->setFrameShape(QFrame::HLine);

  if (color_choose != NULL)
    delete color_choose;
  if (pencilSize != NULL)
    delete pencilSize;

  QComboBox* matrixTypes = new QComboBox();
  matrixTypes->insertItem(0, "CV_8U");
  matrixTypes->insertItem(1, "CV_8S");
  matrixTypes->insertItem(2, "CV_16U");
  matrixTypes->insertItem(3, "CV_16S");
  matrixTypes->insertItem(4, "CV_32S");
  matrixTypes->insertItem(5, "CV_32F");
  matrixTypes->insertItem(6, "CV_64F");

  myTools->addWidget(new QLabel("Matrix data type:"));
  myTools->addWidget(matrixTypes);

  QWidget* labelsSize = new QWidget();
  QHBoxLayout* sizeLayout=new QHBoxLayout();
  labelsSize->setLayout(sizeLayout);

  sizeLayout->addWidget(new QLineEdit("3"));
  sizeLayout->addWidget(new QLineEdit("3"));
  sizeLayout->addWidget(new QLineEdit("1"));

  myTools->addWidget(new QLabel("Size (rows, cols, channels):"));
  myTools->addWidget(labelsSize);

  QComboBox* matrixInit = new QComboBox();
  matrixInit->insertItem(0, "zeros");
  matrixInit->insertItem(1, "constant");
  matrixInit->insertItem(2, "eye");
  matrixInit->insertItem(3, "circle");
  matrixInit->insertItem(4, "box");
  matrixInit->insertItem(5, "elipse");
  myTools->addWidget(new QLabel("Initial values:"));
  myTools->addWidget(matrixInit);

  myTools->addWidget(new QPushButton("Update"));

  pencilSize = new QLineEdit(lexical_cast<std::string>(myView->getPenSize()).c_str(), myTools);
  connect(pencilSize, SIGNAL(editingFinished()), this, SLOT(changePenSize()));
  color_choose = new QPushButton("Color", myTools);
  color_choose->setObjectName("ColorPick");
  QObject::connect(color_choose, SIGNAL(released()), this, SLOT(chooseColor()));

  myTools->addWidget(mySeparator);
  myTools->addWidget(new QLabel("Color:"));
  myTools->addWidget(color_choose);
  myTools->addWidget(new QLabel("Pencil size:"));
  myTools->addWidget(pencilSize);

  return myTools;
}


void MatrixViewer::displayPropertiesWin()
{
  if (myTools == NULL)
    return;
  if (myTools->isHidden())
    myTools->show();
  else
    myTools->hide();
}

void MatrixViewer::chooseColor()
{
  QColor tmpColor = QColorDialog::getColor(myView->getPenColor(), this);
  if (tmpColor.isValid())
  {
    myView->setPenColor(tmpColor);
    QString style = "background: rgb(%1, %2, %3);";
    myTools->setStyleSheet("QPushButton#ColorPick { " + style.arg(tmpColor.red()).arg(tmpColor.green()).arg(tmpColor.blue()) + "; color: black; }");
  }
}

void MatrixViewer::switchEditingImg()
{
  pencil_mode = !pencil_mode;
  if (pencil_mode)
  {
    if (myTools == NULL)
      myTools = createParameterWindow();
    myTools->setParent(this, Qt::Tool);
    myTools->show();
    myView->setCursor(Qt::CrossCursor);
    vect_QActions[__ACT_IMGRAPH_PEN_EDIT]->setIcon(QIcon(":/no_edit-icon"));
  }
  else
  {
    myTools->hide();
    myView->unsetCursor();
    vect_QActions[__ACT_IMGRAPH_PEN_EDIT]->setIcon(QIcon(":/edit_pen-icon"));
  }
}


void MatrixViewer::changePenSize()
{
  QLineEdit* s = dynamic_cast<QLineEdit*>(sender());
  if (s != NULL)
    myView->setPenSize(s->text().toFloat());
}

//Need more test here !
void MatrixViewer::keyPressEvent(QKeyEvent *evnt)
{
  //see http://doc.trolltech.com/4.6/qt.html#Key-enum
  int key = evnt->key();

  Qt::Key qtkey = static_cast<Qt::Key>(key);
  key = evnt->nativeVirtualKey(); //same codes as returned by GTK-based backend

  //control plus (Z, +, -, up, down, left, right) are used for zoom/panning functions
  if (evnt->modifiers() != Qt::ControlModifier)
  {
    mutexKey.lock();
    last_key = key;
    mutexKey.unlock();
    key_pressed.wakeAll();
    //evnt->accept();
  }
  
  // get focus widget:
  QLineEdit* focus = dynamic_cast<QLineEdit*>(this->focusWidget());
  if (focus)
  {
    return;
  }
  
  charliesoft::Window::getInstance()->event(evnt);
  //QWidget::keyPressEvent(evnt);
}

cv::Mat MatrixViewer::getMatrix()
{
  return myView->getMatrix();
}


//////////////////////////////////////////////////////
// DefaultViewPort


DefaultViewPort::DefaultViewPort(MatrixViewer* arg, int arg2) : QGraphicsView(arg)
{
  imgEditPixel_R = imgEditPixel_G = imgEditPixel_B = imgEditPixel_A = NULL;
  labelsShown = false;
  myPenColor = Qt::black;
  myPenWidth = 2;
  centralWidget = arg;
  param_keepRatio = arg2;

  setContentsMargins(0, 0, 0, 0);
  setMinimumSize(1, 1);
  setAlignment(Qt::AlignHCenter);

  setObjectName(QString::fromUtf8("graphicsView"));

  timerDisplay = new QTimer(this);
  timerDisplay->setSingleShot(true);
  connect(timerDisplay, SIGNAL(timeout()), this, SLOT(stopDisplayInfo()));

  drawInfo = false;
  positionGrabbing = QPoint(0, 0);
  positionCorners = QRect(0, 0, size().width(), size().height());

  mouseCoordinate = QPoint(-1, -1);

  //no border
  setStyleSheet("QGraphicsView { border-style: none; }");

  image2Draw_mat = cv::Mat::zeros(viewport()->height(), viewport()->width(), CV_8UC3);

  nbChannelOriginImage = 0;

  setInteractive(false);
  setMouseTracking(true); //receive mouse event everytime
}


DefaultViewPort::~DefaultViewPort()
{
}


QWidget* DefaultViewPort::getWidget()
{
  return this;
}


void DefaultViewPort::writeSettings(QSettings& settings)
{
  settings.setValue("matrix_view.m11", param_matrixWorld.m11());
  settings.setValue("matrix_view.m12", param_matrixWorld.m12());
  settings.setValue("matrix_view.m13", param_matrixWorld.m13());
  settings.setValue("matrix_view.m21", param_matrixWorld.m21());
  settings.setValue("matrix_view.m22", param_matrixWorld.m22());
  settings.setValue("matrix_view.m23", param_matrixWorld.m23());
  settings.setValue("matrix_view.m31", param_matrixWorld.m31());
  settings.setValue("matrix_view.m32", param_matrixWorld.m32());
  settings.setValue("matrix_view.m33", param_matrixWorld.m33());
}


void DefaultViewPort::readSettings(QSettings& settings)
{
  qreal m11 = settings.value("matrix_view.m11", param_matrixWorld.m11()).toDouble();
  qreal m12 = settings.value("matrix_view.m12", param_matrixWorld.m12()).toDouble();
  qreal m13 = settings.value("matrix_view.m13", param_matrixWorld.m13()).toDouble();
  qreal m21 = settings.value("matrix_view.m21", param_matrixWorld.m21()).toDouble();
  qreal m22 = settings.value("matrix_view.m22", param_matrixWorld.m22()).toDouble();
  qreal m23 = settings.value("matrix_view.m23", param_matrixWorld.m23()).toDouble();
  qreal m31 = settings.value("matrix_view.m31", param_matrixWorld.m31()).toDouble();
  qreal m32 = settings.value("matrix_view.m32", param_matrixWorld.m32()).toDouble();
  qreal m33 = settings.value("matrix_view.m33", param_matrixWorld.m33()).toDouble();

  param_matrixWorld = QTransform(m11, m12, m13, m21, m22, m23, m31, m32, m33);
}


double DefaultViewPort::getRatio()
{
  return param_keepRatio;
}


void DefaultViewPort::setRatio(int flags)
{
  if (getRatio() == flags) //nothing to do
    return;

  //if valid flags
  if (flags == CV_WINDOW_FREERATIO || flags == CV_WINDOW_KEEPRATIO)
  {
    centralWidget->param_ratio_mode = flags;
    param_keepRatio = flags;
    updateGeometry();
    viewport()->update();
  }
}


void DefaultViewPort::updateImage(const cv::Mat arr)
{
  CV_Assert(!arr.empty());
  image_copy = arr.clone();
  nbChannelOriginImage = image_copy.channels();
  image2Draw_mat = MatrixConvertor::convert(image_copy, CV_8UC3);

  cv::cvtColor(image2Draw_mat, image2Draw_mat, cv::COLOR_BGR2RGB);

  image2Draw_qt = QImage(image2Draw_mat.ptr<const uchar>(), image2Draw_mat.cols, image2Draw_mat.rows, image2Draw_mat.step, QImage::Format_RGB888);

  //use to compute mouse coordinate, I need to update the ratio here and in resizeEvent
  ratioX = width() / float(image2Draw_mat.cols);
  ratioY = height() / float(image2Draw_mat.rows);

  updateGeometry();

  viewport()->update();
}


void DefaultViewPort::startDisplayInfo(QString text, int delayms)
{
  if (timerDisplay->isActive())
    stopDisplayInfo();

  infoText = text;
  if (delayms > 0) timerDisplay->start(delayms);
  drawInfo = true;
}


//Note: move 2 percent of the window
void DefaultViewPort::siftWindowOnLeft()
{
  float delta = static_cast<float>(2 * width() / (100.0f * param_matrixWorld.m11()));
  moveView(QPointF(delta, 0));
}


//Note: move 2 percent of the window
void DefaultViewPort::siftWindowOnRight()
{
  float delta = static_cast<float>(-2 * width() / (100.0f * param_matrixWorld.m11()));
  moveView(QPointF(delta, 0));
}


//Note: move 2 percent of the window
void DefaultViewPort::siftWindowOnUp()
{
  float delta = static_cast<float>(2 * height() / (100.0f * param_matrixWorld.m11()));
  moveView(QPointF(0, delta));
}


//Note: move 2 percent of the window
void DefaultViewPort::siftWindowOnDown()
{
  float delta = static_cast<float>(-2 * height() / (100.0f * param_matrixWorld.m11()));
  moveView(QPointF(0, delta));
}


void DefaultViewPort::imgRegion()
{
  scaleView((threshold_zoom_img_region / param_matrixWorld.m11() - 1) * 5, QPointF(size().width() / 2, size().height() / 2));
}


void DefaultViewPort::resetZoom()
{
  param_matrixWorld.reset();
  controlImagePosition();
}


void DefaultViewPort::ZoomIn()
{
  scaleView(0.5, QPointF(size().width() / 2, size().height() / 2));
}


void DefaultViewPort::ZoomOut()
{
  scaleView(-0.5, QPointF(size().width() / 2, size().height() / 2));
}

void DefaultViewPort::loadMatrix()
{
  QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(),
    "matrices (*.bmp *.pbm *.pgm *.ppm *.sr *.ras *.jpeg *.jpg *.jpe *.jp2 *.tiff *.tif *.png)");
  if (!fileName.isEmpty())
  {
    Mat img = imread(fileName.toStdString());
    updateImage(img);
  }
}

//can save as JPG, JPEG, BMP, PNG
void DefaultViewPort::saveView()
{
  QDate date_d = QDate::currentDate();
  QString date_s = date_d.toString("dd.MM.yyyy");
  QString name_s = centralWidget->windowTitle() + "_screenshot_" + date_s;

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(name_s), name_s + ".png", tr("Images (*.png *.jpg *.bmp *.jpeg)"));

  if (!fileName.isEmpty()) //save the picture
  {
    QString extension = fileName.right(3);

    // Create a new pixmap to render the viewport into
    QPixmap viewportPixmap(viewport()->size());
    viewport()->render(&viewportPixmap);

    // Save it..
    if (QString::compare(extension, "png", Qt::CaseInsensitive) == 0)
    {
      viewportPixmap.save(fileName, "PNG");
      return;
    }

    if (QString::compare(extension, "jpg", Qt::CaseInsensitive) == 0)
    {
      viewportPixmap.save(fileName, "JPG");
      return;
    }

    if (QString::compare(extension, "bmp", Qt::CaseInsensitive) == 0)
    {
      viewportPixmap.save(fileName, "BMP");
      return;
    }

    if (QString::compare(extension, "jpeg", Qt::CaseInsensitive) == 0)
    {
      viewportPixmap.save(fileName, "JPEG");
      return;
    }

    CV_Error(CV_StsNullPtr, "file extension not recognized, please choose between JPG, JPEG, BMP or PNG");
  }
}


void DefaultViewPort::contextMenuEvent(QContextMenuEvent* evnt)
{
  if (centralWidget->vect_QActions.size() > 0)
  {
    QMenu menu(this);

    foreach(QAction *a, centralWidget->vect_QActions)
      menu.addAction(a);

    menu.exec(evnt->globalPos());
  }
}


void DefaultViewPort::resizeEvent(QResizeEvent* evnt)
{
  controlImagePosition();

  //use to compute mouse coordinate, I need to update the ratio here and in resizeEvent
  ratioX = width() / float(image2Draw_mat.cols);
  ratioY = height() / float(image2Draw_mat.rows);

  if (param_keepRatio == CV_WINDOW_KEEPRATIO)//to keep the same aspect ratio
  {
    QSize newSize = QSize(image2Draw_mat.cols, image2Draw_mat.rows);
    newSize.scale(evnt->size(), Qt::KeepAspectRatio);

    //imageWidth/imageHeight = newWidth/newHeight +/- epsilon
    //ratioX = ratioY +/- epsilon
    //||ratioX - ratioY|| = epsilon
    if (fabs(ratioX - ratioY) * 100 > ratioX) //avoid infinity loop / epsilon = 1% of ratioX
    {
      resize(newSize);

      //move to the middle
      //newSize get the delta offset to place the picture in the middle of its parent
      newSize = (evnt->size() - newSize) / 2;

      //if the toolbar is displayed, avoid drawing myview on top of it
      if (centralWidget->myToolBar)
        if (!centralWidget->myToolBar->isHidden())
          newSize += QSize(0, centralWidget->myToolBar->height());

      move(newSize.width(), newSize.height());
    }
  }

  return QGraphicsView::resizeEvent(evnt);
}

void DefaultViewPort::drawLineTo(const QPointF &endPoint)
{
  if (image_copy.depth() != CV_8U)
    return;//can't draw on math matrix (only on images...)
  QPainter painter(&image2Draw_qt);
  QColor tmpColor = myPenColor;
  painter.setPen(QPen(tmpColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
    Qt::RoundJoin));
  painter.drawLine(lastPoint, endPoint);

  int rad = static_cast<int>((myPenWidth / 2) + 2);
  update(QRect(lastPoint.toPoint(), endPoint.toPoint()).normalized()
    .adjusted(-rad, -rad, +rad, +rad));
  lastPoint = endPoint;
}

void DefaultViewPort::wheelEvent(QWheelEvent* evnt)
{
  scaleView(evnt->delta() / 240.0, evnt->pos());
  viewport()->update();
}

QPoint DefaultViewPort::toImgCoord(QPointF src)
{
  double pixel_width = param_matrixWorld.m11()*ratioX;
  double pixel_height = param_matrixWorld.m11()*ratioY;

  double pointDx = src.x() / pixel_width;
  double pointDy = src.y() / pixel_height;

  qreal offsetX = param_matrixWorld.dx() / pixel_width;
  qreal offsetY = param_matrixWorld.dy() / pixel_height;

  return QPoint((int)(pointDx - offsetX), (int)(pointDy - offsetY));
}

void DefaultViewPort::mousePressEvent(QMouseEvent* evnt)
{
  int cv_event = -1, flags = 0;
  QPoint pt = evnt->pos();

  //icvmouseHandler: pass parameters for cv_event, flags
  icvmouseProcessing(QPointF(pt), cv_event, flags);

  positionGrabbing = evnt->pos();
  if (centralWidget->isPencil_mode())
  {
    lastPoint = toImgCoord(positionGrabbing);
  } 
  else
  {
    if (param_matrixWorld.m11() > 1)
    {
      if (!centralWidget->isPencil_mode())
        setCursor(Qt::ClosedHandCursor);
    }
  }

  QWidget::mousePressEvent(evnt);
}

void DefaultViewPort::mouseReleaseEvent(QMouseEvent* evnt)
{
  int cv_event = -1, flags = 0;
  QPoint pt = evnt->pos();

  //icvmouseHandler: pass parameters for cv_event, flags
  icvmouseProcessing(QPointF(pt), cv_event, flags);

  if (param_matrixWorld.m11() > 1 && !centralWidget->isPencil_mode())
    setCursor(Qt::OpenHandCursor);

  if (centralWidget->isPencil_mode() && labelsShown)
  {
    pixelEdit = toImgCoord(pt);
    canalEdit = 0;

    updateEdits = true;

    if (imgEditPixel_R == NULL)
    {
      imgEditPixel_R = new QLineEdit("0", this);
      connect(imgEditPixel_R, SIGNAL(editingFinished()), this, SLOT(updateImage()));
    }
    imgEditPixel_R->setAlignment(Qt::AlignHCenter);

    if (nbChannelOriginImage > 1)
    {
      if (imgEditPixel_G == NULL)
      {
        imgEditPixel_G = new QLineEdit("0", this);
        connect(imgEditPixel_G, SIGNAL(editingFinished()), this, SLOT(updateImage()));
      }
      if (imgEditPixel_B == NULL)
      {
        imgEditPixel_B = new QLineEdit("0", this);
        connect(imgEditPixel_B, SIGNAL(editingFinished()), this, SLOT(updateImage()));
      }
      imgEditPixel_G->setAlignment(Qt::AlignHCenter);
      imgEditPixel_B->setAlignment(Qt::AlignHCenter);
    }
    if (nbChannelOriginImage > 3)
    {
      if (imgEditPixel_A == NULL)
      {
        imgEditPixel_A = new QLineEdit("0", this);
        connect(imgEditPixel_A, SIGNAL(editingFinished()), this, SLOT(updateImage()));
      }
      imgEditPixel_A->setAlignment(Qt::AlignHCenter);
    }
  }
  
  QWidget::mouseReleaseEvent(evnt);
}


void DefaultViewPort::mouseDoubleClickEvent(QMouseEvent* evnt)
{
  int cv_event = -1, flags = 0;
  QPoint pt = evnt->pos();

  //icvmouseHandler: pass parameters for cv_event, flags
  icvmouseProcessing(QPointF(pt), cv_event, flags);

  QWidget::mouseDoubleClickEvent(evnt);
}


void DefaultViewPort::mouseMoveEvent(QMouseEvent* evnt)
{
  int cv_event = CV_EVENT_MOUSEMOVE, flags = 0;
  QPoint pt = evnt->pos();

  //icvmouseHandler: pass parameters for cv_event, flags
  icvmouseProcessing(QPointF(pt), cv_event, flags);

  if (centralWidget->isPencil_mode())
  {
    if (evnt->buttons() & Qt::LeftButton)
      drawLineTo(toImgCoord(pt));
  }
  else
  {
    if (param_matrixWorld.m11() > 1 && evnt->buttons() == Qt::LeftButton)
    {
      QPointF dxy = (pt - positionGrabbing) / param_matrixWorld.m11();
      positionGrabbing = evnt->pos();
      moveView(dxy);
    }
  }

  //I update the statusbar here because if the user does a cvWaitkey(0) (like with inpaint.cpp)
  //the status bar will only be repaint when a click occurs.
  if (centralWidget->myStatusBar)
    viewport()->update();

  QWidget::mouseMoveEvent(evnt);
}


void DefaultViewPort::paintEvent(QPaintEvent* evnt)
{
  QPainter myPainter(viewport());
  myPainter.setWorldTransform(param_matrixWorld);

  draw2D(&myPainter);

  //Now disable matrixWorld for overlay display
  myPainter.setWorldMatrixEnabled(false);

  //overlay pixel values if zoomed in far enough
  if (param_matrixWorld.m11()*ratioX >= threshold_zoom_img_region &&
    param_matrixWorld.m11()*ratioY >= threshold_zoom_img_region)
  {
    labelsShown = true;
    if (centralWidget->isPencil_mode())
    {
      if (imgEditPixel_R != NULL)imgEditPixel_R->show();
      if (imgEditPixel_G != NULL)imgEditPixel_G->show();
      if (imgEditPixel_B != NULL)imgEditPixel_B->show();
      if (imgEditPixel_A != NULL)imgEditPixel_A->show();
    }
    drawImgRegion(&myPainter);
  }
  else
  {
    labelsShown = false;
    if (imgEditPixel_R != NULL)imgEditPixel_R->hide();
    if (imgEditPixel_G != NULL)imgEditPixel_G->hide();
    if (imgEditPixel_B != NULL)imgEditPixel_B->hide();
    if (imgEditPixel_A != NULL)imgEditPixel_A->hide();
  }

  //in mode zoom/panning
  if (param_matrixWorld.m11() > 1)
  {
    drawViewOverview(&myPainter);
  }

  //for information overlay
  if (drawInfo)
    drawInstructions(&myPainter);

  //for statusbar
  if (centralWidget->myStatusBar)
    drawStatusBar();

  QGraphicsView::paintEvent(evnt);
}


void DefaultViewPort::stopDisplayInfo()
{
  timerDisplay->stop();
  drawInfo = false;
}


inline bool DefaultViewPort::isSameSize(IplImage* img1, IplImage* img2)
{
  return img1->width == img2->width && img1->height == img2->height;
}


void DefaultViewPort::controlImagePosition()
{
  qreal left, top, right, bottom;

  //after check top-left, bottom right corner to avoid getting "out" during zoom/panning
  param_matrixWorld.map(0, 0, &left, &top);

  if (left > 0)
  {
    param_matrixWorld.translate(-left, 0);
    left = 0;
  }
  if (top > 0)
  {
    param_matrixWorld.translate(0, -top);
    top = 0;
  }
  //-------

  QSize sizeImage = size();
  param_matrixWorld.map(sizeImage.width(), sizeImage.height(), &right, &bottom);
  if (right < sizeImage.width())
  {
    param_matrixWorld.translate(sizeImage.width() - right, 0);
    right = sizeImage.width();
  }
  if (bottom < sizeImage.height())
  {
    param_matrixWorld.translate(0, sizeImage.height() - bottom);
    bottom = sizeImage.height();
  }

  //save corner position
  positionCorners.setTopLeft(QPoint((int)left, (int)top));
  positionCorners.setBottomRight(QPoint((int)right, (int)bottom));
  //save also the inv matrix
  matrixWorld_inv = param_matrixWorld.inverted();

  //viewport()->update();
}

void DefaultViewPort::moveView(QPointF delta)
{
  param_matrixWorld.translate(delta.x(), delta.y());
  controlImagePosition();
  viewport()->update();
}

//factor is -0.5 (zoom out) or 0.5 (zoom in)
void DefaultViewPort::scaleView(qreal factor, QPointF center)
{
  factor /= 5;//-0.1 <-> 0.1
  factor += 1;//0.9 <-> 1.1

  //limit zoom out ---
  if (param_matrixWorld.m11() == 1 && factor < 1)
    return;

  if (param_matrixWorld.m11()*factor < 1)
    factor = 1 / param_matrixWorld.m11();


  //limit zoom int ---
  if (param_matrixWorld.m11()>100 && factor > 1)
    return;

  //inverse the transform
  int a, b;
  matrixWorld_inv.map((int)center.x(), (int)center.y(), &a, &b);

  param_matrixWorld.translate(a - factor*a, b - factor*b);
  param_matrixWorld.scale(factor, factor);

  controlImagePosition();

  //display new zoom
  if (centralWidget->myStatusBar)
    centralWidget->displayStatusBar(tr("Zoom: %1%").arg(param_matrixWorld.m11() * 100), 1000);

  if (param_matrixWorld.m11() > 1 && !centralWidget->isPencil_mode())
    setCursor(Qt::OpenHandCursor);
  else
  {
    if (!centralWidget->isPencil_mode())
      unsetCursor();
    else
      setCursor(Qt::CrossCursor);
  }
}


void DefaultViewPort::icvmouseProcessing(QPointF pt, int cv_event, int flags)
{
  //to convert mouse coordinate
  qreal pfx, pfy;
  matrixWorld_inv.map(pt.x(), pt.y(), &pfx, &pfy);

  mouseCoordinate.rx() = static_cast<int>(floor(pfx / ratioX));
  mouseCoordinate.ry() = static_cast<int>(floor(pfy / ratioY));

}


QSize DefaultViewPort::sizeHint() const
{
  if (!image2Draw_mat.empty())
    return QSize(image2Draw_mat.cols, image2Draw_mat.rows);
  else
    return QGraphicsView::sizeHint();
}


void DefaultViewPort::draw2D(QPainter *painter)
{
  image2Draw_qt = QImage(image2Draw_mat.ptr<uchar>(), image2Draw_mat.cols, image2Draw_mat.rows, image2Draw_mat.step, QImage::Format_RGB888);
  painter->drawImage(QRect(0, 0, viewport()->width(), viewport()->height()), image2Draw_qt, QRect(0, 0, image2Draw_qt.width(), image2Draw_qt.height()));
}

//only if CV_8UC1 or CV_8UC3
void DefaultViewPort::drawStatusBar()
{
  if (nbChannelOriginImage != 1 && nbChannelOriginImage != 3)
    return;

  if (mouseCoordinate.x() >= 0 &&
    mouseCoordinate.y() >= 0 &&
    mouseCoordinate.x() < image2Draw_qt.width() &&
    mouseCoordinate.y() < image2Draw_qt.height())
    //  if (mouseCoordinate.x()>=0 && mouseCoordinate.y()>=0)
  {
    QRgb rgbValue = image2Draw_qt.pixel(mouseCoordinate);

    if (nbChannelOriginImage == 3)
    {
      centralWidget->myStatusBar_msg->setText(tr("<font color='black'>(x=%1, y=%2) ~ </font>")
        .arg(mouseCoordinate.x())
        .arg(mouseCoordinate.y()) +
        tr("<font color='red'>R:%3 </font>").arg(qRed(rgbValue)) +//.arg(value.val[0])+
        tr("<font color='green'>G:%4 </font>").arg(qGreen(rgbValue)) +//.arg(value.val[1])+
        tr("<font color='blue'>B:%5</font>").arg(qBlue(rgbValue))//.arg(value.val[2])
        );
    }

    if (nbChannelOriginImage == 1)
    {
      //all the channel have the same value (because of cvconvertimage), so only the r channel is dsplayed
      centralWidget->myStatusBar_msg->setText(tr("<font color='black'>(x=%1, y=%2) ~ </font>")
        .arg(mouseCoordinate.x())
        .arg(mouseCoordinate.y()) +
        tr("<font color='grey'>L:%3 </font>").arg(qRed(rgbValue))
        );
    }
  }
}

void DefaultViewPort::updateImage()
{
  QLineEdit* focusedEdit = dynamic_cast<QLineEdit*>(sender());
  if (focusedEdit == NULL)
    return;
  int nbChannel = 0;
  if (focusedEdit == imgEditPixel_R)
    nbChannel = 0;
  if (focusedEdit == imgEditPixel_G)
    nbChannel = 1;
  if (focusedEdit == imgEditPixel_B)
    nbChannel = 2;
  if (focusedEdit == imgEditPixel_A)
    nbChannel = 3;

  int x = pixelEdit.rx(), y = pixelEdit.ry();
  std::string valString = MatrixConvertor::printElem(image_copy, x, y, nbChannel);

  QRgb rgbValue = image2Draw_qt.pixel(pixelEdit);

  int newVal = 0;

  switch (image_copy.depth())
  {
  case CV_8U:
    newVal = image_copy.ptr<uchar>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toUInt();
    break;
  case CV_8S:
    newVal = image_copy.ptr<char>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toInt();
    break;
  case CV_16U:
    newVal = image_copy.ptr<ushort>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toUShort();
    break;
  case CV_16S:
    newVal = image_copy.ptr<short>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toShort();
    break;
  case CV_32S:
    newVal = image_copy.ptr<int>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toInt();
    break;
  case CV_32F:
    image_copy.ptr<float>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toFloat();
    newVal = (int)focusedEdit->text().toFloat();
    break;
  case CV_64F:
    image_copy.ptr<double>()[y*image_copy.step1() + x*image_copy.channels() + nbChannel] = focusedEdit->text().toDouble();
    newVal = (int)focusedEdit->text().toDouble();
    break;
  default:
    break;
  }

  if (newVal > 255)newVal = 255;
  if (newVal < 0)newVal = 0;
  if (focusedEdit == imgEditPixel_R)
  {
    if (nbChannelOriginImage > 1)
      rgbValue = qRgb(newVal, qGreen(rgbValue), qBlue(rgbValue));
    else
      rgbValue = qRgb(newVal, newVal, newVal);
  }
  if (focusedEdit == imgEditPixel_G)
    rgbValue = qRgb(qRed(rgbValue), newVal, qBlue(rgbValue));
  if (focusedEdit == imgEditPixel_B)
    rgbValue = qRgb(qRed(rgbValue), qGreen(rgbValue), newVal);
  if (focusedEdit == imgEditPixel_A)
    rgbValue = qRgba(qRed(rgbValue), qGreen(rgbValue), qBlue(rgbValue), newVal);

  image2Draw_qt.setPixel(pixelEdit, rgbValue);
}

//accept only CV_8UC1 and CV_8UC8 image for now
void DefaultViewPort::drawImgRegion(QPainter *painter)
{
  if (nbChannelOriginImage > 4)
    return;

  double pixel_width = param_matrixWorld.m11()*ratioX;
  double pixel_height = param_matrixWorld.m11()*ratioY;

  qreal offsetX = param_matrixWorld.dx() / pixel_width;
  offsetX = offsetX - floor(offsetX);
  qreal offsetY = param_matrixWorld.dy() / pixel_height;
  offsetY = offsetY - floor(offsetY);

  QSize view = size();
  QVarLengthArray<QLineF, 30> linesX;
  for (qreal _x = offsetX*pixel_width; _x < view.width(); _x += pixel_width)
    linesX.append(QLineF(_x, 0, _x, view.height()));

  QVarLengthArray<QLineF, 30> linesY;
  for (qreal _y = offsetY*pixel_height; _y < view.height(); _y += pixel_height)
    linesY.append(QLineF(0, _y, view.width(), _y));


  QFont f = painter->font();
  int original_font_size = f.pointSize();
  //change font size
  //f.setPointSize(4+(param_matrixWorld.m11()-threshold_zoom_img_region)/5);
  f.setPixelSize(static_cast<int>(10 + (pixel_height - threshold_zoom_img_region) / 5));
  painter->setFont(f);

  int dataSize = image_copy.elemSize1()*nbChannelOriginImage;
  uchar *dataPtr = image_copy.ptr<uchar>();

  for (int j = -1; j < height() / pixel_height; j++)//-1 because display the pixels top rows left columns
    for (int i = -1; i < width() / pixel_width; i++)//-1
    {
    // Calculate top left of the pixel's position in the viewport (screen space)
    QPointF pos_in_view((i + offsetX)*pixel_width, (j + offsetY)*pixel_height);

    // Calculate top left of the pixel's position in the image (image space)
    QPointF pos_in_image = matrixWorld_inv.map(pos_in_view);// Top left of pixel in view
    pos_in_image.rx() = pos_in_image.x() / ratioX;
    pos_in_image.ry() = pos_in_image.y() / ratioY;
    QPoint point_in_image(static_cast<int>(pos_in_image.x() + 0.5f), static_cast<int>(pos_in_image.y() + 0.5f));// Add 0.5 for rounding

    QRgb rgbValue;
    bool pointValid = image2Draw_qt.valid(point_in_image);
    if (pointValid)
      rgbValue = image2Draw_qt.pixel(point_in_image);
    else
      rgbValue = qRgb(0, 0, 0);

    for (int iChannel = 0; iChannel < nbChannelOriginImage; iChannel++)
    {
      QString val;
      if (pointValid)
      {
        int x = point_in_image.rx(), y = point_in_image.ry(), tmpChannel = iChannel;
        if (nbChannelOriginImage == 3)
          tmpChannel = nbChannelOriginImage - iChannel - 1;
        std::string valString = MatrixConvertor::printElem(image_copy, x, y, iChannel);
        val = valString.c_str();

        QLineEdit * editTmp;
        QColor penColor = Qt::white;
        {
          switch (iChannel)
          {
          case 0:
            penColor = Qt::red;
            editTmp = imgEditPixel_R;
            break;
          case 1:
            penColor = Qt::green;
            editTmp = imgEditPixel_G;
            break;
          case 2:
            penColor = Qt::blue;
            editTmp = imgEditPixel_B;
            break;
          default:
            penColor = Qt::black;
            editTmp = imgEditPixel_A;
            break;
          }
        }
        if (MatrixConvertor::getElem(image_copy, x, y, iChannel) < 64)
          penColor = Qt::white;
        painter->setPen(QPen(penColor, 1));

        painter->drawText(QRect((int)pos_in_view.x(), (int)(pos_in_view.y() + iChannel * pixel_height / nbChannelOriginImage),
          (int)pixel_width, (int)(pixel_height / nbChannelOriginImage)),
          Qt::AlignCenter, val);
        if (point_in_image == pixelEdit && editTmp != NULL)
        {
          editTmp->setGeometry(QRect((int)pos_in_view.x(), (int)(pos_in_view.y() + iChannel * pixel_height / nbChannelOriginImage),
            (int)pixel_width, (int)(pixel_height / nbChannelOriginImage)));
          if (updateEdits)
            editTmp->setText(val);
        }
      }
    }

  }

  painter->setPen(QPen(Qt::black, 1));
  painter->drawLines(linesX.data(), linesX.size());
  painter->drawLines(linesY.data(), linesY.size());

  //restore font size
  f.setPointSize(original_font_size);
  painter->setFont(f);
  updateEdits = false;
}

void DefaultViewPort::drawViewOverview(QPainter *painter)
{
  QSize viewSize = size();
  viewSize.scale(100, 100, Qt::KeepAspectRatio);

  const int margin = 5;

  //draw the image's location
  painter->setBrush(QColor(0, 0, 0, 127));
  painter->setPen(Qt::darkGreen);
  painter->drawRect(QRect(width() - viewSize.width() - margin, 0, viewSize.width(), viewSize.height()));

  //daw the view's location inside the image
  qreal ratioSize = 1 / param_matrixWorld.m11();
  qreal ratioWindow = (qreal)(viewSize.height()) / (qreal)(size().height());
  painter->setPen(Qt::darkBlue);
  painter->drawRect(QRectF(width() - viewSize.width() - positionCorners.left()*ratioSize*ratioWindow - margin,
    -positionCorners.top()*ratioSize*ratioWindow,
    (viewSize.width() - 1)*ratioSize,
    (viewSize.height() - 1)*ratioSize)
    );
}

void DefaultViewPort::drawInstructions(QPainter *painter)
{
  QFontMetrics metrics = QFontMetrics(font());
  int border = qMax(4, metrics.leading());

  QRect qrect = metrics.boundingRect(0, 0, width() - 2 * border, int(height()*0.125),
    Qt::AlignCenter | Qt::TextWordWrap, infoText);
  painter->setRenderHint(QPainter::TextAntialiasing);
  painter->fillRect(QRect(0, 0, width(), qrect.height() + 2 * border),
    QColor(0, 0, 0, 127));
  painter->setPen(Qt::white);
  painter->fillRect(QRect(0, 0, width(), qrect.height() + 2 * border),
    QColor(0, 0, 0, 127));

  painter->drawText((width() - qrect.width()) / 2, border,
    qrect.width(), qrect.height(),
    Qt::AlignCenter | Qt::TextWordWrap, infoText);
}


void DefaultViewPort::setSize(QSize /*size_*/)
{
}

cv::Mat DefaultViewPort::getMatrix()
{
  return image_copy;
}
