
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
#include <QDebug>
#include <QDockWidget>
#include <QSplitter>
#include <QLineEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QDoubleValidator>
#include <QIntValidator>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "Window.h"

#include "ProcessManager.h"
#include "blocks/ParamValidator.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;

namespace charliesoft
{
  VertexRepresentation* VertexRepresentation::selectedBlock_ = NULL;

  void GlobalConfig::saveConfig()
  {
    ptree localElement;
    localElement.put("GlobConfig.LastProject", lastProject_);
    localElement.put("GlobConfig.PrefLang", prefLang_);
    localElement.put("GlobConfig.styleSheet", styleSheet_);
    localElement.put("GlobConfig.ShowMaximized", isMaximized);
    localElement.put("GlobConfig.lastPosition.x", lastPosition.x());
    localElement.put("GlobConfig.lastPosition.y", lastPosition.y());
    localElement.put("GlobConfig.lastPosition.width", lastPosition.width());
    localElement.put("GlobConfig.lastPosition.height", lastPosition.height());

    charliesoft::GraphOfProcess* graph = Window::getInstance()->getModel();
    graph->saveGraph(localElement);

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
      styleSheet_ = xmlTree.get("GlobConfig.styleSheet", "");

      lastPosition.setLeft(xmlTree.get("GlobConfig.lastPosition.x", 0));
      lastPosition.setTop(xmlTree.get("GlobConfig.lastPosition.y", 0));
      lastPosition.setWidth(xmlTree.get("GlobConfig.lastPosition.width", 1024));
      lastPosition.setHeight(xmlTree.get("GlobConfig.lastPosition.height", 768));

      charliesoft::GraphOfProcess* graph = Window::getInstance()->getModel();
      graph->fromGraph(xmlTree);
    }
    else
    {
      styleSheet_ = "QToolTip {font-style:italic; color: #ffffff; background-color: #2a82aa; border: 1px solid white;}"
        "QWidget#MainWidget{ background:white; background-image:url(logo.png); background-repeat:no-repeat; background-position:center; }"
        "QWidget#DraggableWidget{ max - height:50px; padding: 2px; margin:5px; border:1px solid #888; border-radius: 5px;"
        " background: qradialgradient(cx : 0.3, cy : -0.4, fx : 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #bbb); }"
        "QWidget#VertexRepresentation{ border:2px solid #555; border-radius: 11px;"
        " background: qradialgradient(cx : 0.3, cy : -0.4, fx : 0.3, fy : -0.4, radius : 1.35, stop : 0 #fff, stop: 1 #888); }"
        "QWidget#VertexTitle{ background - color:rgba(255, 255, 255, 32); border:none; border-radius:5px; }"
        "QWidget#VertexTitleLine{ border: 2px solid #555; border-radius:0px; }"
        "QWidget#ParamRepresentation{ background-color:rgba(255, 255, 255, 255); border:1px solid #555; padding:1px; }";
      lastProject_ = "";
      prefLang_ = "en";
      isMaximized = true;
      lastPosition.setLeft(0);
      lastPosition.setTop(0);
      lastPosition.setWidth(1024);
      lastPosition.setHeight(768);
    }
  }

  ParamsConfigurator::ParamsConfigurator(VertexRepresentation* vertex,
    std::map<std::string, ParamRepresentation*>& in_param,
    std::map<std::string, ParamRepresentation*>& out_param) :
    QDialog(vertex), vertex_(vertex), in_param_(in_param), out_param_(out_param)
  {
    setModal(true);
    tabWidget_ = new QTabWidget(this);
    
    OKbutton_ = new QPushButton(_QT("BUTTON_OK"));
    Cancelbutton_ = new QPushButton(_QT("BUTTON_CANCEL"));
    connect(OKbutton_, SIGNAL(clicked()), this, SLOT(accept_button()));
    connect(Cancelbutton_, SIGNAL(clicked()), this, SLOT(reject_button()));

    QHBoxLayout* buttonLayout = new QHBoxLayout(this);
    buttonLayout->addWidget(OKbutton_);
    buttonLayout->addWidget(Cancelbutton_);
    QWidget *buttonGroup = new QWidget(this);
    buttonGroup->setLayout(buttonLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget_);
    mainLayout->addWidget(buttonGroup);
    setLayout(mainLayout);

    QScrollArea* scrollarea = new QScrollArea(tabWidget_);
    scrollarea->setWidgetResizable(true);
    tabs_content_.push_back(new QVBoxLayout(scrollarea));//input tab
    tabs_content_.push_back(new QVBoxLayout());//output tab

    QWidget *tmpWidget = new QWidget(scrollarea);
    tmpWidget->setLayout(tabs_content_[0]);
    scrollarea->setWidget(tmpWidget);
    tabWidget_->addTab(scrollarea, _QT("BLOCK_TITLE_INPUT"));

    tmpWidget = new QWidget(this);
    tmpWidget->setLayout(tabs_content_[1]);
    tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_OUTPUT"));

    Block* model = vertex->getModel();
    //fill input parameters:
    auto it = in_param_.begin();
    while (it!=in_param_.end())
    {
      addParamIn(it->second);
      it++;
    }
    it = out_param_.begin();
    while (it != out_param_.end())
    {
      addParamOut(it->second);
      it++;
    }
  };


  void ParamsConfigurator::addParamOut(ParamRepresentation  *p)
  {
    QGroupBox* group = new QGroupBox(_QT(p->getParamName()), tabWidget_->widget(1));
    QVBoxLayout *vbox = new QVBoxLayout();
    group->setLayout(vbox);
    group->setCheckable(true);
    tabs_content_[1]->addWidget(group);
    group->setChecked(p->isVisible());

    ParamValue* param = p->getParamValue();
    outputGroup_[group] = p;
    vbox->addWidget(new QLabel(_QT(p->getParamHelper())));
    QLineEdit* tmp = new QLineEdit();
    tmp->setEnabled(false);
    tmp->setText(param->toString().c_str());
    vbox->addWidget(tmp);
  }

  void ParamsConfigurator::addParamIn(ParamRepresentation  *p)
  {
    QGroupBox* group = new QGroupBox(_QT(p->getParamName()), tabWidget_->widget(0));
    QVBoxLayout *vbox = new QVBoxLayout();
    group->setLayout(vbox);
    group->setCheckable(true);
    tabs_content_[0]->addWidget(group);

    ParamValue* param = p->getParamValue();
    inputGroup_[group] = p;

    if (param->getType() != Boolean)
      vbox->addWidget(new QLabel(_QT(p->getParamHelper())));
    if (param->isDefaultValue())
      group->setChecked(false);

    QWidget* tmp = new QWidget();
    vbox->addWidget(tmp);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignLeft);
    tmp->setLayout(layout);
    QCheckBox* checkGraph = new QCheckBox();
    layout->addWidget(checkGraph);
    connect(checkGraph, SIGNAL(stateChanged(int)), this, SLOT(switchEnable(int)));
    if (!p->isVisible())
      checkGraph->setCheckState(Qt::Checked);
    inputModificator_.insert(Modif_map_type(checkGraph, p));
    switch (param->getType())
    {
    case Boolean:
    {
      QLabel* checkTmp = new QLabel(_QT(p->getParamHelper()));
      checkTmp->setAlignment(Qt::AlignLeft);
      inputValue_.insert(Val_map_type(p, checkTmp));
      layout->addWidget(checkTmp);
      break;
    }
    case Int:
    {
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<int>()).c_str());
      if (p->isVisible())
        lineEdit->setEnabled(false);
      inputValue_.insert(Val_map_type(p, lineEdit));
      lineEdit->setValidator(new QIntValidator());
      layout->addWidget(lineEdit);
      break;
    }
    case Float:
    {
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<double>()).c_str());
      if (p->isVisible())
        lineEdit->setEnabled(false);
      inputValue_.insert(Val_map_type(p, lineEdit));
      lineEdit->setValidator(new QDoubleValidator());
      layout->addWidget(lineEdit);
      break;
    }
    case Vector:
      layout->addWidget(new QPushButton(_QT("VECTOR_EDITOR")));
      break;
    case Mat:
      layout->addWidget(new QPushButton(_QT("MATRIX_EDITOR")));
      break;
    case String:
    {
      QLineEdit* lineEdit = new QLineEdit(param->toString().c_str());
      if (p->isVisible())
        lineEdit->setEnabled(false);
      layout->addWidget(lineEdit);
      inputValue_.insert(Val_map_type(p, lineEdit));
      break;
    }
    case FilePath:
    {
      QPushButton* button = new QPushButton(_QT("BUTTON_BROWSE"));
      connect(button, SIGNAL(clicked()), this, SLOT(openFile()));
      QLineEdit* lineEdit = new QLineEdit(param->toString().c_str());
      inputValue_.insert(Val_map_type(p, lineEdit));
      openFiles_[button] = lineEdit;
      if (p->isVisible())
      {
        button->setEnabled(false);
        lineEdit->setEnabled(false);
      }

      layout->addWidget(lineEdit);
      button->setFixedWidth(70);
      layout->addWidget(button);
      break;
    }
    default://probably typeError
      break;
    }
  }
  void ParamsConfigurator::switchEnable(int newState)
  {
    QCheckBox* src = dynamic_cast<QCheckBox*>(sender());
    if (src == NULL) return;
    if (inputModificator_.left.find(src) == inputModificator_.left.end())
      return;//nothing to do...
    ParamRepresentation* p = inputModificator_.left.at(src);
    if (p == NULL) return;
    ParamValue* param = p->getParamValue();
    QWidget* inputWidget = dynamic_cast<QWidget*>(inputValue_.left.at(p));
    if (inputWidget == NULL)
      return;
    if (param->getType() == FilePath)
    {
      auto it = openFiles_.begin();
      while (it != openFiles_.end())
      {
        if (it->second == inputWidget)
        {
          QWidget* buttonWidget = dynamic_cast<QWidget*>(it->first);
          buttonWidget->setEnabled(newState == Qt::Checked);
          break;
        }
        it++;
      }
    }
    inputWidget->setEnabled(newState == Qt::Checked);
  }

  void ParamsConfigurator::openFile()
  {
    if (openFiles_.find(sender()) == openFiles_.end())
      return;//nothing to do...
    QString fileName = QFileDialog::getOpenFileName(this, _QT("BLOCK__INPUT_IN_FILE_HELP"),
      openFiles_[sender()]->text(), _QT("BLOCK__INPUT_IN_FILE_FILTER") + " (*.bmp *.pbm *.pgm *.ppm *.sr *.ras *.jpeg *.jpg *.jpe *.jp2 *.tiff *.tif *.png *.avi *.mov *.mxf *.wmv)");
    if (!fileName.isEmpty())
      openFiles_[sender()]->setText(fileName);
  }
  void ParamsConfigurator::accept_button()
  {
    auto it = inputGroup_.begin();
    while (it != inputGroup_.end())
    {
      ParamValue* param = it->second->getParamValue();
      if (it->first->isChecked())
      {
        if (inputModificator_.right.at(it->second)->isChecked())
        {
          it->second->setVisibility(false);
          if (param->getType() == Boolean)
          {
            QLabel* value = dynamic_cast<QLabel*>(inputValue_.left.at(it->second));
            if (value != NULL)
            {
              try
              {
                param->valid_and_set(true);
              }
              catch (ErrorValidator& e)
              {//algo doesn't accept this value!
                QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
                return;//stop here the validation: should correct the error!
              }
            }
          }

          if (param->getType() == Int || param->getType() == Float ||
            param->getType() == String || param->getType() == FilePath)
          {
            QLineEdit* value = dynamic_cast<QLineEdit*>(inputValue_.left.at(it->second));
            if (value != NULL)
            {
              ParamValue& val = ParamValue::fromString(param->getType(), value->text().toStdString());
              try
              {
                param->valid_and_set(val);
              }
              catch (ErrorValidator& e)
              {//algo doesn't accept this value!
                QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
                return;//stop here the validation: should correct the error!
              }
            }
          }
        }
        else
        {
          it->second->setVisibility(true);
        }
      }
      else
      {
        it->second->setVisibility(false);
        try
        {
          param->valid_and_set(Not_A_Value());
        }
        catch (ErrorValidator& e)
        {//algo doesn't accept this value!
          QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
          return;//stop here the validation: should correct the error!
        }
      }
      it++;
    }

    it = outputGroup_.begin();
    while (it != outputGroup_.end())
    {
      it->second->setVisibility(it->first->isChecked());
      it++;
    }

    vertex_->reshape();
    close();
  }
  void ParamsConfigurator::reject_button()
  {
    close();
  }

  VertexRepresentation::~VertexRepresentation()
  {
    for (auto it = listOfInputChilds_.begin(); it != listOfInputChilds_.end(); it++)
      delete it->second;
    listOfInputChilds_.clear();
    for (auto it = listOfOutputChilds_.begin(); it != listOfOutputChilds_.end(); it++)
      delete it->second;
    listOfOutputChilds_.clear();
    for (auto it = links_.begin(); it != links_.end(); it++)
      delete it->second;
    for (auto it = back_links_.begin(); it != back_links_.end(); it++)
      it->first->links_.erase(it->second);

    links_.clear();
    back_links_.clear();
  }

  VertexRepresentation::VertexRepresentation(Block* model)
  {
    setObjectName("VertexRepresentation");
    paramActiv_ = NULL;
    isDragging_ = false;

    model_ = model;
    vertexTitle_ = new QLabel(_QT(model->getName()), this);
    vertexTitle_->setObjectName("VertexTitle");


    //for each input and output create buttons:
    vector<ParamDefinition> inputParams = _PROCESS_MANAGER->getAlgo_InParams(model->getName());
    vector<ParamRepresentation*> tmpButtonsIn;
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (size_t i = 0; i < inputParams.size() ; i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, inputParams[i], true, this);
      connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
      connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
      listOfInputChilds_[inputParams[i].name_] = tmp;
    }

    vector<ParamDefinition> outputParams = _PROCESS_MANAGER->getAlgo_OutParams(model->getName());
    vector<ParamRepresentation*> tmpButtonsOut;
    for (size_t i = 0; i < outputParams.size(); i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, outputParams[i], false, this);
      connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
      connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
      listOfOutputChilds_[outputParams[i].name_] = tmp;
    }

    lineTitle = new QFrame(this);//add a line...
    lineTitle->setFrameShape(QFrame::HLine);
    lineTitle->setObjectName("VertexTitleLine");

    reshape();

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(3, 3);
    setGraphicsEffect(shadowEffect);

    move(model->getPosition().x, model->getPosition().y);
  }

  void VertexRepresentation::reshape()
  {
    QRect sizeNameVertex = vertexTitle_->fontMetrics().boundingRect(vertexTitle_->text());
    
    int topPadding = sizeNameVertex.height() + 20;

    int projectedHeight = topPadding;
    int inputHeight, outputHeight, maxInputWidth, maxOutputWidth;
    inputHeight = outputHeight = topPadding;
    maxInputWidth = maxOutputWidth = 0;


    //for each input and output create buttons:
    auto it = listOfInputChilds_.begin();
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (; it != listOfInputChilds_.end(); it++)
    {
      ParamRepresentation  *tmp = it->second;
      tmp->setMinimumWidth(5);
      tmp->move(-2, inputHeight);//move the name at the top of vertex...
      tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->setVisible(tmp->shouldShow());
      if (tmp->shouldShow())
      {
        showIn++;
        inputHeight += tmpSize.height() + 10;
        if (maxInputWidth < tmpSize.width())
          maxInputWidth = tmpSize.width();
      }
    }

    it = listOfOutputChilds_.begin();
    for (; it != listOfOutputChilds_.end(); it++)
    {
      ParamRepresentation  *tmp = it->second;

      tmp->setMinimumWidth(5);
      tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->move(sizeNameVertex.width() + 16 - tmpSize.width() - 8, outputHeight);//move the name at the top of vertex...
      tmp->setVisible(tmp->shouldShow());
      if (tmp->shouldShow())
      {
        showOut++;
        outputHeight += tmpSize.height() + 10;
        if (maxOutputWidth < tmpSize.width())
          maxOutputWidth = tmpSize.width();
      }
    }
    maxInputWidth += 10;
    maxOutputWidth += 10;
    //now recompute with correct width:
    int newWidth = maxOutputWidth + maxInputWidth + 10;
    if (newWidth < sizeNameVertex.width() + 16)
      newWidth = sizeNameVertex.width() + 16;

    inputHeight = outputHeight = topPadding;
    if (showIn > showOut)
      outputHeight += (tmpSize.height() + 10) * ((static_cast<double>(showIn)-showOut) / 2.);
    else
      inputHeight += (tmpSize.height() + 10) * ((static_cast<double>(showOut)-showIn) / 2.);

    it = listOfInputChilds_.begin();
    for (; it != listOfInputChilds_.end(); it++)
    {
      QRect tmpSize = it->second->fontMetrics().boundingRect(it->second->text());
      it->second->resize(maxInputWidth, tmpSize.height() + 5);
      it->second->move(-2, inputHeight);//move the name at the top of vertex...
      if (it->second->shouldShow())
        inputHeight += tmpSize.height() + 10;
    }
    it = listOfOutputChilds_.begin();
    for (; it != listOfOutputChilds_.end(); it++)
    {
      QRect tmpSize = it->second->fontMetrics().boundingRect(it->second->text());
      it->second->resize(maxOutputWidth, tmpSize.height() + 5);
      it->second->move(newWidth - maxOutputWidth + 4, outputHeight);//move the name at the top of vertex...
      if (it->second->shouldShow())
        outputHeight += tmpSize.height() + 10;
    }

    vertexTitle_->move((newWidth - sizeNameVertex.width()) / 2, 5);//move the name at the top of vertex...

    lineTitle->resize(newWidth, 2);
    lineTitle->move(0, sizeNameVertex.height() + 8);//move the name at the top of vertex...


    projectedHeight += max(inputHeight, outputHeight);

    resize(newWidth, projectedHeight - 20);
  }

  void VertexRepresentation::setEdge(const BlockLink& link)
  {
    try {
      VertexRepresentation* toVertex = Window::getGraphLayout()->getVertexRepresentation(link.to_);
      ParamRepresentation* fromWidget, *toWidget;
      fromWidget = listOfOutputChilds_[link.fromParam_];
      toWidget = toVertex->listOfInputChilds_[link.toParam_];
      if (fromWidget != NULL && toWidget != NULL)
      {
        fromWidget->setVisibility(true);
        toWidget->setVisibility(true);
        toVertex->reshape();
        reshape();
        QPainterPath* previousPath = links_[link];
        if (previousPath != NULL)
          delete previousPath;
        QPainterPath* path = new QPainterPath();
        links_[link] = path;

        path->moveTo(fromWidget->getWorldAnchor());
        path->lineTo(toWidget->getWorldAnchor());
        path->closeSubpath();

        if (previousPath == NULL)
          toVertex->notifyBackLink(link, this);
      }
    }
    catch (const std::out_of_range&) {
    }
  }

  void VertexRepresentation::paintLinks(QPainter& p)
  {
    if (!links_.empty())
    {
      auto iter = links_.begin();
      while (iter != links_.end())
      {
        p.drawPath(*(iter->second));
        iter++;
      }
    }
  }

  void VertexRepresentation::setParamActiv(ParamRepresentation* param)
  {
    paramActiv_ = param;
  }

  void VertexRepresentation::changeStyleProperty(const char* propertyName, QVariant val)
  {
    setProperty(propertyName, val);
    style()->unpolish(this);
    style()->polish(this);
    update();
  }
  void VertexRepresentation::mousePressEvent(QMouseEvent *mouseE)
  {
    if (selectedBlock_ != NULL)
    {
      selectedBlock_->changeStyleProperty("selected", false);
    }
    selectedBlock_ = NULL;
    if (paramActiv_ == NULL && mouseE->button() == Qt::LeftButton)
    {
      selectedBlock_ = this;
      changeStyleProperty("selected", true);
      isDragging_ = true;
      const QPoint p = mouseE->globalPos();
      QPoint myPos = pos();
      deltaClick_ = myPos - p;
    }
    else
      mouseE->ignore();
    raise();
  }

  void VertexRepresentation::mouseReleaseEvent(QMouseEvent *)
  {
    isDragging_ = false;
  }

  void VertexRepresentation::mouseMoveEvent(QMouseEvent *mouseE)
  {
    if (isDragging_)
    {
      QPoint p = mouseE->globalPos() + deltaClick_;
      move(p.x(), p.y());
      model_->setPosition(p.x(), p.y());
      Window::getInstance()->update();

      //recompute owned link positions:
      auto iter = links_.begin();
      while (iter!=links_.end())
      {
        setEdge(iter->first);
        iter++;
      }
      //and ask other vertex to do the same:
      auto iter_back = back_links_.begin();
      while (iter_back != back_links_.end())
      {
        iter_back->first->setEdge(iter_back->second);
        iter_back++;
      }
    }
    else
      mouseE->ignore();
  }

  void VertexRepresentation::mouseDoubleClickEvent(QMouseEvent *)
  {
    ParamsConfigurator config(this, listOfInputChilds_, listOfOutputChilds_);
    int retour = config.exec();
  }

  ParamRepresentation::ParamRepresentation(Block* model, ParamDefinition param, bool isInput, QWidget *father) :
    QLabel(_QT(param.name_.c_str()), father), model_(model), param_(param), isInput_(isInput){
    setObjectName("ParamRepresentation");
    if (!param.show_) this->hide();
    setToolTip(_QT(param.helper_));
  };

  QPoint ParamRepresentation::getWorldAnchor()
  {
    QPoint p = parentWidget()->mapTo(Window::getInstance()->getMainWidget(), mapTo(parentWidget(), pos()) / 2);
    if (isInput_)
      return QPoint(p.x(), p.y() + height() / 2.);
    else
      return QPoint(p.x() + width(), p.y() + height() / 2.);
  }
  
  void ParamRepresentation::mousePressEvent(QMouseEvent *e)
  {
    if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(parentWidget()))
      vertex->setParamActiv(this);
    //get the position of widget inside main widget and send signal of a new link creation:
    emit creationLink(getWorldAnchor());
    e->ignore();
  }
  void ParamRepresentation::mouseReleaseEvent(QMouseEvent *e)
  {
    if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(parentWidget()))
      vertex->setParamActiv(NULL);

    emit releaseLink(Window::getInstance()->getMainWidget()->mapFromGlobal(e->globalPos()));
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
    //get widget:
    if (VertexRepresentation* derived = dynamic_cast<VertexRepresentation*>(item->widget())) {
      orderedBlocks_.push_back(derived->getModel());
      items_[derived->getModel()] = item;
    }
  }

  QLayoutItem * GraphRepresentation::itemAt(int index) const
  {
    if (index >= (int)orderedBlocks_.size())
      return NULL;
    try {
      QLayoutItem * tmp = items_.at(orderedBlocks_[index]);
      return tmp;
    }
    catch (const std::out_of_range&) {
      return NULL;
    }
  }

  QLayoutItem * GraphRepresentation::takeAt(int index)
  {
    if (index >= (int)items_.size())
      return NULL;
    QLayoutItem *output = items_[orderedBlocks_[index]];
    items_.erase(orderedBlocks_[index]);
    orderedBlocks_.erase(orderedBlocks_.begin() + index);
    //TODO: remove edges!
    return output;
  }

  int GraphRepresentation::indexOf(QWidget *widget) const
  {
    for (size_t i = 0; i < orderedBlocks_.size(); i++)
    {
      try {
        if (items_.at(orderedBlocks_[i])->widget() == widget)
          return i;
      }
      catch (const std::out_of_range&) {
        return -1;
      }
    }
    return -1;
  }

  int GraphRepresentation::indexOf(Block *widget) const
  {
    for (size_t i = 0; i < orderedBlocks_.size(); i++)
    {
      if (orderedBlocks_[i] == widget)
        return i;
    }
    return -1;
  }

  int GraphRepresentation::count() const
  {
    return orderedBlocks_.size();
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

  void GraphRepresentation::drawEdges(QPainter& p)
  {
    auto iter = items_.begin();
    while (iter!=items_.end())
    {
      ((VertexRepresentation*)iter->second->widget())->paintLinks(p);
      iter++;
    }
  }

  VertexRepresentation* GraphRepresentation::getVertexRepresentation(Block* b)
  {
    return dynamic_cast<VertexRepresentation*>(items_[b]->widget());
  }

  void GraphRepresentation::synchronize(charliesoft::GraphOfProcess *model)
  {
    //for each vertex, we look for the corresponding representation:
    std::vector<Block*> blocks = model->getVertices();
    for (auto it = blocks.begin(); it != blocks.end(); it++)
    {
      if (items_.find(*it) == items_.end())//add this vertex to view:
        addWidget(new VertexRepresentation(*it));
    }
    //test if block still exist:
    for (auto it_ = items_.begin(); it_ != items_.end(); it_++)
    {
      bool found = false;
      for (auto it = blocks.begin(); it != blocks.end() && !found; it++)
      {
        if (it_->first == *it)
          found = true;
      }

      if (!found)//remove this block from view:
      {
        int pos = indexOf(it_->first);
        auto representation = takeAt(pos);
        delete representation->widget();
        delete representation;
        it_ = items_.begin();//restart iteration (we can't presume for iterator position)
      }
    }
    
    //now get each connections:
    for (auto it = blocks.begin(); it != blocks.end(); it++)
    {
      vector<BlockLink> edges = (*it)->getInEdges();
      for (auto itEdges = edges.begin(); itEdges != edges.end(); itEdges++)
      {
        BlockLink& link = *itEdges;
        VertexRepresentation* fromVertex, *toVertex;
        fromVertex = dynamic_cast<VertexRepresentation*>(items_[link.from_]->widget());
        toVertex = dynamic_cast<VertexRepresentation*>(items_[link.to_]->widget());
        if (fromVertex != NULL && toVertex != NULL)
          fromVertex->setEdge(link);
      }
    }
    Window::getInstance()->update();
  }

  MainWidget::MainWidget(charliesoft::GraphOfProcess *model)
  {
    setObjectName("MainWidget");
    startParam_ = NULL;
    model_ = model;
    setAcceptDrops(true);
  }


  void MainWidget::dragEnterEvent(QDragEnterEvent *event)
  {
    if (event->mimeData()->hasFormat("text/plain"))
      event->acceptProposedAction();
  }

  void MainWidget::dropEvent(QDropEvent *event)
  {
    Block* block = ProcessManager::getInstance()->createAlgoInstance(
      event->mimeData()->text().toStdString());
    model_->addNewProcess(block);
    block->updatePosition(event->pos().x(), event->pos().y());
    emit askSynchro(model_);//as we updated the model, we ask the layout to redraw itself...
  }
  
  void MainWidget::paintEvent(QPaintEvent *pe)
  {
    QStyleOption o;
    o.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(
      QStyle::PE_Widget, &o, &painter, this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 2));

    //now ask each vertex to draw the links:
    Window::getGraphLayout()->drawEdges(painter);

    if (creatingLink_)
      painter.drawLine(startMouse_, endMouse_);
  }

  void MainWidget::mouseMoveEvent(QMouseEvent *me)
  {
    if (creatingLink_)
    {
      endMouse_ = me->pos();
      Window::getInstance()->update();
    }
  };

  void MainWidget::mousePressEvent(QMouseEvent *)
  {
    if (VertexRepresentation::selectedBlock_ != NULL)
      VertexRepresentation::selectedBlock_->changeStyleProperty("selected", false);
    VertexRepresentation::selectedBlock_ = NULL;
  }

  void MainWidget::endLinkCreation(QPoint end)
  {
    endMouse_ = end;
    creatingLink_ = false;

    Window::getInstance()->update();//redraw window...

    //find an hypotetic param widget under mouse:
    if (ParamRepresentation* param = dynamic_cast<ParamRepresentation*>(childAt(endMouse_)))
    {
      //we have a candidate!
      //we should link input on output(or vice versa)
      if (param->isInput() == startParam_->isInput())
      {
        string typeLink = param->isInput() ? _STR("BLOCK_INPUT") : _STR("BLOCK_OUTPUT");
        QMessageBox messageBox;
        string msg = (my_format(_STR("ERROR_LINK_WRONG_INPUT_OUTPUT")) % startParam_->getParamName() % param->getParamName() % typeLink).str();
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), msg.c_str());
        return;
      }

      if (param->getModel() == startParam_->getModel())
      {
        QMessageBox messageBox;
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), _STR("ERROR_LINK_SAME_BLOCK").c_str());
        return;
      }

      //everything seems correct, create the link!!!
      if (param->isInput())
        startParam_->getModel()->createLink(startParam_->getParamName(), param->getModel(), param->getParamName());
      else
        param->getModel()->createLink(param->getParamName(), startParam_->getModel(), startParam_->getParamName());

      emit askSynchro(model_);
    }

  }
  void MainWidget::initLinkCreation(QPoint start)
  {
    startMouse_ = endMouse_ = start;
    creatingLink_ = true;
    startParam_ = dynamic_cast<ParamRepresentation*>(sender());
  }

}