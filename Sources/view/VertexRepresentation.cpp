
#include "VertexRepresentation.h"
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
#include "blocks/ParamValidator.h"
#include "SubBlock.h"
#include "GraphicView.h"

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

  VertexRepresentation::~VertexRepresentation()
  {
    GraphLayout* representation = dynamic_cast<GraphLayout*>(layout());
    if (representation!=NULL)
      representation->removeLinks(this);

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

    _hasDynamicParams = dynamic_cast<SubBlock*>(model) != NULL;
    _model = model;
    _vertexTitle = new QLabel(_QT(model->getName()), _blockRepresentation);
    _vertexTitle->setObjectName("VertexTitle");

    _conditionTitle = new QLabel("Conditions", _conditionsRepresentation);
    _conditionTitle->setObjectName("ConditionTitle");
    _conditionTitle->setAlignment(Qt::AlignCenter);

    _conditionsValues = new QLabel("No conditions...", _conditionsRepresentation);
    _conditionsValues->setAlignment(Qt::AlignCenter);
    _conditionsValues->setWordWrap(true);
    createListParamsFromModel();

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

    connect(this, SIGNAL(updateProp(VertexRepresentation*)), Window::getInstance(), SLOT(updatePropertyDock(VertexRepresentation*)));
  }

  ParamRepresentation* VertexRepresentation::addNewInputParam(ParamDefinition def)
  {
    ParamRepresentation  *tmp = new ParamRepresentation(_model, def, true, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    listOfInputChilds_[def._name] = tmp;
    if (def._type == ListBox)
    {
      std::vector<string> paramChoices = tmp->getParamListChoice();
      string paramValName = _STR(tmp->getParamName());
      for (size_t idSubParam = 0; idSubParam < paramChoices.size(); idSubParam++)
      {
        vector<cv::String> subParams = _model->getSubParams(def._name + "." + paramChoices[idSubParam]);
        for (cv::String subParam : subParams)
        {
          string fullSubName = def._name + "." + paramChoices[idSubParam] + "." + subParam;
          ParamValue* param = _model->getParam(fullSubName, true);
          if (param != NULL)
          {
            ParamDefinition tmpDef(false, param->getType(), fullSubName, subParam);
            ParamRepresentation *tmp = new ParamRepresentation(_model, tmpDef, true, _blockRepresentation);
            tmp->setVisibility(false);
            tmp->isSubParam(true);
            connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
            connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
            listOfInputSubParams_[tmpDef._name] = tmp;
          }
        }
      }
    }
    return tmp;
  }

  ParamRepresentation* VertexRepresentation::addNewOutputParam(ParamDefinition def)
  {
    ParamRepresentation  *tmp = new ParamRepresentation(_model, def, false, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    listOfOutputChilds_[def._name] = tmp;
    return tmp;
  }

  void VertexRepresentation::createListParamsFromModel()
  {
    //for each input and output create buttons:
    const vector<ParamDefinition>& inputParams = _model->getInParams();
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (size_t i = 0; i < inputParams.size(); i++)
      addNewInputParam(inputParams[i]);

    const vector<ParamDefinition>& outputParams = _model->getOutParams();
    for (size_t i = 0; i < outputParams.size(); i++)
      addNewOutputParam(outputParams[i]);
  }

  void VertexRepresentation::removeLink(BlockLink l){
    _links.erase(l);

    MainWidget* parent = dynamic_cast<MainWidget*>(parentWidget());
    if (parent == NULL)
      return;
    parent->getModel()->removeLink(l);
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
    vector<ParamDefinition> tmpParamsIn = _model->getInParams();
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

    for (auto subPara : listOfInputSubParams_)
    {
      ParamRepresentation  *tmp = subPara.second;
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

    vector<ParamDefinition> tmpParamsOut = _model->getOutParams();
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
    for (auto subPara : listOfInputSubParams_)
    {
      ParamRepresentation  *tmp = subPara.second;
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
      _isMoving = false;
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

  void VertexRepresentation::mouseReleaseEvent(QMouseEvent *mouseE)
  {
    _isDragging = false;

    if (!_isMoving && mouseE->button() == Qt::LeftButton)
      emit updateProp(this);
  }

  void VertexRepresentation::moveDelta(QPoint delta)
  {
    delta = pos() + delta;
    move(delta.x(), delta.y());
    _model->setPosition(delta.x(), delta.y());
    Window::getInstance()->redraw();
  }

  void VertexRepresentation::mouseMoveEvent(QMouseEvent *mouseE)
  {
    if (_isDragging)
    {
      QPoint p = mouseE->globalPos();
      QPoint deltaClick_ = p - startClick_;
      startClick_ = p;
      _isMoving = true;
      for (VertexRepresentation* vRep:selectedBlock_)
        vRep->moveDelta(deltaClick_);
    }
    else
      mouseE->ignore();
  }

  void VertexRepresentation::mouseDoubleClickEvent(QMouseEvent *mouseE)
  {/*
    QPoint mouseP = mouseE->pos();
    if (mouseP.y() > heightOfConditions + 5)
    {
      ParamsConfigurator config(this);
      int retour = config.exec();
    }
    else
    {
      ConditionConfigurator config(this);
      int retour = config.exec();
    }*/
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

}