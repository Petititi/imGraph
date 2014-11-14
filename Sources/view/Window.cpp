
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
#include "ParameterDock.h"

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

  MainWidget* Window::getMainWidget() const {
    QScrollArea* tmp = dynamic_cast<QScrollArea*>(_tabWidget->currentWidget());
    if (tmp == NULL)
      return NULL;
    return dynamic_cast<MainWidget*>(tmp->widget());
  };

  void Window::synchroMainGraph()
  {
    MainWidget* mainWidget = getInstance()->getMainWidget();
    if (mainWidget != NULL)
    {
      GraphRepresentation* graphRep = dynamic_cast<GraphRepresentation*>(mainWidget->layout());
      if (graphRep != NULL)
      {
        graphRep->synchronize();
      }
    }
  }

  void Window::redraw()
  {
    MainWidget* tmp = getMainWidget();
    tmp->update();
    tmp->adjustSize();
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
    _tabWidget = new QTabWidget(); 
    _tabWidget->setTabsClosable(true);

    connect(_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab_(int)));

    //create opencv main thread:
    new GuiReceiver();

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
    QAction* saveAct = new QAction(_QT("MENU_FILE_SAVE"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(_QT("MENU_FILE_SAVE_TIP"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveProject()));
    menuFichier->addAction(saveAct);
    QAction* saveAsAct = new QAction(_QT("MENU_FILE_SAVEAS"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(_QT("MENU_FILE_SAVEAS_TIP"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsProject()));
    menuFichier->addAction(saveAsAct);
    QAction* actionQuitter = new QAction(_QT("MENU_FILE_QUIT"), this);
    actionQuitter->setShortcuts(QKeySequence::Close);
    actionQuitter->setStatusTip(_QT("MENU_FILE_QUIT_TIP"));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(quitProg()));
    menuFichier->addAction(actionQuitter);

    menuAide = menuBar()->addMenu("?");
    menuAide->addAction(_QT("MENU_HELP_INFO"));
    menuAide->addAction(_QT("MENU_HELP_HELP"));

    MainWidget* tmp = new MainWidget(new GraphOfProcess());
    addTab(tmp, "MainTab");

    setCentralWidget(_tabWidget);

    statusBar();

    //create dock for block properties:
    property_dock_ = new QDockWidget(_QT("DOCK_PROPERTY_TITLE"), this);
    property_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    property_dock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    addDockWidget(Qt::RightDockWidgetArea, property_dock_);

    _dock_content = new DraggableContainer();
    _dock_content->setColumnCount(1);
    _dock_content->setHeaderLabel(_QT("MATRIX_EDITOR_BLOCKS"));
    dock_ = new QDockWidget(_QT("DOCK_TITLE"), this);
    dock_->setWidget(_dock_content);
    addDockWidget(Qt::LeftDockWidgetArea, dock_);
    dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    //create the 5 docks:
    for (int i = 0; i < 5; i++)
    {
      QTreeWidgetItem *tmp = new QTreeWidgetItem(_dock_content);
      dock_categories.push_back(tmp);
      switch (i)
      {
      case 0:
        tmp->setText(0, _QT("BLOCK_TITLE_INPUT"));
        break;
      case 1:
        tmp->setText(0, _QT("BLOCK_TITLE_IMG_PROCESS"));
        break;
      case 2:
        tmp->setText(0, _QT("BLOCK_TITLE_SIGNAL"));
        break;
      case 3:
        tmp->setText(0, _QT("BLOCK_TITLE_MATH"));
        break;
      default:
        tmp->setText(0, _QT("BLOCK_TITLE_OUTPUT"));
      }
      _dock_content->addTopLevelItem(tmp);
      fillDock(i);
    }
  }

  void Window::fillDock(int idDock)
  {
    std::vector<std::string> list = ProcessManager::getInstance()->getAlgos(AlgoType(idDock));
    auto it = list.begin();
    while (it != list.end())
    {
      QTreeWidgetItem* tmp = new QTreeWidgetItem();
      tmp->setText(0, _QT(*it));
      keysName_[tmp] = *it;
      dock_categories[idDock]->addChild(tmp);
      it++;
    }
  }

  void Window::addTab(MainWidget* tmp, QString tabName)
  {
    GraphRepresentation* layout = new GraphRepresentation();
    tmp->setLayout(layout);

    QScrollArea* scrollarea = new QScrollArea(this);
    scrollarea->setWidgetResizable(true);


    QVBoxLayout* tmpLayout = new QVBoxLayout(scrollarea);
    scrollarea->setLayout(tmpLayout);
    scrollarea->setWidget(tmp);

    _tabWidget->addTab(scrollarea, tabName);
    _tabWidget->setCurrentWidget(scrollarea);
  }

  void Window::show()
  {
    //first load config file:
    GlobalConfig::getInstance()->loadConfig();

    
    if (GlobalConfig::getInstance()->isMaximized)
      showMaximized();
    else
    {
      resize(GlobalConfig::getInstance()->lastPosition.size());
      move(GlobalConfig::getInstance()->lastPosition.topLeft());
      QMainWindow::show();
    }
    
    setStyleSheet(GlobalConfig::getInstance()->styleSheet_.c_str());

    synchroMainGraph();
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
          getMainWidget()->getModel()->run();
          break;
        case Qt::Key_End:
          startRun = true;
          GraphOfProcess::pauseProcess = false;
          getMainWidget()->getModel()->stop();
          break;
        case Qt::Key_Space:
          if (startRun)
          {
            startRun = false;
            getMainWidget()->getModel()->run();
          }
          else
            getMainWidget()->getModel()->switchPause();
          break;
        case Qt::Key_Delete:
          dynamic_cast<GraphRepresentation*>(getMainWidget()->layout())->removeSelectedLinks();
          for (auto rep : VertexRepresentation::getSelection())
            getMainWidget()->getModel()->deleteProcess(rep->getModel());
          VertexRepresentation::resetSelection();
          synchroMainGraph();
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
      ptree xmlTree;
      //try to read the file:
      ifstream ifs(file.toStdString());
      if (ifs.is_open())
      {
        boost::filesystem::path fileName(file.toStdString());
        string test = fileName.filename().string();
        test = test.substr(0, test.length()-4);

        GraphOfProcess* tmp = new GraphOfProcess();
        addTab(new MainWidget(tmp), test.c_str());

        string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
        stringstream contentStreamed;
        contentStreamed << str;
        try
        {
          read_xml(contentStreamed, xmlTree);
        }
        catch (boost::property_tree::ptree_bad_path&)
        {//nothing to do...
        }
        if (!xmlTree.empty())
          tmp->fromGraph(xmlTree);
      }
      synchroMainGraph();
    }

  }

  void Window::newProject()
  {
    GlobalConfig::getInstance()->lastProject_ = "";

    GraphOfProcess* tmp = new GraphOfProcess();
    addTab(new MainWidget(tmp), "newTab");
    synchroMainGraph();
  }

  void Window::saveProject()
  {
    if (GlobalConfig::getInstance()->lastProject_.empty() || getMainWidget()->getModel() == NULL)
      GlobalConfig::getInstance()->saveConfig();
    else
    {
      ptree localElement;
      getMainWidget()->getModel()->saveGraph(localElement);
      boost::property_tree::xml_writer_settings<char> settings(' ', 2);
      write_xml(GlobalConfig::getInstance()->lastProject_, localElement, std::locale(), settings);
    }
  };

  void Window::saveAsProject()
  {
    string lastDirectory = GlobalConfig::getInstance()->lastProject_;
    if (!lastDirectory.empty() && lastDirectory.find_last_of('/') != string::npos)
      lastDirectory = lastDirectory.substr(0, lastDirectory.find_last_of('/'));

    QString file = QFileDialog::getSaveFileName(this, _QT("PROJ_CREATE_FILE"),
      lastDirectory.c_str(), _QT("CONF_FILE_TYPE"));
    if (!file.isEmpty())
      GlobalConfig::getInstance()->lastProject_ = file.toStdString();

    saveProject();
  };

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

  void Window::updatePropertyDock(VertexRepresentation* vertex)
  {
    QWidget *prevWidget = property_dock_->widget();
    property_dock_->setWidget(new ParamsConfigurator(vertex));
    if (prevWidget != NULL)
      delete prevWidget;
  }

  void DraggableContainer::mousePressEvent(QMouseEvent *mouse)
  {
    QTreeWidget::mousePressEvent(mouse);
    if (mouse->button() == Qt::LeftButton)
    {
      //find widget below mouse:
      string objName = Window::getInstance()->getKey(itemAt(mouse->pos()));
      if (!objName.empty())
      {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(objName.c_str());
        drag->setMimeData(mimeData);

        Qt::DropAction dropAction = drag->exec();
      }
    }
  }
}