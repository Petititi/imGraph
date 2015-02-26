
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
#include <QMimeData>
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
  GraphLayout::GraphLayout()
  {
    connect(this, SIGNAL(updateDock(GroupParamRepresentation*)), Window::getInstance(), SLOT(updatePropertyDock(GroupParamRepresentation*)));
  }

  void GraphLayout::addItem(QLayoutItem * item)
  {
    //get widget:
    if (VertexRepresentation* derived = dynamic_cast<VertexRepresentation*>(item->widget())) {
      _orderedBlocks.push_back(derived->getModel());
      _items[derived->getModel()] = item;
    }
  }

  QLayoutItem * GraphLayout::itemAt(int index) const
  {
    if (index >= (int)_orderedBlocks.size())
      return NULL;
    try {
      QLayoutItem * tmp = _items.at(_orderedBlocks[index]);
      return tmp;
    }
    catch (const std::out_of_range&) {
      return NULL;
    }
  }

  QLayoutItem * GraphLayout::takeAt(int index)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    if (index >= (int)_items.size())
      return NULL;
    QLayoutItem *output = _items[_orderedBlocks[index]];
    _items.erase(_orderedBlocks[index]);
    _orderedBlocks.erase(_orderedBlocks.begin() + index);
    ///\todo: remove edges!
    return output;
  }

  int GraphLayout::indexOf(QWidget *widget) const
  {
    for (size_t i = 0; i < _orderedBlocks.size(); i++)
    {
      try {
        if (_items.at(_orderedBlocks[i])->widget() == widget)
          return i;
      }
      catch (const std::out_of_range&) {
        return -1;
      }
    }
    return -1;
  }

  int GraphLayout::indexOf(Block *widget) const
  {
    for (size_t i = 0; i < _orderedBlocks.size(); i++)
    {
      if (_orderedBlocks[i] == widget)
        return i;
    }
    return -1;
  }

  int GraphLayout::count() const
  {
    return _orderedBlocks.size();
  }

  QSize GraphLayout::sizeHint() const
  {
    QSize mySize(800,600);
    //test if block still exist:
    for (auto it = _items.begin(); it != _items.end(); it++)
    {
      const QRect& rect = it->second->widget()->geometry();
      if (rect.x() + rect.width()>mySize.width())
        mySize.setWidth(rect.x() + rect.width());
      if (rect.y() + rect.height() > mySize.height())
        mySize.setHeight(rect.y() + rect.height());
    }
    return mySize;
  }

  void GraphLayout::removeLinks(VertexRepresentation* vertex)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    //remove every links connected to vertex (in and out):
    map<BlockLink, LinkPath*> links = vertex->getLinks();
    for (auto link : links)
    {
      if (_links.find(link.first) != _links.end())
      {
        _links.erase(link.first);//delete map association...
        if (_items.find(link.first._to) != _items.end())
          dynamic_cast<VertexRepresentation*>(_items[link.first._to]->widget())->removeLink(link.first);

        delete link.second;//delete LinkPath
      }
    }
  }

  void GraphLayout::removeSelectedLinks()
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    auto it = _links.begin();
    while (it != _links.end())
    {
      if (it->second->isSelected())
      {
        if (_items.find(it->first._to) != _items.end())
          dynamic_cast<VertexRepresentation*>(_items[it->first._to]->widget())->removeLink(it->first);

        delete it->second;//delete LinkPath

        _links.erase(it);//delete map association...
        it = _links.begin();//as "it" is now in an undefined state...
      }
      else
        it++;
    }

    MainWidget_SubGraph* mainWidget = dynamic_cast<MainWidget_SubGraph*>(parentWidget());
    if (mainWidget != NULL)
    {
      auto it1 = _sublinks.begin();
      while (it1 != _sublinks.end())
      {
        if (it1->second->isSelected())
        {
          mainWidget->removeParamLink(it1->first);
          delete it1->second;//delete LinkPath

          _sublinks.erase(it1);//delete map association...
          it1 = _sublinks.begin();//as "it" is now in an undefined state...
        }
        else
          it1++;
      }
    }
  }


  void GraphLayout::addLink(const BlockLink& link, LinkPath* path)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    if (_links.find(link) != _links.end())
      return;//nothing to do... Already exist

    if (path == NULL)
    {
      VertexRepresentation* fromVertex, *toVertex;
      fromVertex = dynamic_cast<VertexRepresentation*>(_items[link._from]->widget());
      toVertex = dynamic_cast<VertexRepresentation*>(_items[link._to]->widget());
      if (fromVertex != NULL && toVertex != NULL)
      {
        auto paramFrom = fromVertex->getParamRep(link._fromParam, false);
        auto paramTo = toVertex->getParamRep(link._toParam, true);
        paramFrom->setVisibility(true);
        paramTo->setVisibility(true);
        path = new LinkPath(paramFrom, paramTo);
        fromVertex->addLink(link, path);
        toVertex->addLink(link, path);
        _links[link] = path;
      }
    }
    else
      _sublinks[link] = path;
  }

  void GraphLayout::clearLayout(QLayout* layout)
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

  void GraphLayout::drawEdges(QPainter& p)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    for (auto iter : _links)
      iter.second->draw(&p, NULL, NULL);
    for (auto iter : _sublinks)
      iter.second->draw(&p, NULL, NULL);
  }

  VertexRepresentation* GraphLayout::getVertexRepresentation(Block* b)
  {
    boost::unique_lock<boost::mutex> lock(_mtx);
    if (_items.find(b) == _items.end())
      return NULL;
    return dynamic_cast<VertexRepresentation*>(_items[b]->widget());
  }

  void GraphLayout::synchronize()
  {
    MainWidget* parent = dynamic_cast<MainWidget*>(parentWidget());
    if (parent == NULL)
      return;

    //for each vertex, we look for the corresponding representation:
    std::vector<Block*> blocks = parent->getModel()->getVertices();
    for (auto it = blocks.begin(); it != blocks.end(); it++)
    {
      if (_items.find(*it) == _items.end())//add this vertex to view:
      {
        VertexRepresentation* blockRep = new VertexRepresentation(*it);
        addWidget(blockRep);
      }
    }
    GroupParamRepresentation* dockingBlock = Window::getInstance()->getParamDock();
    //test if block still exist:
    auto it_ = _items.begin();
    while (it_ != _items.end())
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
        //delete edges if needed:
        auto link = _links.begin();
        while (link != _links.end())
        {
          if ((it_->first == link->first._from) ||
            (it_->first == link->first._to))
          {
            delete link->second;
            auto tmpLink = link;
            _links.erase(tmpLink);
            link = _links.begin();//restart iteration (we can't presume for iterator position)
          }
          else
            link++;
        }
        
        //remove widget from representation
        auto representation = takeAt(pos);
        if (dockingBlock == representation->widget())
          emit updateDock(NULL);

        delete representation->widget();
        delete representation;

        it_ = _items.begin();//restart iteration (we can't presume for iterator position)
      }
      else
      {
        //activate preview:
        VertexRepresentation *vr = dynamic_cast<VertexRepresentation *>(it_->second->widget());
        if (NULL != vr && it_->first->getCurrentPreview().compare("None") == string::npos)
        {
          //get link representation:
          LinkConnexionRepresentation* tmpRep = vr->getParamRep(it_->first->getCurrentPreview(), false);
          if (tmpRep == NULL)
            tmpRep = vr->getParamRep(it_->first->getCurrentPreview(), true);
          if (tmpRep != NULL)
            vr->setPreview(tmpRep, true);
        }
        it_++;
      }
    }

    //test if link still exist:
    auto link = _links.begin();
    while (link != _links.end())
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
      if (link != _links.end())
        link++;
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

    MainWidget_SubGraph* handler = dynamic_cast<MainWidget_SubGraph*>(parent);
    if (handler != NULL)
    {
      SubBlock* subModel = handler->getSubModel();
      //if we are a subgraph, we also draw the sublinks:
      LinkConnexionRepresentation* paramFrom, *paramTo;

      const std::vector<BlockLink>& externBlocksInput = subModel->getExternBlocksInput();
      for (auto link : externBlocksInput)
      {
        if (_sublinks.find(link) == _sublinks.end())
        {
          paramFrom = getVertexRepresentation(link._to)->getParamRep(link._toParam, true);
          paramTo = handler->getParamRepresentation(link._fromParam);

          LinkPath* path = new LinkPath(paramTo, paramFrom);
          addLink(link, path);
        }
      }

      const std::vector<BlockLink>& externBlocksOutput = subModel->getExternBlocksOutput();
      for (auto link : externBlocksOutput)
      {
        if (_sublinks.find(link) == _sublinks.end())
        {
          paramFrom = handler->getParamRepresentation(link._toParam);
          paramTo = getVertexRepresentation(link._from)->getParamRep(link._fromParam, false);

          LinkPath* path = new LinkPath(paramTo, paramFrom);
          addLink(link, path);
        }
      }
    }

    Window::getInstance()->redraw();
  }

  MainWidget::MainWidget(charliesoft::GraphOfProcess *model)
  {
    _model = model;
    setObjectName("MainWidget");
    _isSelecting = _creatingLink = false;
    _startParam = NULL;
    setAcceptDrops(true);

    setLayout(new GraphLayout());
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

    Window::synchroMainGraph();//as we updated the model, we ask the layout to redraw itself...
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

    if (_creatingLink)
      painter.drawLine(_startMouse, _endMouse);

    if (_isSelecting)
    {
      painter.fillRect((int)_selectBox.x(), (int)_selectBox.y(), (int)_selectBox.width(), (int)_selectBox.height(),
        QColor(0, 0, 255, 128));
      painter.setPen(QColor(0, 0, 200, 255));
      painter.drawRect(_selectBox);
    }

    //now ask each vertex to draw the links:
    dynamic_cast<GraphLayout*>(layout())->drawEdges(painter);
  }

  void MainWidget::mouseMoveEvent(QMouseEvent *me)
  {
    if (_creatingLink)
    {
      _endMouse = me->pos();
      Window::getInstance()->redraw();
    }
    if (_isSelecting)
    {
      _selectBox.setCoords(_startMouse.x(), _startMouse.y(),
        me->x() + 1, me->y() + 1);
      //test intersection between vertex representation and selection rect:
      GraphLayout* representation = dynamic_cast<GraphLayout*>(layout());
      if (representation != NULL)
      {
        const std::map<Block*, QLayoutItem*>& items = representation->getItems();

        for (auto& item : items)
        {
          if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(item.second->widget()))
          {
            if (vertex->geometry().intersects(_selectBox.toRect()))
              vertex->setSelected(true);
            else if (!me->modifiers().testFlag(Qt::ControlModifier))
              vertex->setSelected(false);
          }
        }

        const std::map<BlockLink, LinkPath*>& links = representation->getLinks();
        for (auto& link : links)
        {
          if (link.second->intersect(_selectBox.toRect()))
            link.second->setSelected(true);
          else if (!me->modifiers().testFlag(Qt::ControlModifier))
            link.second->setSelected(false);
        }

        const map<BlockLink, LinkPath*>& links_sub = representation->getSubLinks();
        for (auto& link : links_sub)
        {
          if (link.second->intersect(_selectBox.toRect()))
            link.second->setSelected(true);
          else if (!me->modifiers().testFlag(Qt::ControlModifier))
            link.second->setSelected(false);
        }
      }
      Window::getInstance()->redraw();
    }
  };

  void MainWidget::mousePressEvent(QMouseEvent *mouseE)
  {
    if (!_creatingLink && mouseE->button() == Qt::LeftButton)
    {
      _startMouse = mouseE->pos();
      //begin rect:
      _selectBox.setCoords(_startMouse.x()-3, _startMouse.y()-3,
        _startMouse.x() + 3, _startMouse.y() + 3);

      if (!mouseE->modifiers().testFlag(Qt::ControlModifier))
      {
        GraphLayout* representation = dynamic_cast<GraphLayout*>(layout());
        const std::map<BlockLink, LinkPath*>& links = representation->getLinks();
        for (auto link : links)
          link.second->setSelected(link.second->intersect(_selectBox.toRect()));

        VertexRepresentation::resetSelection();
      }
      
      _isSelecting = true;
    }
  }
  
  void MainWidget::mouseReleaseEvent(QMouseEvent *)
  {
    size_t nbSelected = VertexRepresentation::getSelection().size();
    if (nbSelected == 1)
      Window::getInstance()->showListAlgoDock(false);
    else
      Window::getInstance()->showListAlgoDock(true);
    _creatingLink = _isSelecting = false;
    Window::getInstance()->redraw();
  }

  void MainWidget::endLinkCreation(QPoint end)
  {
    _endMouse = end;
    _creatingLink = false;
    //find an hypotetic param widget under mouse:
    QWidget* childAtEnd = childAt(_endMouse);
    ParamRepresentation* param = dynamic_cast<ParamRepresentation*>(childAtEnd);
    ParamRepresentation* startParam = dynamic_cast<ParamRepresentation*>(_startParam);
    if (param == startParam)
    {
      //someone want to select this block as preview!
      VertexRepresentation* paramRep = param->getVertex();
      if (paramRep != NULL)
        paramRep->setPreview(param);

      return;
    }

    if (param != NULL && startParam != NULL)
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
          _model->createLink(startParam->getModel(), startParam->getParamName(), param->getModel(), param->getParamName());
        else
          _model->createLink(param->getModel(), param->getParamName(), startParam->getModel(), startParam->getParamName());
      }
      catch (ErrorValidator& e)
      {
        QMessageBox messageBox;
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), e.errorMsg.c_str());
        return;
      }
      param->getModel()->getGraph()->initChildDatas(param->getModel(), std::set<Block*>());
    }

    //if one is ConditionLinkRepresentation, other is ParamRepresentation:
    ConditionLinkRepresentation* condParam;
    if (param != NULL)
      condParam = dynamic_cast<ConditionLinkRepresentation*>(_startParam);
    if (startParam != NULL)
    {
      param = startParam;
      condParam = dynamic_cast<ConditionLinkRepresentation*>(childAtEnd);
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
    }

    //if one is ConditionLinkRepresentation, other is ParamRepresentation:
    SubGraphParamRepresentation* subGraphParam;
    if (param != NULL)
      subGraphParam = dynamic_cast<SubGraphParamRepresentation*>(_startParam);
    if (startParam != NULL)
    {
      param = startParam;
      subGraphParam = dynamic_cast<SubGraphParamRepresentation*>(childAtEnd);
    }
    if (param != NULL &&subGraphParam != NULL)
    {
      //If we are here, we are a MainWidget_SubGraph:
      MainWidget_SubGraph* subGraphWidget = dynamic_cast<MainWidget_SubGraph*>(this);

      //we should link input on output(or vice versa)
      if (param->isInput() == subGraphParam->isInput())
      {
        string typeLink = param->isInput() ? _STR("BLOCK_INPUT") : _STR("BLOCK_OUTPUT");
        QMessageBox messageBox;
        string msg = (my_format(_STR("ERROR_LINK_WRONG_INPUT_OUTPUT")) % _STR(startParam->getParamName()) % _STR(param->getParamName()) % typeLink).str();
        messageBox.critical(0, _STR("ERROR_GENERIC_TITLE").c_str(), msg.c_str());
        return;
      }

      if (param->isInput())
        subGraphWidget->addNewParamLink(BlockLink(subGraphParam->getModel(), param->getModel(),
                subGraphParam->getDefinition()._name, param->getParamName()));
      else
        subGraphWidget->addNewParamLink(BlockLink(param->getModel(), subGraphParam->getModel(),
        param->getParamName(), subGraphParam->getDefinition()._name));
    }
    Window::synchroMainGraph();

    Window::getInstance()->redraw();//redraw window...
  }

  void MainWidget::initLinkCreation(QPoint start)
  {
    _startMouse = _endMouse = start;
    _creatingLink = true;
    _startParam = dynamic_cast<LinkConnexionRepresentation*>(sender());
  }

  QSize MainWidget::sizeHint() const
  {
    QLayout* myLayout = layout();
    if (myLayout != NULL)
      return myLayout->sizeHint();
    return QSize(800, 600);
  }

  QSize MainWidget::minimumSizeHint() const
  {
    return sizeHint();
  }


  MainWidget_ForGraph::MainWidget_ForGraph(SubBlock *model) : MainWidget_SubGraph(model)
  {
  };

  MainWidget_SubGraph::MainWidget_SubGraph(SubBlock *model) :
    MainWidget(model->getSubGraph())
  {
    _posInput = _posOutput = 10;
    _subModel = model;
    //add input/output parameters:

    const vector<ParamDefinition>& inputParams = model->getInParams();
    QRect tmpSize;
    int showIn = 0, showOut = 0;
    for (size_t i = 0; i < inputParams.size(); i++)
      addParameter(new SubGraphParamRepresentation(_subModel, inputParams[i], false, this));

    const vector<ParamDefinition>& outputParams = _subModel->getOutParams();
    for (size_t i = 0; i < outputParams.size(); i++)
      addParameter(new SubGraphParamRepresentation(_subModel, outputParams[i], true, this));
  };

  void MainWidget_SubGraph::addParameter(SubGraphParamRepresentation* param)
  {
    _params[param->getDefinition()._name] = param;
    layout()->addWidget(param);
    connect(param, SIGNAL(creationLink(QPoint)), this, SLOT(initLinkCreation(QPoint)));
    connect(param, SIGNAL(releaseLink(QPoint)), this, SLOT(endLinkCreation(QPoint)));
    QRect sizeNameVertex = param->fontMetrics().boundingRect(param->text());
    int newWidth = sizeNameVertex.width() + 15;
    if (newWidth < 50)newWidth = 50;
    int newHeight = sizeNameVertex.height() + 5;
    if (newHeight < 25)newHeight = 25;
    param->resize(newWidth, newHeight);
    if (!param->isInput())
    {
      param->move(5, _posInput);//move the name at the top of vertex...
      _posInput += newHeight + 10;
    }
    else
    {
      param->move(width() - sizeNameVertex.width() - 5, _posOutput);//move the name at the top of vertex...
      _posOutput += newHeight + 10;
    }
  };

  void MainWidget_SubGraph::removeParamLink(const BlockLink& link)
  {
    _subModel->removeExternLink(link);
  }

  void MainWidget_SubGraph::addNewParamLink(const BlockLink& link)
  {
    GraphLayout* graphRep = dynamic_cast<GraphLayout*>(layout());
    if (graphRep == NULL)
      return;//Nothing to do...

    //get parameters link:
    LinkConnexionRepresentation* paramFrom, *paramTo;
    if (link._from->getGraph() == _subModel->getSubGraph())
    {
      paramFrom = _params[link._toParam];
      paramTo = graphRep->getVertexRepresentation(link._from)->getParamRep(link._fromParam, false);
      _subModel->addExternLink(link, false);
    }
    else
    {
      paramFrom = graphRep->getVertexRepresentation(link._to)->getParamRep(link._toParam, true);
      paramTo = _params[link._fromParam];
      _subModel->addExternLink(link, true);
    }

    LinkPath* path = new LinkPath(paramTo, paramFrom);
    graphRep->addLink(link, path);
  }

  SubGraphParamRepresentation* MainWidget_SubGraph::getParamRepresentation(
    std::string paramName) const
  {
    auto it = _params.find(paramName);
    if (it == _params.end())
      return NULL;
    else
      return it->second;
  }

  void MainWidget_SubGraph::resizeEvent(QResizeEvent * event)
  {
    int myWidth = width();
    for (auto param : _params)
    {
      QRect sizeNameVertex = param.second->geometry();
      if (param.second->isInput())
      {
        param.second->move(myWidth - sizeNameVertex.width() - 5, sizeNameVertex.y());
      }
    }
  }
}