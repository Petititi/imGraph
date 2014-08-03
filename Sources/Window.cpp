
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

  void Window::show()
  {
    mainLayout_ = new GraphRepresentation();
    mainWidget_ = new MainWidget(model_);
    setCentralWidget(mainWidget_);
    mainWidget_->setLayout(mainLayout_);
    
    statusBar();
    
    QWidget *dock_input_widget_ = new QWidget(this);
    QWidget *dock_img_widget_ = new QWidget(this);
    QWidget *dock_signal_widget_ = new QWidget(this);
    QWidget *dock_math_widget_ = new QWidget(this);
    QWidget *dock_output_widget_ = new QWidget(this);

    docks_content_.push_back(new QVBoxLayout());
    docks_content_.push_back(new QVBoxLayout());
    docks_content_.push_back(new QVBoxLayout());
    docks_content_.push_back(new QVBoxLayout());
    docks_content_.push_back(new QVBoxLayout());
    dock_input_widget_->setLayout(docks_content_[0]);
    dock_img_widget_->setLayout(docks_content_[1]);
    dock_signal_widget_->setLayout(docks_content_[2]);
    dock_math_widget_->setLayout(docks_content_[3]);
    dock_output_widget_->setLayout(docks_content_[4]);

    docks_.push_back( new QDockWidget(_QT("BLOCK_INPUT"), this) );
    docks_[0]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    docks_[0]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, docks_[0]);
    docks_[0]->setWidget(dock_input_widget_);

    docks_.push_back(new QDockWidget(_QT("BLOCK_IMG_PROCESS"), this));
    docks_[1]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    docks_[1]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, docks_[1]);
    docks_[1]->setWidget(dock_img_widget_);

    docks_.push_back(new QDockWidget(_QT("BLOCK_SIGNAL"), this));
    docks_[2]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    docks_[2]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, docks_[2]);
    docks_[2]->setWidget(dock_signal_widget_);

    docks_.push_back(new QDockWidget(_QT("BLOCK_MATH"), this));
    docks_[3]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    docks_[3]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, docks_[3]);
    docks_[3]->setWidget(dock_math_widget_);
    
    docks_.push_back(new QDockWidget(_QT("BLOCK_OUTPUT"), this));
    docks_[4]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    docks_[4]->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, docks_[4]);
    docks_[4]->setWidget(dock_output_widget_);

    tabifyDockWidget(docks_[0], docks_[1]);
    tabifyDockWidget(docks_[0], docks_[2]);
    tabifyDockWidget(docks_[0], docks_[3]);
    tabifyDockWidget(docks_[0], docks_[4]);
    
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
}