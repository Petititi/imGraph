
#include "ParameterDock.h"
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
  ParamsConfigurator::ParamsConfigurator(VertexRepresentation* vertex) :
    QDialog(vertex), _vertex(vertex), 
    in_param_(vertex->getListOfInputChilds()),
    sub_param_(vertex->getListOfSubParams()),
    out_param_(vertex->getListOfOutputChilds())
  {
    setWindowFlags(Qt::Tool);
    tabWidget_ = new QTabWidget(this);
    
    QPushButton* button = new QPushButton(_QT("CONDITION_EDITOR"));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(configCondition()));
    mainLayout->addWidget(tabWidget_);
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

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs_content_[0]->addWidget(empty); 
    empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs_content_[1]->addWidget(empty);

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
      tabs_content_[0]->addWidget(group);
    }
    else
    {
      isSubParam = true;
      vbox = dynamic_cast<QVBoxLayout*>(subWidget_[parent]->layout());
    }

    ParamValue* param = p->getParamValue();
    if (isSubParam)
    {
      subparamGroup_[subWidget_[parent]].push_back(p);
    }
    else
    {
      inputGroup_[group] = p;

      if (param->getType() != Boolean)
        vbox->addWidget(new QLabel(_QT(p->getParamHelper())));
      if (param->isDefaultValue() && !p->isVisible())
        group->setChecked(false);
    }
    QWidget* subContent = new QWidget();
    vbox->addWidget(subContent);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignLeft);
    subContent->setLayout(layout);
    QCheckBox* checkGraph = new QCheckBox();
    layout->addWidget(checkGraph);
    _inputModificator12[checkGraph] = p;
    _inputModificator21[p] = checkGraph;

    if (isSubParam && param->getType() != Boolean)
      layout->addWidget(new QLabel(p->getParamHelper().c_str()));

    if (!p->isVisible())
      checkGraph->setCheckState(Qt::Checked);
    else
      checkGraph->setCheckState(Qt::Unchecked);

    switch (param->getType())
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
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<int>(false)).c_str());
      if (p->isVisible())
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
      combo->setCurrentIndex(-1);//that way, next setCurrentIndex will throw signal, even if it's index 0!
      connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(subParamChange(int)));
      layout->addWidget(combo);
      _inputValue12[p] = combo;
      _inputValue21[combo] = p;

      combo->setCurrentIndex(param->get<int>(false));
      break;
    }
    case Float:
    {
      QLineEdit* lineEdit = new QLineEdit(lexical_cast<string>(param->get<double>(false)).c_str());
      if (p->isVisible())
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
      QPushButton* colorEditor = new QPushButton(_QT("COLOR_EDITOR"));
      layout->addWidget(colorEditor);
      _inputValue12[p] = colorEditor;
      _inputValue21[colorEditor] = p;
      if (!p->getParamValue()->isDefaultValue())
      {
        cv::Scalar tmpColor = p->getParamValue()->get<cv::Scalar>(false);
        _paramColor[p] = tmpColor;
      }

      connect(colorEditor, SIGNAL(clicked()), this, SLOT(colorEditor()));
      break;
    }
    case Matrix:
    {
      QPushButton* matEditor = new QPushButton(_QT("MATRIX_EDITOR"));
      layout->addWidget(matEditor);
      _inputValue12[p] = matEditor;
      _inputValue21[matEditor] = p;
      if (!p->getParamValue()->isDefaultValue())
      {
        Mat img = p->getParamValue()->get<cv::Mat>(false);
        if (!img.empty())
          _paramMatrix[p] = img;
      }
      if (p->isVisible())
        matEditor->setEnabled(false);
      
      connect(matEditor, SIGNAL(clicked()), this, SLOT(matrixEditor()));
      break;
    }
    case String:
    {
      QLineEdit* lineEdit = new QLineEdit(param->toString().c_str());
      if (p->isVisible())
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
      if (p->isVisible())
      {
        button->setEnabled(false);
        lineEdit->setEnabled(false);
      }

      layout->addWidget(lineEdit);
      button->setFixedWidth(70);
      layout->addWidget(button);
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(textChanged()));
      break;
    }
    default://probably typeError
      break;
    }

    connect(checkGraph, SIGNAL(stateChanged(int)), this, SLOT(switchEnable(int)));

    if (!isSubParam)
      connect(group, SIGNAL(toggled(bool)), this, SLOT(switchParamUse(bool)));
  }

  void ParamsConfigurator::subParamChange(int newVal)
  {
    if (newVal < 0)
      return;
    QComboBox* src = dynamic_cast<QComboBox*>(sender());
    ParamRepresentation* value = dynamic_cast<ParamRepresentation*>(_inputValue21[src]);

    //find GroupBox:
    QGroupBox* groupParams = NULL;
    auto it = inputGroup_.begin();
    while ((it != inputGroup_.end()) && groupParams==NULL)
    {
      if (it->second == value)
        groupParams = it->first;
      it++;
    }
    if (groupParams != NULL)
    {
      //try to add this param:
      if (!updateParamModel(value))
        return;
      //remove the subParams:
      std::vector<ParamRepresentation*>& paramsRep = subparamGroup_[subWidget_[value]];
      for (ParamRepresentation* p : paramsRep)
      {
        _inputValue21.erase(_inputValue12[p]);
        _inputValue12.erase(p);
        _inputModificator12.erase(_inputModificator21[p]);
        _inputModificator21.erase(p);
      }

      QVBoxLayout *vbox = dynamic_cast<QVBoxLayout*>(groupParams->layout());
      vbox->removeWidget(subWidget_[value]);
      delete subWidget_[value];
      subWidget_.erase(value);


      subparamGroup_[value].clear();

      //reconstruct the subParams group:
      subWidget_[value] = new QWidget();
      vbox->addWidget(subWidget_[value]);
      subWidget_[value]->setLayout(new QVBoxLayout());

      //now add also subparameters, if any:
      Block* model = value->getModel();
      string paramValName = src->currentText().toStdString();
      ParamRepresentation* p = inputGroup_[groupParams];

      vector<cv::String> subParams = model->getSubParams(p->getParamName() + "." + paramValName);
      for (cv::String subParam : subParams)
      {
        string fullSubName = p->getParamName() + "." + paramValName + "." + subParam;
        ParamRepresentation *tmp = sub_param_[fullSubName];

        addParamIn(tmp, value);
      }
    }
  }

  void ParamsConfigurator::switchParamUse(bool state)
  {
    QGroupBox* src = dynamic_cast<QGroupBox*>(sender());
    if (src == NULL) return;

    ParamRepresentation* param;
    if (inputGroup_.find(src) != inputGroup_.end())
      param = inputGroup_.at(src);
    if (outputGroup_.find(src) != outputGroup_.end())
      param = outputGroup_.at(src);
    try
    {
      if (param == NULL) return;

      param->useDefault(!state);

      if (!state)
      {
        if (!updateParamModel(param))
          src->setChecked(!state);
        else
          Window::synchroMainGraph();
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
      updateParamModel(p);

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
    updateParamModel(_inputValue21[send]);
  }

  void ParamsConfigurator::openFile()
  {
    if (openFiles_.find(sender()) == openFiles_.end())
      return;//nothing to do...
    QString fileName = QFileDialog::getOpenFileName(this, _QT("BLOCK__INPUT_IN_FILE_HELP"),
      openFiles_[sender()]->text(), _QT("BLOCK__INPUT_IN_FILE_FILTER") + " (*.bmp *.pbm *.pgm *.ppm *.sr *.ras *.jpeg *.jpg *.jpe *.jp2 *.tiff *.tif *.png *.avi *.mov *.mxf *.wmv *.asf)");
    if (!fileName.isEmpty())
    {
      QLineEdit* tmp = openFiles_[sender()];
      if (tmp != NULL)
      {
        QString prevTxt = tmp->text();
        tmp->setText(fileName);
        if (!updateParamModel(_inputValue21[tmp]))
          tmp->setText(prevTxt);//set previous text...
      }
    }
  }

  bool ParamsConfigurator::updateParamModel(ParamRepresentation* paramRep)
  {
    if (paramRep == NULL)
      return false;
    ParamValue* param = paramRep->getParamValue();
    if (paramRep->isDefaultVal())
    {
      try
      {
        param->valid_and_set(Not_A_Value());
      }
      catch (ErrorValidator& e)
      {//algo doesn't accept this value!
        QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
        return false;//stop here the validation: should correct the error!
      }
      paramRep->setVisibility(false);
      return true;//nothing left to do as we don't use val...
    }
    try
    {
      if (!_inputModificator21.at(paramRep)->isChecked())
      {
        paramRep->setVisibility(true);
        paramRep->show();
        return true;//nothing left to do as we will set value using graph
      }
      else
        paramRep->setVisibility(false);
    }
    catch (std::out_of_range&)
    {
    }

    if (param->getType() == Boolean)
    {
      QLabel* value = dynamic_cast<QLabel*>(_inputValue12.at(paramRep));
      if (value != NULL)
      {
        try
        {
          param->valid_and_set(true);
        }
        catch (ErrorValidator& e)
        {//algo doesn't accept this value!
          QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
          return false;//stop here the validation: should correct the error!
        }
      }
    }

    if (param->getType() == Int || param->getType() == Float ||
      param->getType() == String || param->getType() == FilePath)
    {
      QLineEdit* value = dynamic_cast<QLineEdit*>(_inputValue12.at(paramRep));
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
          return false;//stop here the validation: should correct the error!
        }
      }
    }

    if (param->getType() == ListBox)
    {
      QComboBox* value = dynamic_cast<QComboBox*>(_inputValue12.at(paramRep));
      if (value != NULL)
      {
        ParamValue val = value->currentIndex();
        try
        {
          param->valid_and_set(val);
        }
        catch (ErrorValidator& e)
        {//algo doesn't accept this value!
          QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
          return false;//stop here the validation: should correct the error!
        }
      }
    }
    if (param->getType() == Color)
    {
      ParamValue val = _paramColor[paramRep];
      try
      {
        param->valid_and_set(val);
      }
      catch (ErrorValidator& e)
      {//algo doesn't accept this value!
        QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
        return false;//stop here the validation: should correct the error!
      }
    }
    if (param->getType() == Matrix)
    {
      ParamValue val = _paramMatrix[paramRep];
      try
      {
        param->valid_and_set(val);
      }
      catch (ErrorValidator& e)
      {//algo doesn't accept this value!
        QMessageBox::warning(this, _QT("ERROR_GENERIC_TITLE"), e.errorMsg.c_str());
        return false;//stop here the validation: should correct the error!
      }
    }
    return true;
  }

  void ParamsConfigurator::colorEditor()
  {
    auto param = _inputValue21.find(sender());
    if (param == _inputValue21.end())
      return;//param src not found...
    QColor src;
    cv::Scalar tmpScal = _paramColor[param->second];
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
      _paramColor[param->second] = color;

      if (!updateParamModel(param->second))
        _paramColor[param->second] = tmpScal;//set previous color...
    }
  }

  void ParamsConfigurator::matrixEditor()
  {
    auto param = _inputValue21.find(sender());
    if (param == _inputValue21.end())
      return;//param src not found...
    MatrixViewer* win = createWindow(_STR("MATRIX_EDITOR"), _WINDOW_MATRIX_CREATION_MODE);
    win->setParent(this, Qt::Tool);

    if (!_paramMatrix[param->second].empty())
      win->updateImage(_paramMatrix[param->second]);

    if (win->exec() == QDialog::Accepted)//now try to get matrix:
    {
      Mat tmp = _paramMatrix[param->second];
      _paramMatrix[param->second] = win->getMatrix();
      if (!updateParamModel(param->second))
        _paramMatrix[param->second] = tmp;//set previous matrix...
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
        _value_left->setText(lexical_cast<string>(c.getOpt_value_left().get<double>(false)).c_str());
      if (c.getCategory_right() > 1)
        _value_right->setText(lexical_cast<string>(c.getOpt_value_right().get<double>(false)).c_str());
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

}