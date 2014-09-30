
#include "GraphicView.h"
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
#include <QLineEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QDoubleValidator>
#include <QIntValidator>
#include "opencv2/core.hpp"

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
#include "Window.h"
#include "Graph.h"


#include "MatrixViewer.h"

#include "ProcessManager.h"
#include "blocks/ParamValidator.h"

using namespace std;
using namespace charliesoft;
using namespace boost::filesystem;
using boost::recursive_mutex;
using boost::property_tree::ptree;
using boost::lexical_cast;
using boost::lock_guard;
using cv::Mat;

namespace charliesoft
{
  vector<VertexRepresentation*> VertexRepresentation::selectedBlock_;

  ParamsConfigurator::ParamsConfigurator(VertexRepresentation* vertex,
    std::map<std::string, ParamRepresentation*>& in_param,
    std::map<std::string, ParamRepresentation*>& out_param) :
    QDialog(vertex), _vertex(vertex), in_param_(in_param), out_param_(out_param)
  {
    setWindowFlags(Qt::Tool);
    tabWidget_ = new QTabWidget(this);
    
    _OKbutton = new QPushButton(_QT("BUTTON_OK"));
    _Cancelbutton = new QPushButton(_QT("BUTTON_CANCEL"));
    connect(_OKbutton, SIGNAL(clicked()), this, SLOT(accept_button()));
    connect(_Cancelbutton, SIGNAL(clicked()), this, SLOT(reject_button()));

    QHBoxLayout* buttonLayout = new QHBoxLayout(this);
    buttonLayout->addWidget(_OKbutton);
    buttonLayout->addWidget(_Cancelbutton);
    QWidget *buttonGroup = new QWidget(this);
    buttonGroup->setLayout(buttonLayout);

    QPushButton* button = new QPushButton(_QT("CONDITION_EDITOR"));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(configCondition()));
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
    const vector<ParamDefinition>& paramsIn = ProcessManager::getInstance()->getAlgo_InParams(model->getName());
    auto it = paramsIn.begin();
    while (it != paramsIn.end())
    {
      addParamIn(in_param_[it->_name]);
      it++;
    }
    const vector<ParamDefinition>& paramsOut = ProcessManager::getInstance()->getAlgo_OutParams(model->getName());
    it = paramsOut.begin();
    while (it != paramsOut.end())
    {
      addParamOut(out_param_[it->_name]);
      it++;
    }

    connect(this, SIGNAL(askSynchro()), vertex, SLOT(reshape()));
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
    if (param->getType()!=ParamType::Matrix)
      tmp->setText(param->toString().c_str());
    else
      tmp->setText("Matrix...");
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
    case Matrix:
    {
      QPushButton* matEditor = new QPushButton(_QT("MATRIX_EDITOR"));
      layout->addWidget(matEditor);
      inputValue_.insert(Val_map_type(p, matEditor));
      connect(matEditor, SIGNAL(clicked()), this, SLOT(matrixEditor()));
      break;
    }
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

  void ParamsConfigurator::configCondition()
  {
    ConditionConfigurator config_condition(_vertex);
    int retour = config_condition.exec();
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

          if (param->getType() == Matrix)
          {
            ParamValue val = _paramMatrix[it->second];
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

    emit askSynchro();
    Window::getGraphLayout()->synchronize(Window::getInstance()->getModel());
    close();
  }
  void ParamsConfigurator::reject_button()
  {
    close();
  }

  void ParamsConfigurator::matrixEditor()
  {
    auto param = inputValue_.right.find(sender());
    if (param == inputValue_.right.end())
      return;//param src not found...
    MatrixViewer* win = createWindow("test", _WINDOW_MATRIX_CREATION_MODE);
    win->setParent(this, Qt::Tool);

    if (!param->get_left()->getParamValue()->isDefaultValue())
    {
      Mat img = param->get_left()->getParamValue()->get<cv::Mat>();
      if (!img.empty())
        win->updateImage(img);
    }

    if (win->exec() == QDialog::Accepted)
    {
      //now try to get matrix:
      _paramMatrix[param->get_left()] = win->getMatrix();
    }
    delete win;
  }

  ConditionConfigurator::ConditionConfigurator(VertexRepresentation* vertex) :
    QDialog(vertex), _vertex(vertex)
  {
    setModal(true);

    _OKbutton = new QPushButton(_QT("BUTTON_OK"));
    _Cancelbutton = new QPushButton(_QT("BUTTON_CANCEL"));
    _Deletebutton = new QPushButton(_QT("BUTTON_DELETE"));
    connect(_OKbutton, SIGNAL(clicked()), this, SLOT(accept_button()));
    connect(_Deletebutton, SIGNAL(clicked()), this, SLOT(delete_button()));
    connect(_Cancelbutton, SIGNAL(clicked()), this, SLOT(reject_button()));

    _condition_left = new QComboBox(this);
    _condition_type = new QComboBox(this);
    _condition_right = new QComboBox(this);

    _value_left = new QLineEdit(this);
    _value_left->hide();
    _value_right = new QLineEdit(this);
    _value_right->hide();

    _comboBoxLayout = new QGridLayout(this);
    _comboBoxLayout->addWidget(_condition_left, 0, 0);
    _comboBoxLayout->addWidget(_condition_type, 0, 1);
    _comboBoxLayout->addWidget(_condition_right, 0, 2);
    _comboBoxLayout->addWidget(_value_left, 1, 0);
    _comboBoxLayout->addWidget(_value_right, 1, 2);

    connect(_condition_left, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLeft(int)));
    connect(_condition_right, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRight(int)));

