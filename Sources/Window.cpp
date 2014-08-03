
#include "Window.h"
#include "Internationalizator.h"

#include <QPaintEngine>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QtOpenGL/QGLWidget>
#include <QAction>
#include <QFileDialog>
#include <QApplication>
#include <QStatusBar>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QSizePolicy>
#include <QDebug>
#include <QDockWidget>
#include <QSplitter>
#include <QTextEdit>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "GraphicView.h"
#include "ProcessManager.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  Window* Window::ptr = NULL;
  recursive_mutex _windowMutex;

  //////////////////////////////////////////////////////////////////////////
  //////////////////Window///////////////////////////////////// 
  ////////////////////////////////////////////////////////////////////////// 

  Window* Window::getInstance()
  {
    lock_guard<recursive_mutex> guard(_windowMutex);//evite les problemes d'acces concurrent
    if (ptr == NULL)
      ptr = new Window();
    return ptr;
  }
  GraphRepresentation* Window::getGraphLayout()
  {
    return getInstance()->mainLayout_;
  }

  void Window::releaseInstance()
  {
    lock_guard<recursive_mutex> guard(_windowMutex);//evite les problemes d'acces concurrent
    if (ptr != NULL)
      delete ptr;
    ptr = NULL;//already set in destructor, but just in case processor want to do jokes...
  }

  Window::~Window()
  {
    ptr = NULL;
  }

  Window::Window()
  {
    //first load config file:
    config_ = new GlobalConfig();
    config_->loadConfig();

    model_ = new GraphOfProcess();

    menuFichier = menuBar()->addMenu(_QT("MENU_FILE"));

    QAction* openAct = new QAction(_QT("MENU_FILE_OPEN"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(_QT("MENU_FILE_OPEN_TIP"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
    menuFichier->addAction(openAct);
    QAction* createAct = new QAction(_QT("MENU_FILE_CREATE"), this);
    createAct->setShortcuts(QKeySequence::New);
    createAct->setStatusTip(_QT("MENU_FILE_CREATE_TIP"));
    connect(createAct, SIGNAL(triggered()), this, SLOT(newProject()));
    menuFichier->addAction(createAct);
    QAction* actionQuitter = new QAction(_QT("MENU_FILE_QUIT"), this);
    actionQuitter->setShortcuts(QKeySequence::Close);
    actionQuitter->setStatusTip(_QT("MENU_FILE_QUIT_TIP"));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(quitProg()));
    menuFichier->addAction(actionQuitter);

    menuAide = menuBar()->addMenu("?");
    menuAide->addAction(_QT("MENU_HELP_INFO"));
    menuAide->addAction(_QT("MENU_HELP_HELP"));
  }

  void Window::fillDock(int idDock)
  {
    std::vector<std::string> list = ProcessManager::getInstance()->getAlgos(AlgoType(idDock));
    QLabel* tmpLabel;
    auto it = list.begin();
    while (it != list.end())
    {
      tmpLabel = new QLabel(_QT(*it), this);
      keysName_[tmpLabel] = *it;
      tmpLabel->setAlignment(Qt::AlignCenter);
      tmpLabel->setStyleSheet("max-height:50px; border:2px solid #555;border-radius: 11px;background: qradialgradient(cx: 0.3, cy: -0.4, fx: 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #888);");
      docks_content_[idDock]->addWidget(tmpLabel);
      it++;
    }
  }

  void Window::show()
  {
    mainLayout_ = new GraphRepresentation();
    mainWidget_ = new MainWidget(model_);
    setCentralWidget(mainWidget_);
    mainWidget_->setLayout(mainLayout_);
    
    statusBar();

    tabWidget_ = new QTabWidget;
    dock_ = new QDockWidget(_QT("DOCK_TITLE"), this);
    dock_->setWidget(tabWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, dock_);
    dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    //create the 5 docks:
    for (int i = 0; i < 5; i++)
    {
      docks_content_.push_back(new QVBoxLayout());
      DraggableWidget *tmpWidget = new DraggableWidget(this);
      tmpWidget->setMaximumWidth(200);
      tmpWidget->setLayout(docks_content_[i]);
      docks_content_[i]->setAlignment(Qt::AlignTop);

      switch (i)
      {
      case 0:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_INPUT"));
        break;
      case 1:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_IMG_PROCESS"));
        break;
      case 2:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_SIGNAL"));
        break;
      case 3:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_MATH"));
        break;
      default:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_OUTPUT"));
      }

      fillDock(i);
    }

    
    if (config_->isMaximized)
      showMaximized();
    else
    {
      resize(config_->lastPosition.size()); move(config_->lastPosition.topLeft());
      QMainWindow::show();
    }

    connect(mainWidget_, SIGNAL(askSynchro(charliesoft::GraphOfProcess *)),
      mainLayout_, SLOT(synchronize(charliesoft::GraphOfProcess *)));
    connect(this, SIGNAL(askSynchro(charliesoft::GraphOfProcess *)),
      mainLayout_, SLOT(synchronize(charliesoft::GraphOfProcess *)));
  }

  void Window::mousePressEvent(QMouseEvent *event)
  {
    if (event->button() == Qt::RightButton)
    {
      Block* block = ProcessManager::getInstance()->createAlgoInstance("BlockLoader");
      model_->addNewProcess(block);
      emit askSynchro(model_);//as we updated the model, we ask the layout to redraw itself...
    }
    else if (event->button() == Qt::MidButton)
      std::cout << "middle mouse click " << std::endl;
  }

  void Window::closeEvent(QCloseEvent *event)
  {
    if (!quitProg())
      event->ignore();
    event->accept();
  }

  void Window::openFile()
  {
    QString file = QFileDialog::getOpenFileName(this, _QT("PROJ_LOAD_FILE"),
      config_->lastProject_.c_str(), _QT("CONF_FILE_TYPE"));
    if (!file.isEmpty())
    {
      config_->lastProject_ = file.toStdString();
      //load project...
    }

    mainLayout_->clearLayout();
    if (model_ != NULL)
      delete model_;
    model_ = new GraphOfProcess();
  }

  void Window::newProject()
  {
    string lastDirectory = config_->lastProject_;
    if (!lastDirectory.empty() && lastDirectory.find_last_of('/')!=string::npos)
      lastDirectory = lastDirectory.substr(0, lastDirectory.find_last_of('/'));

    QString file = QFileDialog::getSaveFileName(this, _QT("PROJ_CREATE_FILE"),
      lastDirectory.c_str(), _QT("CONF_FILE_TYPE"));
    config_->lastProject_ = file.toStdString();

    mainLayout_->clearLayout();
    if (model_ != NULL)
      delete model_; 
    model_ = new GraphOfProcess();
  }

  bool Window::quitProg()
  {
    config_->isMaximized = isMaximized();
    if (!config_->isMaximized)
      config_->lastPosition = QRect(pos(), size());
    config_->saveConfig();
    QApplication::quit();
    return true;
  }
  
  void Window::printHelp()
  {

  }

  void DraggableWidget::mousePressEvent(QMouseEvent *mouse)
  {
    if (mouse->button() == Qt::LeftButton)
    {
      //find widget below mouse:
      if (QWidget* widget = dynamic_cast<QWidget*>(childAt(mouse->pos())))
      {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(Window::getInstance()->getKey(widget).c_str());
        drag->setMimeData(mimeData);

        Qt::DropAction dropAction = drag->exec();
      }
    }
  }

}