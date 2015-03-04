#include "ParameterDock.h"
#include "Internationalizator.h"

#include "GuiReceiver.h"

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
#include <QColorDialog>
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
#include "GraphicView.h"
#include "blocks/ParamValidator.h"
#include "SmallDialogs.h"
#include "SubBlock.h"
#include "VertexRepresentation.h"

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
  void ParamsConfigurator::addParam(ParamDefinition* def, bool input)
  {
    Block* model = _vertex->getModel();
    if (model == NULL)
      return;

    ParamRepresentation* param;
    if (input)
    {
      model->addNewInput(def);
      param = dynamic_cast<ParamRepresentation*>(_vertex->addNewInputParam(def));
      addParamIn(param);
    }
    else
    {
      model->addNewOutput(def);
      param = dynamic_cast<ParamRepresentation*>(_vertex->addNewOutputParam(def));
      addParamOut(param);
    }
    emit askSynchro();
  }

  void ParamsConfigurator::addNewParamIn()
  {
    CreateParamWindow test;
    if (test.exec() == QDialog::Accepted)
      addParam(new ParamDefinition(test.getParamDef()), true);
  };

  void ParamsConfigurator::addNewParamOut()
  {
    CreateParamWindow test;
    if (test.exec() == QDialog::Accepted)
      addParam(new ParamDefinition(test.getParamDef()), false);
  };

  ParamsConfigurator::ParamsConfigurator(VertexRepresentation* vertex) :
    QDialog(vertex), _vertex(vertex),
    _in_param(vertex->getListOfInputChilds()),
    _sub_param(vertex->getListOfSubParams()),
    _out_param(vertex->getListOfOutputChilds())
  {
    Block* model = _vertex->getModel();
    Block::BlockType type = model->getExecType();

    tabWidget_ = new QTabWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QWidget *tmpWidget = new QWidget();
    QHBoxLayout* tmpLay = new QHBoxLayout();
    tmpWidget->setLayout(tmpLay);

    mainLayout->addWidget(tmpWidget);

    QPushButton* button = new QPushButton(_QT("CONDITION_EDITOR"));
    tmpLay->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(configCondition()));

    _switchSynchro = new QPushButton();
    _switchSynchro->setFixedWidth(30);
    switch (type)
    {
    case Block::BlockType::synchrone:
      _switchSynchro->setIcon(QIcon(":/synchr-icon"));
      _switchSynchro->setToolTip("<FONT>" + _QT("BUTTON_SWITCH_SYNC") + "</FONT>");
      break;
    default:
      _switchSynchro->setIcon(QIcon(":/asynchr-icon"));
      _switchSynchro->setToolTip("<FONT>" + _QT("BUTTON_SWITCH_ASYNC") + "</FONT>");
      break;
    }
    tmpLay->addWidget(_switchSynchro);
    connect(_switchSynchro, SIGNAL(clicked()), this, SLOT(changeSynchro()));

    mainLayout->addWidget(tabWidget_);
    setLayout(mainLayout);

    QScrollArea* scrollarea = new QScrollArea(tabWidget_);
    scrollarea->setWidgetResizable(true);
    tabs_content_.push_back(new QVBoxLayout(scrollarea));//input tab
    tabs_content_.push_back(new QVBoxLayout());//output tab
    tabs_content_.push_back(new QVBoxLayout());//infos tab

    tmpWidget = new QWidget(scrollarea);
    tmpWidget->setLayout(tabs_content_[0]);
    scrollarea->setWidget(tmpWidget);
    tabWidget_->addTab(scrollarea, _QT("BLOCK_TITLE_INPUT"));

    tmpWidget = new QWidget(this);
    tmpWidget->setLayout(tabs_content_[1]);
    tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_OUTPUT"));

    tmpWidget = new QWidget(this);
    tmpWidget->setLayout(tabs_content_[2]);
    tabWidget_->addTab(tmpWidget, _QT("BLOCK_TITLE_INFOS"));

    _statLabel = new QLabel("EMPTY");
    tabs_content_[2]->addWidget(_statLabel);

    //fill input parameters:
    for (auto& it : _in_param)
    {
      ParamRepresentation* param = dynamic_cast<ParamRepresentation*>(it);
      if (param != NULL)
        addParamIn(param);
    }

    for (auto& it : _out_param)
    {
      ParamRepresentation* param = dynamic_cast<ParamRepresentation*>(it);
      if (param != NULL)
        addParamOut(param);
    }

    if (vertex->hasDynamicParams())
    {
      QPushButton* addInput = new QPushButton(_QT("BUTTON_ADD_INPUT"));
      tabs_content_[0]->addWidget(addInput);
      QPushButton* addOutput = new QPushButton(_QT("BUTTON_ADD_OUTPUT"));
      tabs_content_[1]->addWidget(addOutput);

      connect(addInput, SIGNAL(clicked()), this, SLOT(addNewParamIn()));
      connect(addOutput, SIGNAL(clicked()), this, SLOT(addNewParamOut()));
    }

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs_content_[0]->addWidget(empty);
    empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs_content_[1]->addWidget(empty);
    empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs_content_[2]->addWidget(empty);

    connect(this, SIGNAL(askSynchro()), vertex, SLOT(reshape()));
    connect(tabWidget_, SIGNAL(currentChanged(int)), this, SLOT(changeTab(int)));
  };

  void ParamsConfigurator::updateInfoTab()
  {
    Block* model = _vertex->getModel();
    const AlgoPerformance& algoPerf = model->getPerf();
    string msg = (my_format(_STR("BLOCK_INFOS")) % algoPerf.getMeanPerf() % algoPerf.getMaxPerf() % algoPerf.getMinPerf() %
      model->getNbRendering() % model->getErrorMsg()).str();
    _statLabel->setText(msg.c_str());
  }

  void ParamsConfigurator::timerEvent(QTimerEvent * ev) {
    if (ev->timerId() == _timer.timerId())
      updateInfoTab();
  }

  void ParamsConfigurator::changeTab(int newTab)
  {
    if (newTab == 2)
    {
      if (!_timer.isActive()) _timer.start(1000, this);
      updateInfoTab();
    }
  }

  void ParamsConfigurator::addParamOut(ParamRepresentation  *p)
  {
    QGroupBox* group = new QGroupBox(_QT(p->getParamName()), tabWidget_->widget(1));
    QVBoxLayout *vbox = new QVBoxLayout();
    group->setLayout(vbox);
    group->setCheckable(true);
    tabs_content_[1]->addWidget(group);
    group->setChecked(p->isVisible());

    ParamValue* param = p->getParamValue();
    _outputGroup[group] = p;
    vbox->addWidget(new QLabel(_QT(p->getParamHelper())));
    if (param->getType(false) != ParamType::Matrix)//we want the true type here!
    {
      QLineEdit* tmp = new QLineEdit();
      tmp->setEnabled(false);
      tmp->setText(param->toString().c_str());
      vbox->addWidget(tmp);
    }
    else
    {
      QPushButton* matEditor = new QPushButton(_QT("BUTTON_MATRIX"));
      vbox->addWidget(matEditor);
      _inputValue12[p] = matEditor;
      _inputValue21[matEditor] = p;
      if (p->getParamValue()->isDefaultValue())
        matEditor->setEnabled(false);
      else
      {
        _paramMatrix[p] = p->getParamValue()->get<cv::Mat>();
        connect(matEditor, SIGNAL(clicked()), this, SLOT(matrixEditor()));
      }
    }

    connect(group, SIGNAL(toggled(bool)), this, SLOT(switchParamUse(bool)));
  }

  void ParamsConfigurator::addParamIn(ParamRepresentation  *p, ParamRepresentation* parent)
  {
    bool isSubParam = false;
    QVBoxLayout *vbox;
    QGroupBox* group = NULL;
    if (parent == NULL)
    {
      group = new QGroupBox(_QT(p->getParamName()), tabWidget_->widget(0));
      vbox = new QVBoxLayout();
      group->setLayout(vbox);
      group->setCheckable(true);
      if (_vertex->hasDynamicParams())
        tabs_content_[0]->insertWidget(0, group);
      else
        tabs_content_[0]->addWidget(group);
    }
    else
    {
      isSubParam = true;
      vbox = dynamic_cast<QVBoxLayout*>(subWidget_[parent]->layout());
    }

    ParamValue* param = p->getParamValue();
    if (param == NULL)
    {
      QMessageBox msgBox;
      msgBox.setText((my_format(_STR("ERROR_PARAM_FOUND")) % p->getParamName()).str().c_str());
      msgBox.exec();
      return;
    }

    ParamType tmpType = param->getType();
    bool subParamAnyType = p->isSubParam() && tmpType == AnyType;

    if (isSubParam)
    {
      _subparamGroup[subWidget_[parent]].push_back(p);
    }
    else
    {
      _inputGroup[group] = p;

      if (param->getType() != Boolean)
        vbox->addWidget(new QLabel(_QT(p->getParamHelper())));
      group->setChecked(p->shouldShow() != notUsed);
    }
    QWidget* subContent = new QWidget();
    vbox->addWidget(subContent);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignLeft);
    subContent->setLayout(layout);

    QCheckBox* checkGraph = new QCheckBox();
    if (!subParamAnyType)
    {
      layout->addWidget(checkGraph);
      _inputModificator12[checkGraph] = p;
      _inputModificator21[p] = checkGraph;

      if (isSubParam && param->getType() != Boolean)
        layout->addWidget(new QLabel(p->getParamHelper().c_str()));

      if (p->shouldShow() == userConstant)
        checkGraph->setCheckState(Qt::Checked);
      else
        checkGraph->setCheckState(Qt::Unchecked);
    }


    if (subParamAnyType)
    {
      //get wanted type:
      string fullName = p->getParamName();
      size_t pos = fullName.find_last_of('.');
      if (pos != string::npos)
      {
        string typeName = fullName.substr(pos + 1);
        if (typeName.compare("BOOLEAN") == 0)
          tmpType = Boolean;
        else if (typeName.compare("INT") == 0)
          tmpType = Int;
        else if (typeName.compare("FLOAT") == 0)
          tmpType = Float;
        else if (typeName.compare("COLOR") == 0)
          tmpType = Color;
        else if (typeName.compare("MATRIX") == 0)
          tmpType = Matrix;
        else if (typeName.compare("STRING") == 0)
          tmpType = String;
        else if (typeName.compare("FILE") == 0)
          tmpType = FilePath;
      }
    }

    switch (tmpType)
    {
    case Boolean:
    {
      QLabel* checkTmp = new QLabel(_QT(p->getParamHelper()));
      checkTmp->setAlignment(Qt::AlignLeft);
      _inputValue12[p] = checkTmp;
      _inputValue21[checkTmp] = p;
      layout->addWidget(checkTmp);
      break;
    }
    case Int:
    {
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<int>()).c_str());
      if (p->shouldShow() == toBeLinked)
        lineEdit->setEnabled(false);
      _inputValue12[p] = lineEdit;
      _inputValue21[lineEdit] = p;
      lineEdit->setValidator(new QIntValidator());
      layout->addWidget(lineEdit);
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
      break;
    }
    case ListBox:
    {
      QComboBox* combo = new QComboBox();
      std::vector<std::string>& paramsList = p->getParamListChoice();
      for (std::string paramVal : paramsList)
        combo->addItem(paramVal.c_str());
      combo->setCurrentIndex(param->get<int>());
      if (p->shouldShow() == toBeLinked)
        combo->setEnabled(false);

      connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(subParamChange(int)));
      layout->addWidget(combo);
      _inputValue12[p] = combo;
      _inputValue21[combo] = p;


      changeSubParam(param->get<int>(), combo, false);
      break;
    }
    case Float:
    {
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<double>()).c_str());
      if (p->shouldShow() == toBeLinked)
        lineEdit->setEnabled(false);
      _inputValue12[p] = lineEdit;
      _inputValue21[lineEdit] = p;
      lineEdit->setValidator(new QDoubleValidator());
      layout->addWidget(lineEdit);
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
      break;
    }
    case Color:
    {
      QPushButton* colorEditor = new QPushButton(_QT("BUTTON_COLOR"));
      layout->addWidget(colorEditor);
      _inputValue12[p] = colorEditor;
      _inputValue21[colorEditor] = p;
      if (!p->getParamValue()->isDefaultValue())
      {
        cv::Scalar tmpColor = p->getParamValue()->get<cv::Scalar>();
        _paramColor[p] = tmpColor;
      }
      if (p->shouldShow() == toBeLinked)
        colorEditor->setEnabled(false);

      connect(colorEditor, SIGNAL(clicked()), this, SLOT(colorEditor()));
      break;
    }
    case Matrix:
    {
      QPushButton* matEditor = new QPushButton(_QT("BUTTON_MATRIX"));
      layout->addWidget(matEditor);
      _inputValue12[p] = matEditor;
      _inputValue21[matEditor] = p;
      if (!p->getParamValue()->isDefaultValue())
      {
        Mat img = p->getParamValue()->get<cv::Mat>();
        if (!img.empty())
          _paramMatrix[p] = img;
      }
      if (p->shouldShow() == toBeLinked)
        matEditor->setEnabled(false);

      connect(matEditor, SIGNAL(clicked()), this, SLOT(matrixEditor()));
      break;
    }
    case String:
    {
      QLineEdit* lineEdit = new QLineEdit(param->toString().c_str());
      if (p->shouldShow() == toBeLinked)
        lineEdit->setEnabled(false);
      layout->addWidget(lineEdit);
      _inputValue12[p] = lineEdit;
      _inputValue21[lineEdit] = p;
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
      break;
    }
    case FilePath:
    {
      QPushButton* button = new QPushButton(_QT("BUTTON_BROWSE"));
      connect(button, SIGNAL(clicked()), this, SLOT(openFile()));
      QLineEdit* lineEdit = new QLineEdit(param->toString().c_str());
      _inputValue12[p] = lineEdit;
      _inputValue21[lineEdit] = p;
      openFiles_[button] = lineEdit;
      if (p->shouldShow() == toBeLinked)
      {
        button->setEnabled(false);
        lineEdit->setEnabled(false);
      }

      layout->addWidget(lineEdit);
      button->setFixedWidth(70);
      layout->addWidget(button);
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
      break;
    };
    case AnyType:
    {
      AnyTypeWidget* anyWidget = new AnyTypeWidget(this, p);
      layout->addWidget(anyWidget);
      _inputValue12[p] = anyWidget;
      _inputValue21[anyWidget] = p;


      if (p->shouldShow() == toBeLinked)
        anyWidget->setEnabled(false);

      break;
    }
    default://nothing to do
      break;
    }

    if (!subParamAnyType)
      connect(checkGraph, SIGNAL(stateChanged(int)), this, SLOT(switchEnable(int)));

    if (!isSubParam)
      connect(group, SIGNAL(toggled(bool)), this, SLOT(switchParamUse(bool)));
  }

  AnyTypeWidget::AnyTypeWidget(ParamsConfigurator* configurator, ParamRepresentation* paramRep)
  {
    ParamValue* param = paramRep->getParamValue();
    _configurator = configurator;
    _paramRep = paramRep;
    _vbox = new QVBoxLayout();
    setLayout(_vbox);

    ///\todo: filetype is now yet working!
    //construct vector of values:
    _widgets.push_back(new QCheckBox(this));//for boolean
    _widgets.push_back(new QLineEdit(this));//for int, float and string...
    connect(_widgets.back(), SIGNAL(editingFinished()), _configurator, SLOT(textChanged()));
    _widgets.push_back(new QPushButton(_QT("BUTTON_COLOR"), this));//for color
    connect(_widgets.back(), SIGNAL(clicked()), _configurator, SLOT(colorEditor()));
    _widgets.push_back(new QPushButton(_QT("BUTTON_MATRIX"), this));//for Matrix
    connect(_widgets.back(), SIGNAL(clicked()), _configurator, SLOT(matrixEditor()));
    //_widgets.push_back(new QPushButton(_QT("BUTTON_BROWSE")));//for FileType

    for (QWidget* widget : _widgets)
      widget->setVisible(false);//for now, no visibility...

    _combo = new QComboBox();
    _oldIndex = -1;
    _combo->addItem(_QT("TYPE_DATAS_BOOL"));
    _combo->addItem(_QT("TYPE_DATAS_INT"));
    _combo->addItem(_QT("TYPE_DATAS_FLOAT"));
    _combo->addItem(_QT("TYPE_DATAS_COLOR"));
    _combo->addItem(_QT("TYPE_DATAS_MATRIX"));
    _combo->addItem(_QT("TYPE_DATAS_STRING"));
    _combo->addItem(_QT("TYPE_DATAS_FILE"));
    _combo->setCurrentIndex(_oldIndex);//that way, next setCurrentIndex will throw signal, even if it's index 0!
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(anyTypeChange(int)));

    _vbox->addWidget(_combo);
    _vbox->addWidget(_widgets[0]);

    ParamType realType = param->getType(false);
    switch (realType)
    {
    case Boolean:
      _combo->setCurrentIndex(0);
      break;
    case Int:
      _combo->setCurrentIndex(1);
      break;
    case Float:
      _combo->setCurrentIndex(2);
      break;
    case Color:
      _combo->setCurrentIndex(3);
      break;
    case Matrix:
      _combo->setCurrentIndex(4);
      break;
    case String:
      _combo->setCurrentIndex(5);
      break;
    case FilePath:
      _combo->setCurrentIndex(6);
      break;
    default:
      break;
    }
  }

  void AnyTypeWidget::anyTypeChange(int newVal)
  {
    switch (_oldIndex)
    {
    case 0://bool
      _vbox->removeWidget(_widgets[0]);
      _widgets[0]->setVisible(false);
      break;
    case 1://int
    case 2://float
    case 5://string
      _vbox->removeWidget(_widgets[1]);
      _widgets[1]->setVisible(false);
      break;
    case 3://color
      _vbox->removeWidget(_widgets[2]);
      _widgets[2]->setVisible(false);
      break;
    case 4://Matrix
      _vbox->removeWidget(_widgets[3]);
      _widgets[3]->setVisible(false);
      break;
    case 6://FileType
      break;
    default:
      break;
    }
    _oldIndex = newVal;
    ParamValue* param = _paramRep->getParamValue();
    ParamType realType = param->getType(false);
    switch (newVal)
    {
    case 0://bool
      _vbox->addWidget(_widgets[0]);
      _widgets[0]->setVisible(true);
      if (realType == Boolean)
        ((QCheckBox*)_widgets[0])->setChecked(param->get<bool>());
      break;
    case 1://int
    case 2://float
    case 5://string
    case 6://FileType
      _vbox->addWidget(_widgets[1]);
      _widgets[1]->setVisible(true);
      if (realType != Matrix)//if it'a an image, it would be too big!
        ((QLineEdit*)_widgets[1])->setText(param->toString().c_str());
      break;
    case 3://color
      _vbox->addWidget(_widgets[2]);
      _widgets[2]->setVisible(true);
      if (realType == Color)
        _configurator->setColor(_paramRep, param->get<cv::Scalar>());
      break;
    case 4://Matrix
      _vbox->addWidget(_widgets[3]);
      _widgets[3]->setVisible(true);
      if (realType == Matrix)
        _configurator->setMatrix(_paramRep, param->get<cv::Mat>());
      break;
    default:
      break;
    }
  }

  ParamValue AnyTypeWidget::getValue()
  {
    switch (_oldIndex)
    {
    case 0://bool
      return ((QCheckBox*)_widgets[0])->isChecked();
      break;
    case 1://int
    case 2://float
    case 5://string
    case 6://FileType
    {
      QByteArray valUTF8 = ((QLineEdit*)_widgets[1])->text().toLocal8Bit();
      switch (_oldIndex)
      {
      case 1://int
        return ParamValue::fromString(Int, std::string(valUTF8.constData(), valUTF8.length()));
      case 2://float
        return ParamValue::fromString(Float, std::string(valUTF8.constData(), valUTF8.length()));
      case 5://string
        return ParamValue::fromString(String, std::string(valUTF8.constData(), valUTF8.length()));
      case 6://FileType
        return ParamValue::fromString(FilePath, std::string(valUTF8.constData(), valUTF8.length()));
      }
    }
      break;
    case 3://color
      return _configurator->getColor(_paramRep);
      break;
    case 4://Matrix
      return _configurator->getMatrix(_paramRep);
      break;
    default:
      break;
    }
    return Not_A_Value();
  }

  void ParamsConfigurator::changeSubParam(int newVal, QComboBox* src, bool updateBlock)
  {
    if (newVal < 0)
      return;
    ParamRepresentation* value = dynamic_cast<ParamRepresentation*>(_inputValue21[src]);

    //find GroupBox:
    QGroupBox* groupParams = NULL;
    auto it = _inputGroup.begin();
    while ((it != _inputGroup.end()) && groupParams == NULL)
    {
      if (it->second == value)
        groupParams = it->first;
      it++;
    }
    if (groupParams != NULL)
    {
      //remove the subParams:
      std::vector<ParamRepresentation*>& paramsRep = _subparamGroup[subWidget_[value]];
      for (ParamRepresentation* p : paramsRep)
      {
        _inputValue21.erase(_inputValue12[p]);
        _inputValue12.erase(p);
        _inputModificator12.erase(_inputModificator21[p]);
        _inputModificator21.erase(p);
      }
      //try to add this param:
      if (groupParams->isChecked() && updateBlock)
      {//but only if the group is used!
        if (!updateParamModel(value))
          return;
      }

      QVBoxLayout *vbox = dynamic_cast<QVBoxLayout*>(groupParams->layout());
      vbox->removeWidget(subWidget_[value]);
      delete subWidget_[value];
      subWidget_.erase(value);

      _subparamGroup[value].clear();

      //reconstruct the subParams group:
      subWidget_[value] = new QWidget();
      vbox->addWidget(subWidget_[value]);
      subWidget_[value]->setLayout(new QVBoxLayout());

      if (value->getParamValue()->getType() == ListBox)
      {//This is a listBox, so we set the value, and add subParams if needed:
        //now add also subparameters, if any:
        Block* model = value->getModel();
        string paramValName = src->currentText().toStdString();

        vector<cv::String> subParams = model->getSubParams(value->getParamName() + "." + paramValName);
        for (cv::String subParam : subParams)
        {
          string fullSubName = value->getParamName() + "." + paramValName + "." + subParam;
          ParamRepresentation *tmp = dynamic_cast<ParamRepresentation*>(_sub_param[fullSubName]);

          addParamIn(tmp, value);
        }
      }
      else
      {/// It's probably a "AnyType" value, so we provide a subCategory input, only if param is active!
        ParamValue* param = value->getParamValue();
        if (param != NULL)
        {
          ParamRepresentation *tmp = new ParamRepresentation(value);
          string fullSubName;
          switch (newVal)
          {
          case 0://TYPE_DATAS_BOOL
            tmp->redefineParam(value->getParamName() + ".BOOLEAN", Boolean);
            break;
          case 1://TYPE_DATAS_INT
            tmp->redefineParam(value->getParamName() + ".INT", Int);
            break;
          case 2://TYPE_DATAS_FLOAT
            tmp->redefineParam(value->getParamName() + ".FLOAT", Float);
            break;
          case 3://TYPE_DATAS_COLOR
            tmp->redefineParam(value->getParamName() + ".COLOR", Color);
            break;
          case 4://TYPE_DATAS_MATRIX
            tmp->redefineParam(value->getParamName() + ".MATRIX", Matrix);
            break;
          case 5://TYPE_DATAS_STRING
            tmp->redefineParam(value->getParamName() + ".STRING", String);
            break;
          case 6://TYPE_DATAS_FILE
            tmp->redefineParam(value->getParamName() + ".FILE", FilePath);
            break;
          default://TYPE_ERROR
            break;
          }
          tmp->hide();

          addParamIn(tmp, value);
        }
      }
    }
  }
  void ParamsConfigurator::subParamChange(int newVal)
  {
    QComboBox* src = dynamic_cast<QComboBox*>(sender());
    changeSubParam(newVal, src);
  }

  void ParamsConfigurator::switchParamUse(bool state)
  {
    QGroupBox* src = dynamic_cast<QGroupBox*>(sender());
    if (src == NULL) return;

    ParamRepresentation* param;
    if (_outputGroup.find(src) != _outputGroup.end())
    {
      param = _outputGroup.at(src);
      param->setVisibility(state ? toBeLinked : notUsed);
      Window::synchroMainGraph();
      return;
    }
    if (_inputGroup.find(src) != _inputGroup.end())
      param = _inputGroup.at(src);
    if (param == NULL) return;
    try
    {
      param->useDefault(!state);

      //if there is subParameters, set them to the new state:
      ParamValue* paramVal = param->getParamValue();
      if (paramVal->getType() == ListBox)
      {
        size_t value = paramVal->get<int>();
        std::vector<std::string>& paramsList = param->getParamListChoice();
        if (paramsList.size() > value)
        {
          vector<cv::String> subParams = paramVal->getBlock()->getSubParams(param->getParamName() + "." + paramsList[value]);
          for (cv::String subParam : subParams)
          {
            string fullSubName = param->getParamName() + "." + paramsList[value] + "." + subParam;
            ParamRepresentation *tmp = dynamic_cast<ParamRepresentation*>(_sub_param[fullSubName]);
            param->useDefault(!state);
          }
        }
      }

      if (state)
      {
        if (!updateParamModel(param))
          src->setChecked(!state);
        else
          Window::synchroMainGraph();
      }
      else
      {
        param->setVisibility(notUsed);
        ParamValue* paramVal = param->getParamValue();
        paramVal->setDefaultValue();
      }
    }
    catch (std::out_of_range&)
    {
      QMessageBox msgBox;
      msgBox.setText((my_format(_STR("ERROR_PARAM_FOUND")) % src->title().toStdString()).str().c_str());
      msgBox.exec();
    }

  };

  void ParamsConfigurator::switchEnable(int newState)
  {
    QCheckBox* src = dynamic_cast<QCheckBox*>(sender());
    if (src == NULL) return;
    try
    {
      ParamRepresentation* p = _inputModificator12.at(src);
      if (p == NULL) return;

      ParamValue* param = p->getParamValue();
      param->isNeeded(newState == Qt::Checked);

      QWidget* inputWidget = dynamic_cast<QWidget*>(_inputValue12.at(p));
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

      updateParamModel(p, false);//don't want the annoying message!

      Window::synchroMainGraph();
    }
    catch (std::out_of_range&)
    {
      QMessageBox msgBox;
      msgBox.setText((my_format(_STR("ERROR_PARAM_FOUND")) % src->text().toStdString()).str().c_str());
      msgBox.exec();
    }
  }

  void ParamsConfigurator::configCondition()
  {
    ConditionConfigurator config_condition(_vertex);
    int retour = config_condition.exec();
  }

  void ParamsConfigurator::textChanged()
  {
    QLineEdit* send = dynamic_cast<QLineEdit*>(sender());
    if (send->hasFocus())//remove focus!
      Window::getInstance()->getMainWidget()->setFocus();
    else
    {
      ParamRepresentation* p = NULL;

      AnyTypeWidget *valAnyType = dynamic_cast<AnyTypeWidget *>(send->parentWidget());
      if (NULL != valAnyType)
        p = dynamic_cast<ParamRepresentation*>(_inputValue21[valAnyType]);
      else
        p = dynamic_cast<ParamRepresentation*>(_inputValue21[send]);

      if (p == NULL)
        return;//nothing to do...
      if (!updateParamModel(p))
      {
        //value is not accepted, restore previous one:
        ParamValue* param = p->getParamValue();
        send->setText(param->toString().c_str());
      }
    }
  }

  void ParamsConfigurator::changeSynchro()
  {
    //More to come(like adjust the frequency, one shot...)
    QPushButton* send = dynamic_cast<QPushButton*>(sender());
    if (send == NULL)
      return;
    Block* model = _vertex->getModel();
    Block::BlockType type = model->getExecType();
    if (type == Block::BlockType::synchrone)
    {
      _switchSynchro->setIcon(QIcon(":/asynchr-icon"));
      _switchSynchro->setToolTip("<FONT>" + _QT("BUTTON_SWITCH_ASYNC") + "</FONT>");
      model->setExecType(Block::BlockType::asynchrone);
    }
    else
    {
      _switchSynchro->setIcon(QIcon(":/synchr-icon"));
      _switchSynchro->setToolTip("<FONT>" + _QT("BUTTON_SWITCH_SYNC") + "</FONT>");
      model->setExecType(Block::BlockType::synchrone);
    }

  }

  void ParamsConfigurator::openFile()
  {
    if (openFiles_.find(sender()) == openFiles_.end())
      return;//nothing to do...
    QLineEdit* tmp = openFiles_[sender()];
    ParamRepresentation* paramRep = _inputValue21[tmp];
    QFileDialog dialog(this, _QT(paramRep->getParamHelper()), tmp->text());
    QString fileName;
    ParamValue* val = paramRep->getParamValue();
    if (val->containValidator<ValFileExist>())
    {
      fileName = QFileDialog::getOpenFileName(this, _QT(paramRep->getParamHelper()), tmp->text(),
        _QT("BLOCK__INPUT_IN_FILE_FILTER") +
        " (*.bmp *.pbm *.pgm *.ppm *.sr *.ras *.jpeg *.jpg *.jpe *.jp2 *.tiff *.tif *.png *.avi *.mov *.mxf *.wmv *.asf);;" +
        _QT("ALL_TYPES") + " (*.*)");
    }
    else if (val->containValidator<FileIsFolder>())
    {
      fileName = QFileDialog::getExistingDirectory(this, _QT(paramRep->getParamHelper()), tmp->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    }
    else
    {
      fileName = QFileDialog::getSaveFileName(this, _QT(paramRep->getParamHelper()), tmp->text());
    }
    if (!fileName.isEmpty())
    {
      //dialog.selectFile(fileName);
      if (tmp != NULL)
      {
        QString prevTxt = tmp->text();
        tmp->setText(fileName);
        if (!updateParamModel(paramRep))
          tmp->setText(prevTxt);//set previous text...
      }
    }
  }

  bool ParamsConfigurator::updateParamModel(ParamRepresentation* paramRep, bool withAlert)
  {
    if (paramRep == NULL)
      return false;
    ParamValue* param = paramRep->getParamValue();
    Block* model = param->getBlock();
    if (paramRep->isDefaultVal())
    {
      try
      {
        param->valid_and_set(Not_A_Value());
      }
      catch (ErrorValidator& e)
      {//algo doesn't accept this value!
        if (QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"),
          QString(e.errorMsg.c_str()) + "<br/>" + _QT("ERROR_CONFIRM_SET_VALUE"),
          QMessageBox::Apply | QMessageBox::RestoreDefaults, QMessageBox::Apply) != QMessageBox::Apply)
          return false;//stop here the validation: should correct the error!
      }
      paramRep->setVisibility(notUsed);
      return true;//nothing left to do as we don't use val...
    }
    try
    {
      if (!_inputModificator21.at(paramRep)->isChecked())
      {
        paramRep->setVisibility(toBeLinked);
        paramRep->show();
        paramRep->getModel()->getGraph()->initChildDatas(paramRep->getModel(), std::set<Block*>());
        return true;//nothing left to do as we will set value using graph
      }
      else
        paramRep->setVisibility(userConstant);
    }
    catch (std::out_of_range&)
    {
    }

    ParamValue val;
    ParamType myType = param->getType();

    if (myType == Boolean)
    {
      QLabel* value = dynamic_cast<QLabel*>(_inputValue12.at(paramRep));
      if (value != NULL)
        val = true;
    }

    if (myType == Int || myType == Float ||
      myType == String || myType == FilePath)
    {
      QLineEdit* value = dynamic_cast<QLineEdit*>(_inputValue12.at(paramRep));
      if (value != NULL)
      {
        QByteArray valUTF8 = value->text().toLocal8Bit();
        val = ParamValue::fromString(param->getType(), std::string(valUTF8.constData(), valUTF8.length()));
      }
    }

    if (myType == ListBox)
    {
      QComboBox* value = dynamic_cast<QComboBox*>(_inputValue12.at(paramRep));
      if (value != NULL)
        val = value->currentIndex();
    }
    if (myType == Color)
      val = _paramColor[paramRep];

    if (myType == Matrix)
      val = _paramMatrix[paramRep];

    if (myType == AnyType)
    {
      AnyTypeWidget* value = dynamic_cast<AnyTypeWidget*>(_inputValue12.at(paramRep));
      if (value != NULL)
        val = value->getValue();
    }

    try
    {
      if (*param != val)
        param->valid_and_set(val);
      else
        return true;
    }
    catch (ErrorValidator& e)
    {//algo doesn't accept this value!
      if (withAlert)
      {
        if (QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"),
          QString(e.errorMsg.c_str()) + "<br/>" + _QT("ERROR_CONFIRM_SET_VALUE"),
          QMessageBox::Apply | QMessageBox::RestoreDefaults, QMessageBox::Apply) != QMessageBox::Apply)
          return false;//stop here the validation: should correct the error!
      }
      *param = val;//force the value!
    }

    paramRep->getModel()->getGraph()->initChildDatas(paramRep->getModel(), std::set<Block*>());
    Window::synchroMainGraph();
    return true;
  }

  void ParamsConfigurator::colorEditor()
  {
    QPushButton* send = dynamic_cast<QPushButton*>(sender());
    auto param = _inputValue21.find(send);
    ParamRepresentation* paramRep = NULL;
    if (param == _inputValue21.end())
    {
      AnyTypeWidget *valAnyType = dynamic_cast<AnyTypeWidget *>(send->parentWidget());
      if (NULL != valAnyType)
        paramRep = dynamic_cast<ParamRepresentation*>(_inputValue21[valAnyType]);
      else
        return;///\todo send error message (param src not found...)
      _paramMatrix[paramRep] = valAnyType->getValue().get<cv::Scalar>();
    }
    else
      paramRep = param->second;

    QColor src;
    cv::Scalar tmpScal = _paramColor[paramRep];
    src.setBlue((int)tmpScal[0]);
    src.setGreen((int)tmpScal[1]);
    src.setRed((int)tmpScal[2]);
    src.setAlpha((int)tmpScal[3]);
    QColor tmpColor = QColorDialog::getColor(src, this);
    if (tmpColor.isValid())
    {
      cv::Scalar color;
      color[0] = tmpColor.blue();
      color[1] = tmpColor.green();
      color[2] = tmpColor.red();
      color[3] = tmpColor.alpha();
      _paramColor[paramRep] = color;

      if (!updateParamModel(paramRep))
        _paramColor[paramRep] = tmpScal;//set previous color...
    }
  }

  void ParamsConfigurator::matrixEditor()
  {
    QPushButton* send = dynamic_cast<QPushButton*>(sender());
    auto param = _inputValue21.find(send);
    ParamRepresentation* paramRep = NULL;
    if (param == _inputValue21.end())
    {
      AnyTypeWidget *valAnyType = dynamic_cast<AnyTypeWidget *>(send->parentWidget());
      if (NULL != valAnyType)
        paramRep = dynamic_cast<ParamRepresentation*>(_inputValue21[valAnyType]);
      else
        return;///\todo send error message (param src not found...)
      _paramMatrix[paramRep] = valAnyType->getValue().get<cv::Mat>();
    }
    else
      paramRep = param->second;

    MatrixViewer* win = createWindow(_STR("BUTTON_MATRIX"), _WINDOW_MATRIX_CREATION_MODE);
    win->setParent(this, Qt::Tool);

    if (!_paramMatrix[paramRep].empty())
      win->updateImage(_paramMatrix[paramRep]);

    if (win->exec() == QDialog::Accepted)//now try to get matrix:
    {
      Mat tmp = _paramMatrix[paramRep];
      _paramMatrix[paramRep] = win->getMatrix();
      if (!updateParamModel(paramRep))
        _paramMatrix[paramRep] = tmp;//set previous matrix...
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

    __valueleft = new QLineEdit(this);
    __valueleft->hide();
    __valueright = new QLineEdit(this);
    __valueright->hide();

    _comboBoxLayout = new QGridLayout(this);
    _comboBoxLayout->addWidget(_condition_left, 0, 0);
    _comboBoxLayout->addWidget(_condition_type, 0, 1);
    _comboBoxLayout->addWidget(_condition_right, 0, 2);
    _comboBoxLayout->addWidget(__valueleft, 1, 0);
    _comboBoxLayout->addWidget(__valueright, 1, 2);

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
        __valueleft->setText(lexical_cast<string>(c.getOpt__valueleft().get<double>()).c_str());
      if (c.getCategory_right() > 1)
        __valueright->setText(lexical_cast<string>(c.getOpt__valueright().get<double>()).c_str());
    }

    connect(this, SIGNAL(askSynchro()), vertex, SLOT(reshape()));
  };

  void ConditionConfigurator::updateLeft(int newIndex)
  {
    switch (newIndex)
    {
    case 2://Constante value
      __valueleft->show();
      break;
    default:
      __valueleft->hide();
      break;
    }
  };

  void ConditionConfigurator::updateRight(int newIndex)
  {
    switch (newIndex)
    {
    case 2://Constante value
      __valueright->show();
      break;
    default:
      __valueright->hide();
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
        left = lexical_cast<double>(__valueleft->text().toStdString());
      }
      catch (boost::bad_lexical_cast&)
      {
      }
    }
    if (_condition_right->currentIndex() == 2)
    {
      try
      {
        right = lexical_cast<double>(__valueright->text().toStdString());
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

}