    _condition_left->addItem("Choose left condition");
    _condition_left->addItem("Output of block");
    _condition_left->addItem("Constante value");
    _condition_left->addItem("Cardinal of block rendering");

    _condition_right->addItem("Choose right condition");
    _condition_right->addItem("Output of block");
    _condition_right->addItem("Constante value");
    _condition_right->addItem("Is empty");

    _condition_type->addItem("==");
    _condition_type->addItem("!=");
    _condition_type->addItem("<");
    _condition_type->addItem(">");
    _condition_type->addItem("<=");
    _condition_type->addItem(">=");


    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(_OKbutton);
    buttonLayout->addWidget(_Deletebutton);
    buttonLayout->addWidget(_Cancelbutton);
    QWidget *buttonGroup = new QWidget(this);
    buttonGroup->setLayout(buttonLayout);
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel(_QT("CONDITION_EDITOR_HELP")));
    QWidget *comboGroup = new QWidget(this);
    comboGroup->setLayout(_comboBoxLayout);
    mainLayout->addWidget(comboGroup);
    mainLayout->addWidget(buttonGroup);
    setLayout(mainLayout);

    vector<ConditionOfRendering>& cond = vertex->getModel()->getConditions();
    if (cond.size() > 0)
    {
      ConditionOfRendering& c = cond[0];
      _condition_left->setCurrentIndex(c.getCategory_left());
      _condition_right->setCurrentIndex(c.getCategory_right());
      _condition_type->setCurrentIndex(c.getCondition());
      if (c.getCategory_left() > 1)
        _value_left->setText(lexical_cast<string>(c.getOpt_value_left().get<double>()).c_str());
      if (c.getCategory_right() > 1)
        _value_right->setText(lexical_cast<string>(c.getOpt_value_right().get<double>()).c_str());
    }

    connect(this, SIGNAL(askSynchro()), vertex, SLOT(reshape()));
  };

  void ConditionConfigurator::updateLeft(int newIndex)
  {
    switch (newIndex)
    {
    case 2://Constante value
      _value_left->show();
      break;
    default:
      _value_left->hide();
      break;
    }
  };

  void ConditionConfigurator::updateRight(int newIndex)
  {
    switch (newIndex)
    {
    case 2://Constante value
      _value_right->show();
      break;
    default:
      _value_right->hide();
      break;
    }
  };

  void ConditionConfigurator::delete_button()
  {
    _vertex->getModel()->removeCondition();
    emit askSynchro();
    close();
  };

  void ConditionConfigurator::accept_button()
  {
    ParamValue left, right;
    if (_condition_left->currentIndex() == 2)
    {
      try
      {
        left = lexical_cast<double>(_value_left->text().toStdString());
      }
      catch (boost::bad_lexical_cast&)
      {
      }
    }
    if (_condition_right->currentIndex() == 2)
    {
      try
      {
        right = lexical_cast<double>(_value_right->text().toStdString());
      }
      catch (boost::bad_lexical_cast&)
      {
      }
    }

    _vertex->getModel()->addCondition(ConditionOfRendering(
      _condition_left->currentIndex(), left,
      _condition_right->currentIndex(), right,
      _condition_type->currentIndex(), _vertex->getModel()));

    emit askSynchro();
    close();
  };

  void ConditionConfigurator::reject_button()
  {
    close();
  }

  VertexRepresentation::~VertexRepresentation()
  {
    Window::getGraphLayout()->removeLinks(this);

    for (auto it = listOfInputChilds_.begin(); it != listOfInputChilds_.end(); it++)
      delete it->second;
    listOfInputChilds_.clear();
    for (auto it = listOfOutputChilds_.begin(); it != listOfOutputChilds_.end(); it++)
      delete it->second;
    listOfOutputChilds_.clear();
    _links.clear();
    delete _blockRepresentation;
    delete _conditionsRepresentation;
  }

  VertexRepresentation::VertexRepresentation(Block* model)
  {
    _blockRepresentation = new QWidget(this);
    _conditionsRepresentation = new QWidget(this);

    _blockRepresentation->setObjectName("VertexRepresentation");
    _conditionsRepresentation->setObjectName("CondRepresentation");
    _paramActiv = NULL;
    _isDragging = false;

    //QHBoxLayout* condLayout = new QHBoxLayout(_conditionsRepresentation);

    _model = model;
    _vertexTitle = new QLabel(_QT(model->getName()), _blockRepresentation);
    _vertexTitle->setObjectName("VertexTitle");

    _conditionTitle = new QLabel("Conditions", _conditionsRepresentation);
    _conditionTitle->setObjectName("ConditionTitle");
    _conditionTitle->setAlignment(Qt::AlignCenter);

    _conditionsValues = new QLabel("No conditions...", _conditionsRepresentation);
    _conditionsValues->setAlignment(Qt::AlignCenter);
    _conditionsValues->setWordWrap(true);

    //condLayout->addWidget(_conditionTitle);


    //for each input and output create buttons:
    vector<ParamDefinition> inputParams = _PROCESS_MANAGER->getAlgo_InParams(model->getName());
    vector<ParamRepresentation*> tmpButtonsIn;
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (size_t i = 0; i < inputParams.size() ; i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, inputParams[i], true, _blockRepresentation);
      connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
      connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
      listOfInputChilds_[inputParams[i]._name] = tmp;
    }

    vector<ParamDefinition> outputParams = _PROCESS_MANAGER->getAlgo_OutParams(model->getName());
    vector<ParamRepresentation*> tmpButtonsOut;
    for (size_t i = 0; i < outputParams.size(); i++)
    {
      ParamRepresentation  *tmp = new ParamRepresentation(model, outputParams[i], false, _blockRepresentation);
      connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
      connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
      listOfOutputChilds_[outputParams[i]._name] = tmp;
    }

    _lineTitle = new QFrame(_blockRepresentation);//add a line...
    _lineTitle->setFrameShape(QFrame::HLine);
    _lineTitle->setObjectName("VertexTitleLine");

    reshape();

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(3, 3);
    _blockRepresentation->setGraphicsEffect(shadowEffect);

    move((int)model->getPosition().x, (int)model->getPosition().y);
    _blockRepresentation->move(0, 5);
  }

  void VertexRepresentation::removeLink(BlockLink l){
    _links.erase(l);

    if (l._from == _model)
      (*_model->getParam(l._fromParam, false)) = Not_A_Value();
    if (l._to == _model)
      (*_model->getParam(l._toParam, true)) = Not_A_Value();
  };

  ConditionLinkRepresentation* VertexRepresentation::getCondition(ConditionOfRendering* cor, bool isLeft)
  {
    for (auto it : linksConditions_)
    {
      if (it.first == cor && (it.second->isLeftCond()==isLeft))
        return it.second;
    }
    return NULL;
  }

  void VertexRepresentation::reshape()
  {
    QRect sizeNameVertex = _vertexTitle->fontMetrics().boundingRect(_vertexTitle->text());

    //conditions:
    vector<ConditionOfRendering>& conditions = _model->getConditions();
    int heightOfConditions_tmp = 0;
    int maxWidth = 0;
    string conditionsText = "";
    for (ConditionOfRendering& condition : conditions)
    {
      conditionsText += condition.toString() + "\n";
      QRect sizeTMP_cond = _conditionsValues->fontMetrics().boundingRect(condition.toString().c_str());
      if (maxWidth < sizeTMP_cond.width())maxWidth = sizeTMP_cond.width();
      heightOfConditions_tmp += sizeNameVertex.height() + 5;
    }
    if (sizeNameVertex.width() < maxWidth + 26)
      sizeNameVertex.setWidth(maxWidth + 26);
    
    int topPadding = sizeNameVertex.height() + 20;

    int projectedHeight = topPadding;
    int inputHeight, outputHeight, maxInputWidth, maxOutputWidth;
    inputHeight = outputHeight = topPadding;
    maxInputWidth = maxOutputWidth = 0;

    //for each input and output create buttons:
    vector<ParamDefinition> tmpParamsIn = _PROCESS_MANAGER->getAlgo_InParams(_model->getName());
    auto it = tmpParamsIn.begin();
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (; it != tmpParamsIn.end(); it++)
    {
      ParamRepresentation  *tmp = listOfInputChilds_[it->_name];
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

    vector<ParamDefinition> tmpParamsOut = _PROCESS_MANAGER->getAlgo_OutParams(_model->getName());
    it = tmpParamsOut.begin();
    for (; it != tmpParamsOut.end(); it++)
    {
      ParamRepresentation  *tmp = listOfOutputChilds_[it->_name];

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
      outputHeight += (tmpSize.height() + 10) * (int)((static_cast<double>(showIn)-showOut) / 2.);
    else
      inputHeight += (tmpSize.height() + 10) * (int)((static_cast<double>(showOut)-showIn) / 2.);

    it = tmpParamsIn.begin();
    for (; it != tmpParamsIn.end(); it++)
    {
      ParamRepresentation  *tmp = listOfInputChilds_[it->_name];
      QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->resize(maxInputWidth, tmpSize.height() + 5);
      tmp->move(-2, inputHeight);//move the name at the top of vertex...
      if (tmp->shouldShow())
        inputHeight += tmpSize.height() + 10;
    }
    it = tmpParamsOut.begin();
    for (; it != tmpParamsOut.end(); it++)
    {
      ParamRepresentation  *tmp = listOfOutputChilds_[it->_name];
      QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->resize(maxOutputWidth, tmpSize.height() + 5);
      tmp->move(newWidth - maxOutputWidth + 4, outputHeight);//move the name at the top of vertex...
      if (tmp->shouldShow())
        outputHeight += tmpSize.height() + 10;
    }

    _vertexTitle->move((newWidth - sizeNameVertex.width()) / 2, 5);//move the name at the top of vertex...

    _lineTitle->resize(newWidth, 2);
    _lineTitle->move(0, sizeNameVertex.height() + 8);//move the name at the top of vertex...


    projectedHeight += max(inputHeight, outputHeight);

    heightOfConditions = sizeNameVertex.height();
    _blockRepresentation->resize(newWidth, projectedHeight - 20);
    _conditionTitle->resize(newWidth - 26, heightOfConditions);

    _conditionsValues->move(0, heightOfConditions);

    heightOfConditions += heightOfConditions_tmp;
    _conditionsValues->resize(newWidth - 26, heightOfConditions);

    //conditions reshaping:
    _conditionsValues->setText(conditionsText.c_str());

    _conditionsRepresentation->move(13, 0);

    QRect prevSize = _conditionsRepresentation->geometry();
    _conditionsRepresentation->setGeometry(QRect(prevSize.x(), prevSize.y(), newWidth - 26, heightOfConditions + 20));

    //now add connection bloc for each needed conditions:
    heightOfConditions_tmp = sizeNameVertex.height();
    for (ConditionOfRendering& condition : conditions)
    {
      if (condition.getCategory_left() == 1)//output of block...
      {
        ConditionLinkRepresentation* tmp = getCondition(&condition, true);
        if (tmp == NULL)
        {
          tmp = new ConditionLinkRepresentation(&condition, true, _conditionsRepresentation);
          connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
          connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
          linksConditions_.push_back(std::pair<ConditionOfRendering*, ConditionLinkRepresentation*>(&condition, tmp));
        }
        tmp->move(-2, heightOfConditions_tmp);//move the name at the top of vertex...
        tmp->show();
      }
      if (condition.getCategory_right() == 1)//output of block...
      {
        ConditionLinkRepresentation* tmp = getCondition(&condition, false);
        if (tmp == NULL)
        {
          tmp = new ConditionLinkRepresentation(&condition, false, _conditionsRepresentation);
          connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
          connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
          linksConditions_.push_back(std::pair<ConditionOfRendering*, ConditionLinkRepresentation*>(&condition, tmp));
        }
        QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
        tmp->move(newWidth - 32 - tmpSize.width(), heightOfConditions_tmp);//move the name at the top of vertex...
        tmp->show();
      }
      heightOfConditions_tmp += sizeNameVertex.height() + 5;
    }
    if (conditions.empty())
    {
      while (!linksConditions_.empty())
      {
        delete linksConditions_.back().second;
        linksConditions_.pop_back();
      }
    }
    _blockRepresentation->raise();

    resize(newWidth + 10, projectedHeight - 10);

    for (auto link : _links)
      Window::getGraphLayout()->updateLink(link.first);
  }
  
  void VertexRepresentation::setParamActiv(LinkConnexionRepresentation* param)
  {
    _paramActiv = param;
  }

  ParamRepresentation* VertexRepresentation::getParamRep(std::string paramName, bool input)
  {
    if (input)
    {
      if (listOfInputChilds_.find(paramName) != listOfInputChilds_.end())
        return listOfInputChilds_[paramName];
    }
    else
    {
      if (listOfOutputChilds_.find(paramName) != listOfOutputChilds_.end())
        return listOfOutputChilds_[paramName];
    }
    return NULL;
  }

  void VertexRepresentation::changeStyleProperty(const char* propertyName, QVariant val)
  {
    _blockRepresentation->setProperty(propertyName, val);
    _blockRepresentation->style()->unpolish(_blockRepresentation);
    _blockRepresentation->style()->polish(_blockRepresentation);
    _blockRepresentation->update();
  }

  void VertexRepresentation::setSelected(bool isSelected)
  {
    bool prevProperty = _blockRepresentation->property("selected").toBool();
    if (prevProperty == isSelected)
      return;//nothing to do: already in correct state...
    changeStyleProperty("selected", isSelected);
    if (isSelected)
      selectedBlock_.push_back(this);
    else
    {
      //remove from list:
      for (unsigned int pos = 0; pos < selectedBlock_.size(); pos++)
      {
        if (selectedBlock_[pos] == this)
        {
          selectedBlock_.erase(selectedBlock_.begin() + pos);
          return;
        }
      }
    }
  }

  void VertexRepresentation::resetSelection()
  {
    for (auto selection : selectedBlock_)
      selection->changeStyleProperty("selected", false);
    selectedBlock_.clear();
  }

  void VertexRepresentation::mousePressEvent(QMouseEvent *mouseE)
  {
    QPoint mouseP = mouseE->pos();
    if (_paramActiv == NULL && mouseE->button() == Qt::LeftButton)
    {
      if (mouseP.y()>heightOfConditions + 5)
      {
        bool prevProperty = _blockRepresentation->property("selected").toBool();
        if (!prevProperty)//if not previously selected, we reset the selection list...
          resetSelection();
        setSelected(true);
        _isDragging = true;
        startClick_ = mouseE->globalPos();
      }
      if (mouseP.y()<heightOfConditions + 5)
      {
        resetSelection();
        _isDragging = false;
      }
    }
    else
      mouseE->ignore();
    raise();
  }

  void VertexRepresentation::mouseReleaseEvent(QMouseEvent *)
  {
    _isDragging = false;
  }

  void VertexRepresentation::moveDelta(QPoint delta)
  {
    delta = pos() + delta;
    move(delta.x(), delta.y());
    _model->setPosition(delta.x(), delta.y());
    Window::getInstance()->update();
    for (auto link : _links)
      Window::getGraphLayout()->updateLink(link.first);
  }

  void VertexRepresentation::mouseMoveEvent(QMouseEvent *mouseE)
  {
    if (_isDragging)
    {
      QPoint p = mouseE->globalPos();
      QPoint deltaClick_ = p - startClick_;
      startClick_ = p;
      for (VertexRepresentation* vRep:selectedBlock_)
        vRep->moveDelta(deltaClick_);
    }
    else
      mouseE->ignore();
  }

  void VertexRepresentation::mouseDoubleClickEvent(QMouseEvent *mouseE)
  {
    QPoint mouseP = mouseE->pos();
    if (mouseP.y() > heightOfConditions + 5)
    {
      ParamsConfigurator config(this, listOfInputChilds_, listOfOutputChilds_);
      int retour = config.exec();
      cout << retour << endl;
    }
    else
    {
      ConditionConfigurator config(this);
      int retour = config.exec();
    }
  }

  void VertexRepresentation::enterEvent(QEvent *)
  {
    QRect prevSize = geometry();
    setGeometry(QRect(prevSize.x(), prevSize.y() - heightOfConditions, prevSize.width(), prevSize.height() + heightOfConditions));
    _blockRepresentation->move(0, heightOfConditions + 5);
  }

  void VertexRepresentation::leaveEvent(QEvent *)
  {
    QRect prevSize = geometry();
    setGeometry(QRect(prevSize.x(), prevSize.y() + heightOfConditions, prevSize.width(), prevSize.height() - heightOfConditions));
    _blockRepresentation->move(0, 5);
  }

  void GraphRepresentation::addItem(QLayoutItem * item)
  {
    //get widget:
    if (VertexRepresentation* derived = dynamic_cast<VertexRepresentation*>(item->widget())) {
      orderedBlocks_.push_back(derived->getModel());
      _items[derived->getModel()] = item;
    }
  }

  QLayoutItem * GraphRepresentation::itemAt(int index) const
  {
    if (index >= (int)orderedBlocks_.size())
      return NULL;
    try {
      QLayoutItem * tmp = _items.at(orderedBlocks_[index]);
      return tmp;
    }
    catch (const std::out_of_range&) {
      return NULL;
    }
  }

  QLayoutItem * GraphRepresentation::takeAt(int index)
  {
    if (index >= (int)_items.size())
      return NULL;
    QLayoutItem *output = _items[orderedBlocks_[index]];
    _items.erase(orderedBlocks_[index]);
    orderedBlocks_.erase(orderedBlocks_.begin() + index);
    //TODO: remove edges!
    return output;
  }

  int GraphRepresentation::indexOf(QWidget *widget) const
  {
    for (size_t i = 0; i < orderedBlocks_.size(); i++)
    {
      try {
        if (_items.at(orderedBlocks_[i])->widget() == widget)
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

  void GraphRepresentation::removeLinks(VertexRepresentation* vertex)
  {
    //remove every links connected to vertex (in and out):
    map<BlockLink, LinkPath*> links = vertex->getLinks();
    for (auto link : links)
    {
      if (_links.find(link.first) != _links.end())
      {
        _links.erase(link.first);//delete map association...
        if (_items.find(link.first._from)!=_items.end())
          dynamic_cast<VertexRepresentation*>(_items[link.first._from]->widget())->removeLink(link.first);
        if (_items.find(link.first._to) != _items.end())
          dynamic_cast<VertexRepresentation*>(_items[link.first._to]->widget())->removeLink(link.first);

        delete link.second;//delete LinkPath
      }
    }
  }

  void GraphRepresentation::removeSelectedLinks()
  {
    auto it = _links.begin();
    while (it != _links.end())
    {
      if (it->second->isSelected())
      {
        if (_items.find(it->first._from) != _items.end())
          dynamic_cast<VertexRepresentation*>(_items[it->first._from]->widget())->removeLink(it->first);
        if (_items.find(it->first._to) != _items.end())
          dynamic_cast<VertexRepresentation*>(_items[it->first._to]->widget())->removeLink(it->first);

        delete it->second;//delete LinkPath

        _links.erase(it);//delete map association...
        it = _links.begin();//as "it" is now in an undefined state...
      }
      else
        it++;
    }
  }

  void GraphRepresentation::addLink(const BlockLink& link)
  {
    if (_links.find(link) != _links.end())
      return;//nothing to do...

    VertexRepresentation* fromVertex, *toVertex;
    fromVertex = dynamic_cast<VertexRepresentation*>(_items[link._from]->widget());
    toVertex = dynamic_cast<VertexRepresentation*>(_items[link._to]->widget());
    if (fromVertex != NULL && toVertex != NULL)
    {
      auto paramFrom = fromVertex->getParamRep(link._fromParam, false);
      auto paramTo = toVertex->getParamRep(link._toParam, true);
      paramFrom->setVisibility(true);
      paramTo->setVisibility(true);
      LinkPath* path = new LinkPath();
      path->setFillRule(Qt::WindingFill);
      _links[link] = path;
      fromVertex->addLink(link, path);
      toVertex->addLink(link, path);
      path->moveTo(paramFrom->getWorldAnchor());
      path->lineTo(paramTo->getWorldAnchor());
    }
  }

  void GraphRepresentation::updateLink(const BlockLink& link)
  {
    VertexRepresentation* fromVertex, *toVertex;
    fromVertex = dynamic_cast<VertexRepresentation*>(_items[link._from]->widget());
    toVertex = dynamic_cast<VertexRepresentation*>(_items[link._to]->widget());
    if (fromVertex != NULL && toVertex != NULL)
    {
      auto paramFrom = fromVertex->getParamRep(link._fromParam, false);
      auto paramTo = toVertex->getParamRep(link._toParam, true);
      LinkPath* path = _links[link];
      int test = path->elementCount();
      QPoint tmp = paramFrom->getWorldAnchor();
      path->setElementPositionAt(0, tmp.x(), tmp.y());
      tmp = paramTo->getWorldAnchor();
      path->setElementPositionAt(1, tmp.x(), tmp.y());
    }
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
    for (auto iter : _links)
      iter.second->draw(p);
  }

  VertexRepresentation* GraphRepresentation::getVertexRepresentation(Block* b)
  {
    if (_items.find(b) == _items.end())
      return NULL;
    return dynamic_cast<VertexRepresentation*>(_items[b]->widget());
  }

  void GraphRepresentation::synchronize(charliesoft::GraphOfProcess *model)
  {
    //for each vertex, we look for the corresponding representation:
    std::vector<Block*> blocks = model->getVertices();
    for (auto it = blocks.begin(); it != blocks.end(); it++)
    {
      if (_items.find(*it) == _items.end())//add this vertex to view:
        addWidget(new VertexRepresentation(*it));
    }
    //test if block still exist:
    for (auto it_ = _items.begin(); it_ != _items.end(); it_++)
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
        it_ = _items.begin();//restart iteration (we can't presume for iterator position)
      }
    }

    //test if link still exist:
    for (auto link = _links.begin(); link != _links.end(); link++)
    {
      ParamValue* param = link->first._to->getParam(link->first._toParam, true);
      if (param == NULL || !param->isLinked() ||
        param->get<ParamValue*>()->getBlock() != link->first._from)
      {
        delete link->second;
        auto tmpLink = link;
        _links.erase(tmpLink);
        link = _links.begin();
      }
    }

    //test if blocks are ok:
    for (auto item : _items)
    {
      VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(item.second->widget());
      if (vertex != NULL)
        vertex->changeStyleProperty("inconsistent", !item.first->isReadyToRun());
    }
    
    
    //now get each connections:
    for (auto it = blocks.begin(); it != blocks.end(); it++)
    {
      vector<BlockLink> edges = (*it)->getInEdges();
      for (auto itEdges = edges.begin(); itEdges != edges.end(); itEdges++)
      {
        BlockLink& link = *itEdges;
        addLink(link);
      }
    }
    Window::getInstance()->update();
  }

  MainWidget::MainWidget(charliesoft::GraphOfProcess *model)
  {
    setObjectName("MainWidget");
    isSelecting_ = creatingLink_ = false;
    startParam_ = NULL;
    _model = model;
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
    _model->addNewProcess(block);
    block->updatePosition((float)event->pos().x(), (float)event->pos().y());
    emit askSynchro(_model);//as we updated the model, we ask the layout to redraw itself...
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
    if (isSelecting_)
    {
      painter.fillRect((int)selectBox_.x(), (int)selectBox_.y(), (int)selectBox_.width(), (int)selectBox_.height(),
        QColor(0, 0, 255, 128));
      painter.setPen(QColor(0, 0, 200, 255));
      painter.drawRect(selectBox_);
    }
  }

  void MainWidget::mouseMoveEvent(QMouseEvent *me)
  {
    if (creatingLink_)
    {
      endMouse_ = me->pos();
      Window::getInstance()->update();
    }
    if (isSelecting_)
    {
      selectBox_.setCoords(startMouse_.x(), startMouse_.y(),
        me->x() + 1, me->y() + 1);
      //test intersection between vertex representation and selection rect:
      GraphRepresentation* representation = Window::getGraphLayout();
      std::map<Block*, QLayoutItem*>& items = representation->getItems();

      for (auto item : items)
      {
        if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(item.second->widget()))
        {
          if (vertex->geometry().intersects(selectBox_.toRect()))
            vertex->setSelected(true);
          else
            vertex->setSelected(false);
        }
      }

      std::map<BlockLink, LinkPath*>& links = representation->getLinks();
      for (auto link : links)
      {
        if (link.second->intersect(selectBox_.toRect()))
          link.second->setSelected(true);
        else
          link.second->setSelected(false);
      }
      Window::getInstance()->update();
    }
  };

  void MainWidget::mousePressEvent(QMouseEvent *mouseE)
  {
    if (!creatingLink_ && mouseE->button() == Qt::LeftButton)
    {
      startMouse_ = mouseE->pos();
      //begin rect:
      selectBox_.setCoords(startMouse_.x()-3, startMouse_.y()-3,
        startMouse_.x() + 3, startMouse_.y() + 3);

      VertexRepresentation::resetSelection();
      std::map<BlockLink, LinkPath*>& links = Window::getGraphLayout()->getLinks();
      for (auto link : links)
        link.second->setSelected(link.second->intersect(selectBox_.toRect()));

      isSelecting_ = true;
    }
  }
  
  void MainWidget::mouseReleaseEvent(QMouseEvent *)
  {
    creatingLink_ = isSelecting_ = false;
    Window::getInstance()->update();
  }

  void MainWidget::endLinkCreation(QPoint end)
  {
    endMouse_ = end;
    creatingLink_ = false;

    Window::getInstance()->update();//redraw window...

    //find an hypotetic param widget under mouse:
    ParamRepresentation* param = dynamic_cast<ParamRepresentation*>(childAt(endMouse_));
    ParamRepresentation* startParam = dynamic_cast<ParamRepresentation*>(startParam_);
    if (param != NULL &&startParam != NULL)
    {
      //we have a candidate!
      //we should link input on output(or vice versa)
      if (param->isInput() == startParam->isInput())
      {
        string typeLink = param->isInput() ? _STR("BLOCK_INPUT") : _STR("BLOCK_OUTPUT");
        QMessageBox messageBox;
        string msg = (my_format(_STR("ERROR_LINK_WRONG_INPUT_OUTPUT")) % _STR(startParam->getParamName()) % _STR(param->getParamName()) % typeLink).str();
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), msg.c_str());
        return;
      }

      if (param->getModel() == startParam->getModel())
      {
        QMessageBox messageBox;
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), _STR("ERROR_LINK_SAME_BLOCK").c_str());
        return;
      }

      //everything seems correct, create the link!!!
      try{
        if (param->isInput())
          startParam->getModel()->createLink(startParam->getParamName(), param->getModel(), param->getParamName());
        else
          param->getModel()->createLink(param->getParamName(), startParam->getModel(), startParam->getParamName());
      }
      catch (ErrorValidator& e)
      {
        QMessageBox messageBox;
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), e.errorMsg.c_str());
        return;
      }

      emit askSynchro(_model);
    }

    //if one is ConditionLinkRepresentation, other is ParamRepresentation:
    ConditionLinkRepresentation* condParam;
    if (param != NULL)
      condParam = dynamic_cast<ConditionLinkRepresentation*>(startParam_);
    if (startParam != NULL)
    {
      param = startParam;
      condParam = dynamic_cast<ConditionLinkRepresentation*>(childAt(endMouse_));
    }
    if (param != NULL &&condParam != NULL)
    {
      if (param->isInput())
      {
        QMessageBox messageBox;
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), _STR("CONDITION_BLOCK_ERROR_INPUT").c_str());
        return;
      }
      ConditionOfRendering* model = condParam->getModel();
      model->setValue(condParam->isLeftCond(), param->getParamValue());
      emit askSynchro(_model);
    }

  }

  void MainWidget::initLinkCreation(QPoint start)
  {
    startMouse_ = endMouse_ = start;
    creatingLink_ = true;
    startParam_ = dynamic_cast<LinkConnexionRepresentation*>(sender());
  }

}