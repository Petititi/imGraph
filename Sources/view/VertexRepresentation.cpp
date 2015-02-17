
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
#include <boost/lexical_cast.hpp>

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
  vector<GroupParamRepresentation*> GroupParamRepresentation::_selectedBlock;


  MainVertexBlock::MainVertexBlock(GroupParamRepresentation* parent) :QWidget(parent) {
    setObjectName("VertexRepresentation");
    _parent = parent;
    _state = MouseOut;
  };

  void MainVertexBlock::updateMouseState(const QPoint &pos)
  {
    MouseState newState;
    QRect prevSize = geometry();
    if (pos.x() > prevSize.width() - 10 && pos.y() > prevSize.height() - 10)
      newState = MouseDiagResize;
    else if (pos.x() > prevSize.width() - 10)
      newState = MouseHorResize;
    else if (pos.y() > prevSize.height() - 10)
      newState = MouseVerResize;
    else if (pos.y() < 30)
      newState = MouseDragOpen;
    else
      newState = MouseIn;

    if (newState != _state)
    {
      _state = newState;
      switch (_state)
      {
      case MouseHorResize:
        _parent->setCursor(Qt::SizeHorCursor);
        break;
      case MouseVerResize:
        _parent->setCursor(Qt::SizeVerCursor);
        break;
      case MouseDiagResize:
        _parent->setCursor(Qt::SizeFDiagCursor);
        break;
      case MouseDragOpen:
        _parent->setCursor(Qt::OpenHandCursor);
        break;
      case MouseDragClose:
        _parent->setCursor(Qt::ClosedHandCursor);
        break;
      case MouseOut:
      case MouseIn:
      default:
        _parent->setCursor(Qt::ArrowCursor);
        break;
      }
    }
  }


  void MainVertexBlock::mouseMoveEvent(QMouseEvent *event)
  {
    if (event->buttons() == Qt::NoButton)
      updateMouseState(event->pos());

    event->ignore();
  }

  void MainVertexBlock::paintEvent(QPaintEvent *e)
  {
    QStyleOption o;
    o.initFrom(this);
    QPainter painter(this);

    style()->drawPrimitive(
      QStyle::PE_Widget, &o, &painter, this);
  }

  void MainVertexBlock::enterEvent(QEvent *e)
  {
    QEnterEvent *me = dynamic_cast<QEnterEvent *>(e);
    if (NULL != me)//to have access to pos()...
    {
      _state = MouseIn;
      setMouseTracking(true);
      updateMouseState(me->pos());
    }
  }

  void MainVertexBlock::leaveEvent(QEvent *)
  {
    setMouseTracking(false);
    _parent->setCursor(Qt::ArrowCursor);
    _state = MouseOut;
  }

  GroupParamRepresentation::~GroupParamRepresentation()
  {
    for (auto it = _listOfInputChilds.begin(); it != _listOfInputChilds.end(); it++)
      delete *it;
    _listOfInputChilds.clear();
    for (auto it = _listOfOutputChilds.begin(); it != _listOfOutputChilds.end(); it++)
      delete *it;
    _listOfOutputChilds.clear();
    for (auto it = _listOfInputSubParams.begin(); it != _listOfInputSubParams.end(); it++)
      delete it->second;
    _listOfInputSubParams.clear();
    for (auto it = _listOfSubParams.begin(); it != _listOfSubParams.end(); it++)
      delete *it;
    _listOfSubParams.clear();

    _links.clear();
    delete _blockRepresentation;
    delete _conditionsRepresentation;
  }

  GroupParamRepresentation::GroupParamRepresentation(std::string title):
    _sizeIncrement(0,0)
  {
    _preview = NULL;
    _previewLink = NULL;

    _blockRepresentation = new MainVertexBlock(this);
    _conditionsRepresentation = new QWidget(this);

    _blockRepresentation->setObjectName("GroupParamRepresentation");
    _conditionsRepresentation->setObjectName("CondRepresentation");
    _paramActiv = NULL;

    _hasDynamicParams = true;

    _vertexTitle = new QLabel(_QT(title), _blockRepresentation);
    _vertexTitle->setObjectName("VertexTitle");

    _conditionTitle = new QLabel("Conditions", _conditionsRepresentation);
    _conditionTitle->setObjectName("ConditionTitle");
    _conditionTitle->setAlignment(Qt::AlignCenter);

    _conditionsValues = new QLabel("No conditions...", _conditionsRepresentation);
    _conditionsValues->setAlignment(Qt::AlignCenter);
    _conditionsValues->setWordWrap(true);

    _lineTitle = new QFrame(_blockRepresentation);//add a line...
    _lineTitle->setFrameShape(QFrame::HLine);
    _lineTitle->setObjectName("VertexTitleLine");

    reshape();

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(3, 3);
    _blockRepresentation->setGraphicsEffect(shadowEffect);

    _blockRepresentation->move(0, 5);

    connect(this, SIGNAL(updateProp(GroupParamRepresentation*)), Window::getInstance(), SLOT(updatePropertyDock(GroupParamRepresentation*)));
    installEventFilter(this);
  }

  LinkConnexionRepresentation* GroupParamRepresentation::addNewInputParam(ParamDefinition def)
  {
    LinkConnexionRepresentation  *tmp = new LinkConnexionRepresentation(def._name, true, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    _listOfInputChilds.push_back(tmp);
    return tmp;
  }

  LinkConnexionRepresentation* GroupParamRepresentation::addNewOutputParam(ParamDefinition def)
  {
    LinkConnexionRepresentation  *tmp = new LinkConnexionRepresentation(def._name, false, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    _listOfOutputChilds.push_back(tmp);
    return tmp;
  }

  void GroupParamRepresentation::removeLink(BlockLink l){
    _links.erase(l);

    MainWidget* parent = dynamic_cast<MainWidget*>(parentWidget());
    if (parent == NULL)
      return;
    parent->getModel()->removeLink(l);
  };

  ConditionLinkRepresentation* GroupParamRepresentation::getCondition(ConditionOfRendering* cor, bool isLeft)
  {
    for (auto it : _linksConditions)
    {
      if (it.first == cor && (it.second->isLeftCond() == isLeft))
        return it.second;
    }
    return NULL;
  }

  void GroupParamRepresentation::reshape()
  {
    QRect sizeNameVertex = _vertexTitle->fontMetrics().boundingRect(_vertexTitle->text());

    if (_sizeIncrement.x() < 0)
      _sizeIncrement.setX(0);
    if (_sizeIncrement.y() < 0)
      _sizeIncrement.setY(0);

    //conditions:
    int heightOfConditions_tmp = 0;
    int maxWidth = 0;
    string conditionsText = "";
    for (ConditionOfRendering& condition : _conditions)
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
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (auto& tmp : _listOfInputChilds)
    {
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

    for (auto subPara : _listOfInputSubParams)
    {
      subPara.second->setMinimumWidth(5);
      subPara.second->move(-2, inputHeight);//move the name at the top of vertex...
      tmpSize = subPara.second->fontMetrics().boundingRect(subPara.second->text());
      subPara.second->setVisible(subPara.second->shouldShow());
      if (subPara.second->shouldShow())
      {
        showIn++;
        inputHeight += tmpSize.height() + 10;
        if (maxInputWidth < tmpSize.width())
          maxInputWidth = tmpSize.width();
      }
    }

    for (auto tmp : _listOfOutputChilds)
    {
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
    int newWidth = maxOutputWidth + maxInputWidth + _sizeIncrement.x() + 10;
    if (newWidth < sizeNameVertex.width() + 16)
      newWidth = sizeNameVertex.width() + 16;

    inputHeight = outputHeight = topPadding;
    if (showIn > showOut)
      outputHeight += (tmpSize.height() + 10) * (int)((static_cast<double>(showIn)-showOut) / 2.);
    else
      inputHeight += (tmpSize.height() + 10) * (int)((static_cast<double>(showOut)-showIn) / 2.);

    for (auto& tmp : _listOfInputChilds)
    {
      QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->resize(maxInputWidth, tmpSize.height() + 5);
      tmp->move(-2, inputHeight);//move the name at the top of vertex...
      if (tmp->shouldShow())
      {
        inputHeight += tmpSize.height() + 10;
        tmp->raise();
      }
    }
    for (auto tmp : _listOfInputSubParams)
    {
      QRect tmpSize = tmp.second->fontMetrics().boundingRect(tmp.second->text());
      tmp.second->resize(maxInputWidth, tmpSize.height() + 5);
      tmp.second->move(-2, inputHeight);//move the name at the top of vertex...
      if (tmp.second->shouldShow())
      {
        inputHeight += tmpSize.height() + 10;
        tmp.second->raise();
      }
    }
    for (auto tmp : _listOfOutputChilds)
    {
      QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
      tmp->resize(maxOutputWidth, tmpSize.height() + 5);
      tmp->move(newWidth - maxOutputWidth + 4, outputHeight);//move the name at the top of vertex...
      if (tmp->shouldShow())
      {
        outputHeight += tmpSize.height() + 10;
        tmp->raise();
      }
    }

    _vertexTitle->move((newWidth - sizeNameVertex.width()) / 2, 5);//move the name at the top of vertex...

    _lineTitle->resize(newWidth, 2);
    _lineTitle->move(0, sizeNameVertex.height() + 8);//move the name at the top of vertex...

    if (_sizeIncrement.y() < 10 && _preview != NULL)
      _sizeIncrement.setY(10);
    projectedHeight += max(inputHeight, outputHeight) + _sizeIncrement.y();

    if (_preview != NULL)
    {
      QLabel *label = dynamic_cast<QLabel *>(_preview);
      if (NULL != label)
      {
        _preview->resize(newWidth, _preview->height());
        _preview->move(0, outputHeight);//move the name at the top of vertex...
      }
      else
      {
        _preview->resize(newWidth, projectedHeight - sizeNameVertex.height() - 30);
        _preview->move(0, sizeNameVertex.height() + 10);//move the name at the top of vertex...
      }
    }

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
    for (ConditionOfRendering& condition : _conditions)
    {
      if (condition.getCategory_left() == 1)//output of block...
      {
        ConditionLinkRepresentation* tmp = getCondition(&condition, true);
        if (tmp == NULL)
        {
          tmp = new ConditionLinkRepresentation(&condition, true, _conditionsRepresentation);
          connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
          connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
          _linksConditions.push_back(std::pair<ConditionOfRendering*, ConditionLinkRepresentation*>(&condition, tmp));
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
          _linksConditions.push_back(std::pair<ConditionOfRendering*, ConditionLinkRepresentation*>(&condition, tmp));
        }
        QRect tmpSize = tmp->fontMetrics().boundingRect(tmp->text());
        tmp->move(newWidth - 32 - tmpSize.width(), heightOfConditions_tmp);//move the name at the top of vertex...
        tmp->show();
      }
      heightOfConditions_tmp += sizeNameVertex.height() + 5;
    }
    if (_conditions.empty())
    {
      while (!_linksConditions.empty())
      {
        delete _linksConditions.back().second;
        _linksConditions.pop_back();
      }
    }

    _blockRepresentation->raise();

    resize(newWidth + 10, projectedHeight);
  }

  void GroupParamRepresentation::setParamActiv(LinkConnexionRepresentation* param)
  {
    _paramActiv = param;
  }

  LinkConnexionRepresentation* GroupParamRepresentation::getParamRep(std::string paramName, bool input)
  {
    if (input)
    {
      for (auto& param : _listOfInputChilds)
        if (param->getName().compare(paramName) == 0)
          return param;
    }
    else
    {
      for (auto& param : _listOfOutputChilds)
        if (param->getName().compare(paramName) == 0)
          return param;
    }
    return NULL;
  }

  void GroupParamRepresentation::changeStyleProperty(const char* propertyName, QVariant val)
  {
    _blockRepresentation->setProperty(propertyName, val);
    _blockRepresentation->style()->unpolish(_blockRepresentation);
    _blockRepresentation->style()->polish(_blockRepresentation);
    _blockRepresentation->update();
  }

  void GroupParamRepresentation::setSelected(bool isSelected)
  {
    bool prevProperty = _blockRepresentation->property("selected").toBool();
    if (prevProperty == isSelected)
      return;//nothing to do: already in correct state...
    changeStyleProperty("selected", isSelected);
    if (isSelected)
      _selectedBlock.push_back(this);
    else
    {
      //remove from list:
      for (unsigned int pos = 0; pos < _selectedBlock.size(); pos++)
      {
        if (_selectedBlock[pos] == this)
        {
          _selectedBlock.erase(_selectedBlock.begin() + pos);
          return;
        }
      }
    }
  }

  void GroupParamRepresentation::resetSelection()
  {
    for (auto selection : _selectedBlock)
      selection->changeStyleProperty("selected", false);
    _selectedBlock.clear();
  }

  void GroupParamRepresentation::mousePressEvent(QMouseEvent *mouseE)
  {
    QPoint mouseP = mouseE->pos();
    if (_paramActiv == NULL && mouseE->button() == Qt::LeftButton)
    {
      _isMoving = false;
      switch (_blockRepresentation->getState())
      {
      case MainVertexBlock::MouseOut://not inside block, not a selection...
        if (!mouseE->modifiers().testFlag(Qt::ControlModifier))
          resetSelection();
        break;
      case MainVertexBlock::MouseHorResize:
      case MainVertexBlock::MouseVerResize:
      case MainVertexBlock::MouseDiagResize:
        startClick_ = mouseE->globalPos();
        break;
      case MainVertexBlock::MouseDragOpen:
      {
        startClick_ = mouseE->globalPos();
        setCursor(Qt::ClosedHandCursor); 
      }
      case MainVertexBlock::MouseIn:
      default:
      {
        bool prevProperty = _blockRepresentation->property("selected").toBool();
        //if not previously selected, we reset the selection list...
        if (!prevProperty && !mouseE->modifiers().testFlag(Qt::ControlModifier))
          resetSelection();
        setSelected(true);
      }
      }
    }
    else
    {
      if (_paramActiv != NULL)
        _blockRepresentation->setState(MainVertexBlock::MouseCreateLink);
      mouseE->ignore();
    }
    raise();
  }

  void GroupParamRepresentation::mouseReleaseEvent(QMouseEvent *mouseE)
  {
    switch (_blockRepresentation->getState())
    {
    case MainVertexBlock::MouseOut://not inside block, not a selection...
      if (!mouseE->modifiers().testFlag(Qt::ControlModifier))
        resetSelection();
      break;
    case MainVertexBlock::MouseHorResize:
    case MainVertexBlock::MouseVerResize:
    case MainVertexBlock::MouseDiagResize:
      break;//nothing to do, cursor don't have to change
    case MainVertexBlock::MouseDragClose:
    case MainVertexBlock::MouseDragOpen:
      setCursor(Qt::OpenHandCursor);
      break;
    default://in any other case, use the default cursor
      setCursor(Qt::ArrowCursor);
      break;
    }

    if (!_isMoving && mouseE->button() == Qt::LeftButton)
      emit updateProp(this);
  }

  void GroupParamRepresentation::moveDelta(QPoint delta)
  {
    QPoint newPos = pos() + delta;
    move(newPos.x(), newPos.y());
    Window::getInstance()->redraw();
  }

  void GroupParamRepresentation::mouseMoveEvent(QMouseEvent *mouseE)
  {
    switch (_blockRepresentation->getState())
    {
    case MainVertexBlock::MouseHorResize:
    {
      QPoint p = mouseE->globalPos();
      p.setY(startClick_.y());
      _sizeIncrement = _sizeIncrement + (p - startClick_);
      startClick_ = p;
      reshape();
      Window::getInstance()->redraw();
    };
      break;
    case MainVertexBlock::MouseVerResize:
    {
      QPoint p = mouseE->globalPos();
      p.setX(startClick_.x());
      _sizeIncrement = _sizeIncrement + (p - startClick_);
      startClick_ = p;
      reshape();
      Window::getInstance()->redraw();
    };
      break;
    case MainVertexBlock::MouseDiagResize:
    {
      QPoint p = mouseE->globalPos();
      _sizeIncrement = _sizeIncrement + (p - startClick_);
      startClick_ = p;
      reshape();
      Window::getInstance()->redraw();
    };
      break;
    case MainVertexBlock::MouseDragClose:
    case MainVertexBlock::MouseDragOpen:
    {
      QPoint p = mouseE->globalPos();
      QPoint deltaClick_ = p - startClick_;
      startClick_ = p;
      _isMoving = true;
      for (GroupParamRepresentation* vRep : _selectedBlock)
        vRep->moveDelta(deltaClick_);
    };
    break;
    case MainVertexBlock::MouseIn:
    case MainVertexBlock::MouseCreateLink:
    default://nothing to do
      break;
    }
    updatePosition();
    mouseE->ignore();

  }

  void GroupParamRepresentation::mouseDoubleClickEvent(QMouseEvent *mouseE)
  {
  }
  
  void GroupParamRepresentation::enterEvent(QEvent *e)
  {
    QRect prevSize = geometry();
    setGeometry(QRect(prevSize.x(), prevSize.y() - heightOfConditions, prevSize.width(), prevSize.height() + heightOfConditions));
    _blockRepresentation->move(0, heightOfConditions + 5);
  }

  void GroupParamRepresentation::leaveEvent(QEvent *)
  {
    QRect prevSize = geometry();
    setGeometry(QRect(prevSize.x(), prevSize.y() + heightOfConditions, prevSize.width(), prevSize.height() - heightOfConditions));
    _blockRepresentation->move(0, 5);
  }

  void GroupParamRepresentation::updatePreview(ParamValue* val)
  {
    if (val != NULL)
    {
      bool isNew = false;
      switch (val->getType(false))
      {
      case Boolean:
        if (_preview == NULL)
        {
          isNew = true;
          _preview = new QLabel(_blockRepresentation);
        }
        if (val->get<bool>())
          ((QLabel*)_preview)->setText(_QT("TRUE"));
        else
          ((QLabel*)_preview)->setText(_QT("FALSE"));
        break;
      case Int:
        if (_preview == NULL)
        {
          isNew = true;
          _preview = new QLabel(_blockRepresentation);
        }
        ((QLabel*)_preview)->setText(QString::number(val->get<int>()));
        break;
      case Float:
        if (_preview == NULL)
        {
          isNew = true;
          _preview = new QLabel(_blockRepresentation);
        }
        ((QLabel*)_preview)->setText(QString::number(val->get<double>()));
        break;
      case Matrix:
      {
        cv::Mat img = val->get<cv::Mat>();
        if ((img.cols / (double)img.rows < 0.1) || (img.rows / (double)img.cols < 0.1))
          break;//nothing to do, not an image, probably a vector...
        if ((img.cols < 20) || (img.rows < 20))
          break;//too small image...
        if (_preview == NULL)
        {
          isNew = true;
          _preview = new ImageViewer(_blockRepresentation);
        }
        ((ImageViewer*)_preview)->setImage(img);
        break;
      }
      case String:
      case FilePath:
      case Color:
        if (_preview == NULL)
        {
          isNew = true;
          _preview = new QLabel(_blockRepresentation);
        }
        ((QLabel*)_preview)->setText(val->toString().c_str());
        break;
      default://nothing to do...
        break;
      }

      if (isNew)
      {
        QLabel *label = dynamic_cast<QLabel *>(_preview);
        if (NULL != label)
          label->setAlignment(Qt::AlignCenter);

        _preview->setVisible(true);
        reshape();
      }
    }
  }
  void GroupParamRepresentation::setPreview(LinkConnexionRepresentation* paramSelected)
  {
    ParamRepresentation* paramRep= dynamic_cast<ParamRepresentation *>(paramSelected);
    if (NULL != paramRep)
    {
      if (_previewLink!=NULL)
        disconnect(_previewLink, SIGNAL(paramUpdated()), this, SLOT(paramUpdated()));

      if (_previewLink != paramSelected)
      {
        _previewLink = paramSelected;
        ParamValue* val = paramRep->getParamValue();
        updatePreview(val);
        connect(val, SIGNAL(paramUpdated()), SLOT(paramUpdated()));
      }
      else
      {
        delete _preview;
        _previewLink = NULL;
        _preview = NULL;
      }
    }
  }


  VertexRepresentation::~VertexRepresentation()
  {
    GraphLayout* representation = dynamic_cast<GraphLayout*>(layout());
    if (representation!=NULL)
      representation->removeLinks(this);
  }

  VertexRepresentation::VertexRepresentation(Block* model):
    GroupParamRepresentation(model != NULL ? model->getName():"_NULL_MODEL_")
  {
    _blockRepresentation->setObjectName("VertexRepresentation");

    _hasDynamicParams = dynamic_cast<SubBlock*>(model) != NULL;
    _model = model;

    createListParamsFromModel();

    _sizeIncrement.setX(static_cast<int>(model->getSizeIncrement().x));
    _sizeIncrement.setY(static_cast<int>(model->getSizeIncrement().y));

    reshape();

    move((int)model->getPosition().x, (int)model->getPosition().y);
    _blockRepresentation->move(0, 5);
  }

  void VertexRepresentation::enterEvent(QEvent *e)
  {
    string msg = _STR("PROCESSING_TIME") + lexical_cast<std::string>(_model->getPerf()) + "ms";
    setToolTip(msg.c_str());
    GroupParamRepresentation::enterEvent(e);
  }

  void VertexRepresentation::updatePosition()
  {
    _model->setIncrSize(static_cast<float>(_sizeIncrement.x()),
      static_cast<float>(_sizeIncrement.y()));
  }

  void VertexRepresentation::moveDelta(QPoint delta)
  {
    QPoint newPos = pos() + delta;
    _model->setPosition(newPos.x(), newPos.y(), 
      static_cast<float>(_sizeIncrement.x()), static_cast<float>(_sizeIncrement.y()));
    GroupParamRepresentation::moveDelta(delta);
  }

  void VertexRepresentation::mouseDoubleClickEvent(QMouseEvent *mouseE)
  {
    SubBlock* subBlock = dynamic_cast<SubBlock*>(_model);
    if (subBlock != NULL)
    {
      Window* win = Window::getInstance();
      MainWidget_SubGraph* handler = new MainWidget_SubGraph(subBlock);
      win->addTab(handler, "subGraph");
      win->synchroMainGraph();
    }
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


  LinkConnexionRepresentation* VertexRepresentation::addNewInputParam(ParamDefinition def)
  {
    ParamRepresentation  *tmp = new ParamRepresentation(_model, def, true, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    _listOfInputChilds.push_back(tmp);
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
            const ParamDefinition* tmpDef = param->getDefinition();
            ParamRepresentation *tmp = new ParamRepresentation(_model, *tmpDef, true, _blockRepresentation);
            tmp->setVisibility(false);
            tmp->isSubParam(true);
            connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
            connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
            _listOfInputSubParams[tmpDef->_name] = tmp;
          }
        }
      }
    }
    return tmp;
  }

  LinkConnexionRepresentation* VertexRepresentation::addNewOutputParam(ParamDefinition def)
  {
    ParamRepresentation  *tmp = new ParamRepresentation(_model, def, false, _blockRepresentation);
    connect(tmp, SIGNAL(creationLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(initLinkCreation(QPoint)));
    connect(tmp, SIGNAL(releaseLink(QPoint)), Window::getInstance()->getMainWidget(), SLOT(endLinkCreation(QPoint)));
    _listOfOutputChilds.push_back(tmp);
    return tmp;
  }

  void VertexRepresentation::reshape()
  {
    //update conditions:
    _conditions = _model->getConditions();
    //reshape classical:
    GroupParamRepresentation::reshape();
  }

  void VertexRepresentation::paramUpdated()
  {
    ParamValue* val = dynamic_cast<ParamValue*>(sender());
    if (val != NULL)
      updatePreview(val);
  }
}