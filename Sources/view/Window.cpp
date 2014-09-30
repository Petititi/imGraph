
#include "Window.h"
#include "Internationalizator.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
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

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Graph.h"
#include "MatrixViewer.h"
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
    lock_guard<recursive_mutex> guard(_windowMutex);//for multi-thread access
    if (ptr != NULL)
      delete ptr;//ptr set to NULL in destructor
  }

  Window::~Window()
  {
    ptr = NULL;
  }

  Window::Window()
  {
    ptr = this;
    _model = new GraphOfProcess();
    //create opencv main thread:
    new GuiReceiver();

    //first load config file:
    GlobalConfig::getInstance()->loadConfig();

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
    DraggableWidget* tmpLabel;
    auto it = list.begin();
    while (it != list.end())
    {
      tmpLabel = new DraggableWidget(_QT(*it), this);
      keysName_[tmpLabel] = *it;
      docks_content_[idDock]->addWidget(tmpLabel);
      it++;
    }
  }

  void Window::show()
  {
    mainLayout_ = new GraphRepresentation();

    mainWidget_ = new MainWidget(_model);
    mainWidget_->setLayout(mainLayout_);
    setCentralWidget(mainWidget_);
    
    /*QScrollArea* scrollarea = new QScrollArea();
    scrollarea->setWidgetResizable(true);
    scrollarea->setWidget(mainWidget_);
    setCentralWidget(scrollarea);*/
    
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
      DraggableContainer *tmpWidget = new DraggableContainer(this);
      tmpWidget->setMaximumWidth(200);
      tmpWidget->setLayout(docks_content_[i]);
      docks_content_[i]->setAlignment(Qt::AlignTop);

      switch (i)
      {
      case 0:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_INPUT"));
        break;
      case 1:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_IMG_PROCESS"));
        break;
      case 2:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_SIGNAL"));
        break;
      case 3:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_MATH"));
        break;
      default:
        tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_OUTPUT"));
      }

      fillDock(i);
    }

    
    if (GlobalConfig::getInstance()->isMaximized)
      showMaximized();
    else
    {
      resize(GlobalConfig::getInstance()->lastPosition.size()); move(GlobalConfig::getInstance()->lastPosition.topLeft());
      QMainWindow::show();
    }

    connect(mainWidget_, SIGNAL(askSynchro(charliesoft::GraphOfProcess *)),
      mainLayout_, SLOT(synchronize(charliesoft::GraphOfProcess *)));
    connect(this, SIGNAL(askSynchro(charliesoft::GraphOfProcess *)),
      mainLayout_, SLOT(synchronize(charliesoft::GraphOfProcess *)));

    setStyleSheet(GlobalConfig::getInstance()->styleSheet_.c_str());

    mainLayout_->synchronize(_model);
  }

  void Window::mousePressEvent(QMouseEvent *event)
  {
    if (event->button() == Qt::RightButton)
    {
      //show a popup menu!
    }
    else if (event->button() == Qt::MidButton)
      std::cout << "middle mouse click " << std::endl;
  }

  bool Window::event(QEvent *event)
  {
    static bool startRun = true;
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
      if (key != NULL)
      {
        switch (key->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
          //Enter or return was pressed
          startRun = false;
          GraphOfProcess::pauseProcess = false;
          _model->run();
          break;
        case Qt::Key_End:
          startRun = true;
          GraphOfProcess::pauseProcess = false;
          _model->stop();
          break;
        case Qt::Key_Space:
          if (startRun)
          {
            startRun = false;
            _model->run();
          }
          else
            _model->switchPause();
          break;
        case Qt::Key_Delete:
          mainLayout_->removeSelectedLinks();
          for (auto rep : VertexRepresentation::getSelection())
            _model->deleteProcess(rep->getModel());
          VertexRepresentation::resetSelection();
          mainLayout_->synchronize(_model);
          break;
        default:
          return QMainWindow::event(event);
        }
        return true;
      }
    }
    return QMainWindow::event(event);
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
      GlobalConfig::getInstance()->lastProject_.c_str(), _QT("CONF_FILE_TYPE"));
    if (!file.isEmpty())
    {
      GlobalConfig::getInstance()->lastProject_ = file.toStdString();
      //load project...
    }

    mainLayout_->clearLayout();
    if (_model != NULL)
      delete _model;
    _model = new GraphOfProcess();
  }

  void Window::newProject()
  {
    string lastDirectory = GlobalConfig::getInstance()->lastProject_;
    if (!lastDirectory.empty() && lastDirectory.find_last_of('/')!=string::npos)
      lastDirectory = lastDirectory.substr(0, lastDirectory.find_last_of('/'));

    QString file = QFileDialog::getSaveFileName(this, _QT("PROJ_CREATE_FILE"),
      lastDirectory.c_str(), _QT("CONF_FILE_TYPE"));
    GlobalConfig::getInstance()->lastProject_ = file.toStdString();

    mainLayout_->clearLayout();
    if (_model != NULL)
      delete _model; 
    _model = new GraphOfProcess();
  }

  bool Window::quitProg()
  {
    GlobalConfig::getInstance()->isMaximized = isMaximized();
    if (!GlobalConfig::getInstance()->isMaximized)
      GlobalConfig::getInstance()->lastPosition = QRect(pos(), size());
    GlobalConfig::getInstance()->saveConfig();
    QApplication::quit();
    return true;
  }
  
  void Window::printHelp()
  {

  }

  void DraggableContainer::mousePressEvent(QMouseEvent *mouse)
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

  DraggableWidget::DraggableWidget(QString text, QWidget* p) :QLabel(_QT(text.toStdString()),p)
  {
    setObjectName("DraggableWidget");
    setAlignment(Qt::AlignCenter);
  };
}