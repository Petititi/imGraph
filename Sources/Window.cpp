
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

#include "GraphicView.h"
#include "window_QT.h"
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
    model_ = new GraphOfProcess();
    //create opencv main thread:
    new GuiReceiver();

    //first load config file:
    config_ = new GlobalConfig();
    config_->loadConfig();

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

    setStyleSheet(config_->styleSheet_.c_str());

    mainLayout_->synchronize(model_);
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
    static bool switchRun = true;
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
          switchRun = false;
          model_->run();
          break;
        case Qt::Key_End:
          switchRun = true;
          model_->stop();
          break;
        case Qt::Key_Space:
          switchRun = !switchRun;
          if (switchRun)
            model_->stop();
          else
            model_->run();
          break;
        case Qt::Key_Delete:
          if (VertexRepresentation::selectedBlock_ != NULL)
            model_->deleteProcess(VertexRepresentation::selectedBlock_->getModel());
          VertexRepresentation::selectedBlock_ = NULL;
          mainLayout_->synchronize(model_);
          break;
        default:
          return QMainWindow::event(event);
        }
        return true;
      }
      return QMainWindow::event(event);
    }
    else
    {
      return QMainWindow::event(event);
    }

    return false;
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