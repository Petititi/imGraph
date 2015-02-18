#ifndef _GRAPHICVIEW_HEADER_
#define _GRAPHICVIEW_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4244 4251 4275 4800 4503)
#endif
#include <QGraphicsView>
#include <QResizeEvent>
#include <QDialog >
#include <QPainter>
#include <QRect>
#include <QString>
#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QMainWindow>
#include <QComboBox>
#include <QDial>
#include <QCheckBox>
#include <QPainterPath>
#include <QGroupBox>

#include <boost/bimap.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

#include <map>
#include "Window.h"

namespace charliesoft
{
  class SubBlock;
  class LinkPath;
  class VertexRepresentation;
  class LinkConnexionRepresentation;
  class SubGraphParamRepresentation;

  class GraphLayout :public QLayout
  {
    Q_OBJECT;
    boost::mutex _mtx;    // explicit mutex declaration

    std::map<Block*, QLayoutItem*> _items;
    std::map<BlockLink, LinkPath*> _links;
    std::map<BlockLink, LinkPath*> _sublinks;
    std::vector<Block*> _orderedBlocks;
  public:

    void removeLinks(VertexRepresentation* vertex);
    void removeSelectedLinks();
    void addLink(const BlockLink& link, LinkPath* path=NULL);

    void clearLayout(QLayout* layout = NULL);
    const std::map<Block*, QLayoutItem*>& getItems() const { return _items; }
    const std::map<BlockLink, LinkPath*>& getLinks() const { return _links; }
    const std::map<BlockLink, LinkPath*>& getSubLinks() const { return _sublinks; }

    virtual void addItem(QLayoutItem *);
    virtual QLayoutItem * itemAt(int index) const;
    virtual QLayoutItem * takeAt(int index);
    virtual int indexOf(QWidget *) const;
    virtual int indexOf(Block *) const;
    virtual int count() const;
    virtual QSize sizeHint() const;

    void drawEdges(QPainter& p);
    VertexRepresentation* getVertexRepresentation(Block* b);

    public slots:
    void synchronize();
  };

  class MainWidget :public QWidget
  {
    Q_OBJECT;

    charliesoft::GraphOfProcess *_model;

    LinkConnexionRepresentation* _startParam;
    QPoint _startMouse;
    QPoint _endMouse;
    QRectF _selectBox;
    bool _isSelecting;
    bool _creatingLink;

    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);
  public:
    MainWidget(GraphOfProcess *model);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    charliesoft::GraphOfProcess *getModel() const { return _model; };

    public slots:
    void initLinkCreation(QPoint start);
    void endLinkCreation(QPoint end);
  };

  class MainWidget_SubGraph :public MainWidget
  {
  protected:
    SubBlock* _subModel;
    int _posInput;
    int _posOutput;

    std::map<std::string, SubGraphParamRepresentation*> _params;

    void addParameter(SubGraphParamRepresentation* param);
    virtual void	resizeEvent(QResizeEvent * event);
  public:
    MainWidget_SubGraph(SubBlock *model);

    SubBlock* getSubModel() const { return _subModel; }

    SubGraphParamRepresentation* getParamRepresentation(std::string paramName) const;

    void addNewParamLink(const BlockLink& link);
    void removeParamLink(const BlockLink& link);
  };

  class MainWidget_ForGraph :public MainWidget_SubGraph
  {
  public:
    MainWidget_ForGraph(SubBlock *model);
  };
}


#endif