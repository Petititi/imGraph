#ifndef _GRAPHICVIEW_HEADER_
#define _GRAPHICVIEW_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
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
#include "Configuration.h"
#include "Connectors.h"

namespace charliesoft
{

  class VertexRepresentation :public QWidget
  {
    Q_OBJECT;

    QWidget* _blockRepresentation;
    QWidget* _conditionsRepresentation;
    QFrame* _lineTitle;
    QLabel* _vertexTitle;
    QLabel* _conditionTitle;
    QLabel* _conditionsValues;
    Block* _model;
    bool _isDragging;
    bool _isMoving;
    bool _hasDynamicParams;
    QPoint startClick_;
    LinkConnexionRepresentation* _paramActiv;
    int heightOfConditions;

    std::map<BlockLink, LinkPath*> _links;
    std::vector< std::pair<ConditionOfRendering*, ConditionLinkRepresentation*> > linksConditions_;
    std::map<std::string, ParamRepresentation*> listOfInputChilds_;
    std::map<std::string, ParamRepresentation*> listOfInputSubParams_;
    std::map<std::string, ParamRepresentation*> listOfOutputChilds_;
    std::vector<ParamRepresentation*> listOfSubParams_;
  public:
    VertexRepresentation(Block* model);
    ~VertexRepresentation();

    void createListParamsFromModel();
    bool hasDynamicParams() const { return _hasDynamicParams; };

    ParamRepresentation* addNewInputParam(ParamDefinition def);
    ParamRepresentation* addNewOutputParam(ParamDefinition def);

    Block* getModel() const { return _model; }
    void setParamActiv(LinkConnexionRepresentation*);

    std::map<BlockLink, LinkPath*> getLinks() const { return _links; }

    void addLink(BlockLink l, LinkPath* p){
      _links[l] = p;
    };
    void removeLink(BlockLink l);

    void changeStyleProperty(const char* propertyName, QVariant val);
    void setSelected(bool isSelected);
    static void resetSelection();
    static std::vector<VertexRepresentation*> getSelection(){
      return selectedBlock_;
    };
    ParamRepresentation* getParamRep(std::string paramName, bool input);
    std::map<std::string, ParamRepresentation*>& getListOfInputChilds() { return listOfInputChilds_; }
    std::map<std::string, ParamRepresentation*>& getListOfSubParams() { return listOfInputSubParams_; }
    std::map<std::string, ParamRepresentation*>& getListOfOutputChilds() { return listOfOutputChilds_; }
  protected:
    ConditionLinkRepresentation* getCondition(ConditionOfRendering*, bool isLeft);
    static std::vector<VertexRepresentation*> selectedBlock_;
    void moveDelta(QPoint delta);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    public slots:
    void reshape();

  signals:
    void updateProp(VertexRepresentation*);
  };

  class GraphRepresentation :public QLayout
  {
    Q_OBJECT;
    boost::mutex _mtx;    // explicit mutex declaration

    std::map<Block*, QLayoutItem*> _items;
    std::map<BlockLink, LinkPath*> _links;
    std::vector<Block*> orderedBlocks_;
  public:

    void removeLinks(VertexRepresentation* vertex);
    void removeSelectedLinks();
    void addLink(const BlockLink& link);

    void clearLayout(QLayout* layout = NULL);
    std::map<Block*, QLayoutItem*> getItems() const { return _items; }
    std::map<BlockLink, LinkPath*> getLinks() const { return _links; }

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

    LinkConnexionRepresentation* startParam_;
    QPoint startMouse_;
    QPoint endMouse_;
    QRectF selectBox_;
    bool isSelecting_;
    bool creatingLink_;

    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);
  public:
    MainWidget(charliesoft::GraphOfProcess *model);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    charliesoft::GraphOfProcess *getModel() const {return _model;};

    public slots:
    void initLinkCreation(QPoint start);
    void endLinkCreation(QPoint end);

  };

}


#endif