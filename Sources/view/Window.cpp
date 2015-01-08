
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
#include <QMimeData>
#include <QDrag>
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
#include "SubBlock.h"
#include "VertexRepresentation.h"

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

  MainWidget* Window::getMainWidget() const
  {
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
      GraphLayout* graphRep = dynamic_cast<GraphLayout*>(mainWidget->layout());
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

    menuFile = menuBar()->addMenu(_QT("MENU_FILE"));
    menuEdit = menuBar()->addMenu(_QT("MENU_EDIT"));

    QAction* openAct = new QAction(_QT("MENU_FILE_OPEN"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(_QT("MENU_FILE_OPEN_TIP"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
    menuFile->addAction(openAct);
    QAction* createAct = new QAction(_QT("MENU_FILE_CREATE"), this);
    createAct->setShortcuts(QKeySequence::New);
    createAct->setStatusTip(_QT("MENU_FILE_CREATE_TIP"));
    connect(createAct, SIGNAL(triggered()), this, SLOT(newProject()));
    menuFile->addAction(createAct);
    QAction* saveAct = new QAction(_QT("MENU_FILE_SAVE"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(_QT("MENU_FILE_SAVE_TIP"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveProject()));
    menuFile->addAction(saveAct);
    QAction* saveAsAct = new QAction(_QT("MENU_FILE_SAVEAS"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(_QT("MENU_FILE_SAVEAS_TIP"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsProject()));
    menuFile->addAction(saveAsAct);
    QAction* actionQuitter = new QAction(_QT("MENU_FILE_QUIT"), this);
    actionQuitter->setShortcuts(QKeySequence::Close);
    actionQuitter->setStatusTip(_QT("MENU_FILE_QUIT_TIP"));
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(quitProg()));
    menuFile->addAction(actionQuitter);

    QAction* editSubGraph = new QAction(_QT("MENU_EDIT_SUBGRAPH"), this);
    editSubGraph->setShortcut(QKeySequence("Ctrl+G"));
    editSubGraph->setStatusTip(_QT("MENU_EDIT_SUBGRAPH_TIP"));
    connect(editSubGraph, SIGNAL(triggered()), this, SLOT(createSubgraph()));
    menuEdit->addAction(editSubGraph);

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

    charliesoft::GraphOfProcess* graph = getMainWidget()->getModel();
    graph->fromGraph(GlobalConfig::getInstance()->getXML());

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
          dynamic_cast<GraphLayout*>(getMainWidget()->layout())->removeSelectedLinks();
          for (auto rep : VertexRepresentation::getSelection())
          {
            VertexRepresentation* param = dynamic_cast<VertexRepresentation*>(rep);
            if (param != NULL)
              getMainWidget()->getModel()->deleteProcess(param->getModel());
          }
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

  void Window::createSubgraph()
  {
    MainWidget* currentWidget = getMainWidget();
    GraphOfProcess* graph = currentWidget->getModel();
    //create subgraph:
    SubBlock* subBlock = new SubBlock();
    subBlock->getSubGraph()->setParent(graph);
    GraphOfProcess* subGraph = subBlock->getSubGraph();

    //first select blocks selected:
    set<Block*> selectedBlocks;
    vector<GroupParamRepresentation*> representations = GroupParamRepresentation::getSelection();
    GroupParamRepresentation::resetSelection();//we got the selection, now reset the list...

    float x = 0, y = 0;
    for (auto rep : representations)
    {
      VertexRepresentation* param = dynamic_cast<VertexRepresentation*>(rep);
      if (param != NULL)
      {
        Block* b = param->getModel();
        cv::Vec2f pos = b->getPosition();
        x += pos[0];
        y += pos[1];
        selectedBlocks.insert(b);
      }
    }
    x /= representations.size();
    y /= representations.size();

    //now get external links
    vector<BlockLink> externBlocksInput, externBlocksOutput;
    for (auto block : selectedBlocks)
    {
      const std::map<std::string, ParamValue>& params = block->getInputsVals();
      for (auto& param : params)
      {
        if (param.second.isLinked())
        {
          ParamValue* otherParam = param.second.get<ParamValue*>(false);
          if (selectedBlocks.find(otherParam->getBlock()) == selectedBlocks.end())
          {
            externBlocksInput.push_back(BlockLink(otherParam->getBlock(), param.second.getBlock(),
              otherParam->getName(), param.second.getName()));
          }
        }
      }
      const std::map<std::string, ParamValue>& paramsOut = block->getOutputsVals();
      for (const auto& param : paramsOut)
      {
        const std::set<ParamValue*>& listeners = param.second.getListeners();
        for (auto listener : listeners)
        {
          if (listener->isLinked() && selectedBlocks.find(listener->getBlock()) == selectedBlocks.end())
          {
            externBlocksOutput.push_back(BlockLink(param.second.getBlock(), listener->getBlock(),
              param.second.getName(), listener->getName()));
          }
        }
      }
      //remove from current graph the blocks:
      graph->extractProcess(block);
      subGraph->addNewProcess(block);
    }
    subBlock->updatePosition(x, y);
    int idx = 0;
    //create every input needed:
    for (auto& link : externBlocksInput)
    {
      ParamValue* val = link._from->getParam(link._fromParam, false);
      string newName = _STR(link._fromParam) + lexical_cast<string>(idx++);
      ParamValue* newVal = subBlock->addNewInput(
        ParamDefinition(true, val->getType(), newName, link._fromParam));
      graph->removeLink(link);
      *newVal = val;//create link!
      link._to->getParam(link._toParam, true)->setValue(val);
      link._fromParam = newName;
    }
    //create every output needed:
    for (auto& link : externBlocksOutput)
    {
      ParamValue* val = link._to->getParam(link._toParam, true);
      string newName = _STR(link._toParam) + lexical_cast<string>(idx++);
      ParamValue* newVal = subBlock->addNewOutput(
        ParamDefinition(true, val->getType(), newName, link._toParam));
      graph->removeLink(link);
      *val = newVal;//create link!
      link._toParam = newName;
    }
    graph->addNewProcess(subBlock);
    synchroMainGraph();

    MainWidget_SubGraph* handler = new MainWidget_SubGraph(subBlock);
    addTab(handler, "subGraph");
    synchroMainGraph();

    for (auto link : externBlocksInput)
      handler->addNewParamLink(link);

    for (auto link : externBlocksOutput)
      handler->addNewParamLink(link);
  }

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

  void Window::updatePropertyDock(GroupParamRepresentation* vertex)
  {
    VertexRepresentation* param = dynamic_cast<VertexRepresentation*>(vertex);
    if (param != NULL)
    {
      QWidget *prevWidget = property_dock_->widget();
      property_dock_->setWidget(new ParamsConfigurator(param));
      if (prevWidget != NULL)
        delete prevWidget;
    }
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
        QMimeData *mimeData = new QMimeData;
		mimeData->setText(objName.c_str());
		
		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
        Qt::DropAction dropAction = drag->exec();
      }
    }
  }
}