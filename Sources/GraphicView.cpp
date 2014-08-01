
#include "GraphicView.h"
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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>


using namespace std;
using namespace charliesoft;
using namespace boost;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  Fenetre* Fenetre::ptr = NULL;
  recursive_mutex _windowMutex;

  //////////////////////////////////////////////////////////////////////////
  //////////////////Fonctions utilitaires///////////////////////////////////// 
  ////////////////////////////////////////////////////////////////////////// 

  void addEndSlash(string& pathToCheck)
  {
    if (pathToCheck.find_last_of("/") != pathToCheck.length() - 1)
      pathToCheck = pathToCheck + "/";
  }

  void GlobalConfig::saveConfig()
  {
    ptree localElement;
    localElement.put("GlobConfig.LastProject", lastProject_);
    localElement.put("GlobConfig.PrefLang", prefLang_);
    localElement.put("GlobConfig.ShowMaximized", isMaximized);
    localElement.put("GlobConfig.lastPosition.x", lastPosition.x());
    localElement.put("GlobConfig.lastPosition.y", lastPosition.y());
    localElement.put("GlobConfig.lastPosition.width", lastPosition.width());
    localElement.put("GlobConfig.lastPosition.height", lastPosition.height());

    boost::property_tree::xml_writer_settings<char> settings(' ', 2);
    write_xml("config.xml", localElement, std::locale(), settings);
  }

  void GlobalConfig::loadConfig()
  {
    bool xmlOK = false;
    ptree xmlTree;
    //try to read config.xml:
    ifstream ifs("config.xml");
    if (ifs.is_open())
    {
      string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
      stringstream contentStreamed;
      contentStreamed << str;
      try
      {
        read_xml(contentStreamed, xmlTree);
        xmlOK = true;
      }
      catch (boost::property_tree::ptree_bad_path&)
      {
        //nothing to do...
      }
    }
    if (xmlOK)
    {
      lastProject_ = xmlTree.get("GlobConfig.LastProject", "");
      prefLang_ = xmlTree.get("GlobConfig.PrefLang", "en");
      isMaximized = xmlTree.get("GlobConfig.ShowMaximized", true);

      lastPosition.setLeft(xmlTree.get("GlobConfig.lastPosition.x", 0));
      lastPosition.setTop(xmlTree.get("GlobConfig.lastPosition.y", 0));
      lastPosition.setWidth(xmlTree.get("GlobConfig.lastPosition.width", 1024));
      lastPosition.setHeight(xmlTree.get("GlobConfig.lastPosition.height", 768));
    }
    else
    {
      lastProject_ = "";
      prefLang_ = "en";
      isMaximized = true;
      lastPosition.setLeft(0);
      lastPosition.setTop(0);
      lastPosition.setWidth(1024);
      lastPosition.setHeight(768);
    }
  }
  //////////////////////////////////////////////////////////////////////////
  //////////////////Window///////////////////////////////////// 
  ////////////////////////////////////////////////////////////////////////// 

  Fenetre* Fenetre::getInstance()
  {
    lock_guard<recursive_mutex> guard(_windowMutex);//evite les problemes d'acces concurrent
    if (ptr == NULL)
      ptr = new Fenetre();
    return ptr;
  }

  void Fenetre::releaseInstance()
  {
    lock_guard<recursive_mutex> guard(_windowMutex);//evite les problemes d'acces concurrent
    if (ptr != NULL)
      delete ptr;
    ptr = NULL;//already set in destructor, but just in case processor want to do jokes...
  }

  Fenetre::~Fenetre()
  {
    ptr = NULL;
  }

  Fenetre::Fenetre()
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

  void Fenetre::show()
  {
    MainWidget* mainWidget_ = new MainWidget(model_);
    mainWidget_->setStyleSheet("background-color:white;background-image:url(logo.png);background-repeat:no-repeat;background-position:center;");
    setCentralWidget(mainWidget_);

    mainLayout_ = new GraphRepresentation();
    mainWidget_->setLayout(mainLayout_);
    if (config_->isMaximized)
      showMaximized();
    else
    {
      resize(config_->lastPosition.size()); move(config_->lastPosition.topLeft());
      QMainWindow::show();
    }
  }

  void Fenetre::mousePressEvent(QMouseEvent *event)
  {
    if (event->button() == Qt::RightButton)
    {
      BlockLoader* block = new BlockLoader();
      model_->addNewProcess(block);
      mainLayout_->synchronize(model_);//as we updated the model, we ask the layout to redraw itself...
    }
    else if (event->button() == Qt::MidButton)
      std::cout << "middle mouse click " << std::endl;
  }

  void Fenetre::closeEvent(QCloseEvent *event)
  {
    if (!quitProg())
      event->ignore();
    event->accept();
  }

  void Fenetre::openFile()
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

  void Fenetre::newProject()
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

  bool Fenetre::quitProg()
  {
    config_->isMaximized = isMaximized();
    if (!config_->isMaximized)
      config_->lastPosition = QRect(pos(), size());
    config_->saveConfig();
    QApplication::quit();
    return true;
  }
  void Fenetre::printHelp()
  {

  }

  NodeRepresentation::NodeRepresentation(Block* model)
  {
    isDragging = false;

    model_ = model;
    QLabel *name = new QLabel(model->getName().c_str(), this);
    name->setStyleSheet("background-color:rgba(255,255,255,32);border:none;border-radius:5px;");
    QRect sizeNameNode = name->fontMetrics().boundingRect(name->text());
    name->move(8, 5);//move the name at the top of node...

    int topPadding = sizeNameNode.height() + 20;

    int projectedHeight = topPadding;
    int inputHeight, outputHeight, maxInputWidth, maxOutputWidth;
    inputHeight = outputHeight = topPadding;
    maxInputWidth = maxOutputWidth = 0;

    //for each input and output create buttons:
    vector<string> inputParams = model->getListParams();
    vector<ParamRepresentation*> tmpButtonsIn;
    QRect tmpSize;
    for (size_t i = 0; i < inputParams.size() ; i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, inputParams[i].c_str(), this);
      tmpButtonsIn.push_back(tmp);
      tmp->setStyleSheet("background-color:rgba(255,255,255,255);border-radius:0px;");
      tmp->setMinimumWidth(5);
      tmp->move(-2, inputHeight);//move the name at the top of node...
      tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      inputHeight += tmpSize.height() + 10;
      if (maxInputWidth < tmpSize.width())
        maxInputWidth = tmpSize.width();
    }

    vector<string> outputParams = model->getListOutputs();
    vector<ParamRepresentation*> tmpButtonsOut;
    for (size_t i = 0; i < outputParams.size(); i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, outputParams[i].c_str(), this);
      tmpButtonsOut.push_back(tmp);
      tmp->setStyleSheet("background-color:rgba(255,255,255,255);border-radius:0px;");
      tmp->setMinimumWidth(5);
      tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->move(sizeNameNode.width() + 16 - tmpSize.width() - 4, outputHeight);//move the name at the top of node...
      outputHeight += tmpSize.height() + 10;
      if (maxOutputWidth < tmpSize.width())
        maxOutputWidth = tmpSize.width();
    }
    maxInputWidth += 10;
    maxOutputWidth += 10;
    //now recompute with correct width:
    int newWidth = maxOutputWidth + maxInputWidth + 10;
    if (newWidth < sizeNameNode.width() + 16)
      newWidth = sizeNameNode.width() + 16;
    else
    {
      inputHeight = outputHeight = topPadding;
      if (inputParams.size() > outputParams.size())
        outputHeight += (tmpSize.height() + 10) * ((static_cast<double>(inputParams.size()) - outputParams.size()) / 2.);
      else
        inputHeight += (tmpSize.height() + 10) * ((static_cast<double>(outputParams.size()) - inputParams.size()) / 2.);

      for (size_t i = 0; i < tmpButtonsIn.size(); i++)
      {
        QRect tmpSize = tmpButtonsIn[i]->fontMetrics().boundingRect(tmpButtonsIn[i]->text());
        tmpButtonsIn[i]->resize(maxInputWidth, tmpSize.height() + 5);
        tmpButtonsIn[i]->move(-2, inputHeight);//move the name at the top of node...
        inputHeight += tmpSize.height() + 10;
      }
      for (size_t i = 0; i < tmpButtonsOut.size(); i++)
      {
        QRect tmpSize = tmpButtonsOut[i]->fontMetrics().boundingRect(tmpButtonsOut[i]->text());
        tmpButtonsOut[i]->resize(maxOutputWidth, tmpSize.height() + 5);
        tmpButtonsOut[i]->move(newWidth - maxOutputWidth + 2, outputHeight);//move the name at the top of node...
        outputHeight += tmpSize.height() + 10;
      }
    }

    name->move((newWidth - sizeNameNode.width()) / 2, 5);//move the name at the top of node...
    QFrame* myFrame = new QFrame(this);//add a line...
    myFrame->setFrameShape(QFrame::HLine);
    myFrame->setStyleSheet("border: 2px solid #555;border-radius:0px;");
    myFrame->resize(newWidth, 2);
    myFrame->move(0, sizeNameNode.height() + 8 );//move the name at the top of node...


    projectedHeight += max(inputHeight, outputHeight);

    resize(newWidth, projectedHeight-20);
    setStyleSheet("border: 2px solid #555;border-radius: 11px;background: qradialgradient(cx: 0.3, cy: -0.4, fx: 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #888);");

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(3, 3);
    setGraphicsEffect(shadowEffect);
  }

  void NodeRepresentation::mousePressEvent(QMouseEvent *mouseE)
  {
    raise();
    isDragging = true;
    const QPoint p = mouseE->globalPos();
    QPoint myPos = pos();
    deltaClick = myPos - p;
  }

  void NodeRepresentation::mouseReleaseEvent(QMouseEvent *)
  {
    isDragging = false;
  }

  void NodeRepresentation::mouseMoveEvent(QMouseEvent *mouseE)
  {
    if (isDragging)
    {
      QPoint p = mouseE->globalPos() + deltaClick;
      move(p.x(), p.y());
      Fenetre::getInstance()->update();
    }
    else
      mouseE->ignore();
  }

  void NodeRepresentation::mouseDoubleClickEvent(QMouseEvent *)
  {
  }

  void ParamRepresentation::mousePressEvent(QMouseEvent *e)
  {
    if (NodeRepresentation* father = dynamic_cast<NodeRepresentation*>(this->parentWidget()))
    {

    }
  }
  void ParamRepresentation::mouseReleaseEvent(QMouseEvent *)
  {

  };
  void ParamRepresentation::mouseDoubleClickEvent(QMouseEvent *)
  {

  };
  void ParamRepresentation::mouseMoveEvent(QMouseEvent *me)
  {
    me->ignore();
  };

  GraphRepresentation::GraphRepresentation()
  {
  }

  void GraphRepresentation::addItem(QLayoutItem * item)
  {
    items_.push_back(item);
    static int pos = 150;
    item->widget()->move(pos, 150);
    pos += 150;
  }

  QLayoutItem * GraphRepresentation::itemAt(int index) const
  {
    if (index >= (int)items_.size())
      return NULL;
    return items_[index];
  }

  QLayoutItem * GraphRepresentation::takeAt(int index)
  {
    if (index >= (int)items_.size())
      return NULL;
    QLayoutItem *output = items_[index];
    items_.erase(items_.begin() + index);
    return output;
  }

  int GraphRepresentation::indexOf(QWidget *widget) const
  {
    for (size_t i = 0; i < items_.size(); i++)
    {
      if (items_[i]->widget() == widget)
        return i;
    }
    return -1;
  }

  int GraphRepresentation::count() const
  {
    return items_.size();
  }

  QSize GraphRepresentation::sizeHint() const
  {
    return QSize(128, 64);
  }

  void GraphRepresentation::clearLayout(QLayout* layout)
  {
    if (layout == NULL)
      layout = this;
    while (QLayoutItem* item = layout->takeAt(0))
    {
      if (QWidget* widget = item->widget())
        delete widget;
      if (QLayout* childLayout = item->layout())
        clearLayout(childLayout);
      delete item;
    }
  }

  void GraphRepresentation::synchronize(charliesoft::GraphOfProcess *model)
  {
    //for each node, we look for the corresponding representation:
    std::vector<Block*> blocks = model->getNodes();
    for (size_t i = 0; i < blocks.size() ; i++)
    {
      bool found = false;
      for (size_t j = 0; j < items_.size() && !found; j++)
    	{
        //is the widget a NodeRepresentation?
        if (NodeRepresentation* derived = dynamic_cast<NodeRepresentation*>(items_[j]->widget())) {
          if (derived->getModel() == blocks[i])//ok, the model is already displayed
            found = true;
        }
    	}
      if (!found)
      {
        //add this node to view:
        addWidget(new NodeRepresentation(blocks[i]));
      }
    }
  }


  MainWidget::MainWidget(charliesoft::GraphOfProcess *model)
  {
    model_ = model;
  }

  void MainWidget::paintEvent(QPaintEvent *pe)
  {
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(QPointF(qrand() % 300, qrand() % 300), QPointF(qrand() % 300, qrand() % 300));
  }
  void MainWidget::mouseMoveEvent(QMouseEvent *me)
  {
  };
}