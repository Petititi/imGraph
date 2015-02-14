#ifndef _VERTEXREPRESENTATION_HEADER_
#define _VERTEXREPRESENTATION_HEADER_

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
  class GroupParamRepresentation;
  class MainVertexBlock : public QWidget
  {
    Q_OBJECT;
  public:
    enum MouseState
    {
      MouseOut,
      MouseHorResize,
      MouseVerResize,
      MouseDiagResize,
      MouseDragOpen,
      MouseDragClose,
      MouseIn
    };
  public:
    MainVertexBlock(GroupParamRepresentation* parent);

    MouseState getState() const { return _state; };

  protected:
    void updateMouseState(const QPoint &pos);

    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);

    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    GroupParamRepresentation* _parent;
    MouseState _state;
  };


  class GroupParamRepresentation :public QWidget
  {
    Q_OBJECT;
  protected:
    MainVertexBlock* _blockRepresentation;
    QWidget* _conditionsRepresentation;
    QFrame* _lineTitle;
    QLabel* _vertexTitle;
    QLabel* _conditionTitle;
    QLabel* _conditionsValues;

    bool _isMoving;
    bool _hasDynamicParams;
    QPoint startClick_;
    LinkConnexionRepresentation* _paramActiv;
    int heightOfConditions;

    QPoint _sizeIncrement;

    std::map<BlockLink, LinkPath*> _links;
    std::vector< std::pair<ConditionOfRendering*, ConditionLinkRepresentation*> > _linksConditions;
    std::vector<LinkConnexionRepresentation*> _listOfInputChilds;
    std::map<std::string, LinkConnexionRepresentation*> _listOfInputSubParams;
    std::vector<LinkConnexionRepresentation*> _listOfOutputChilds;
    std::vector<LinkConnexionRepresentation*> _listOfSubParams;
    std::vector<ConditionOfRendering> _conditions;

    static std::vector<GroupParamRepresentation*> _selectedBlock;
  public:
    GroupParamRepresentation(std::string title);
    ~GroupParamRepresentation();

    bool hasDynamicParams() const { return _hasDynamicParams; };

    virtual LinkConnexionRepresentation* addNewInputParam(ParamDefinition def);
    virtual LinkConnexionRepresentation* addNewOutputParam(ParamDefinition def);

    void setParamActiv(LinkConnexionRepresentation*);
    bool isDragging() const {
      return _blockRepresentation->getState() == MainVertexBlock::MouseDragOpen ||
        _blockRepresentation->getState() == MainVertexBlock::MouseDragClose;
    };

    std::map<BlockLink, LinkPath*> getLinks() const { return _links; }

    void addLink(BlockLink l, LinkPath* p){
      _links[l] = p;
    };
    void removeLink(BlockLink l);

    void changeStyleProperty(const char* propertyName, QVariant val);
    void setSelected(bool isSelected);

    static void resetSelection();
    static std::vector<GroupParamRepresentation*> getSelection(){
      return _selectedBlock;
    };

    LinkConnexionRepresentation* getParamRep(std::string paramName, bool input);
    std::vector<LinkConnexionRepresentation*>& getListOfInputChilds() { return _listOfInputChilds; }
    std::map<std::string, LinkConnexionRepresentation*>& getListOfSubParams() { return _listOfInputSubParams; }
    std::vector<LinkConnexionRepresentation*>& getListOfOutputChilds() { return _listOfOutputChilds; }
  protected:
    ConditionLinkRepresentation* getCondition(ConditionOfRendering*, bool isLeft);

    virtual void moveDelta(QPoint delta);
    virtual void updatePosition(){};
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    public slots:
    virtual void reshape();

  signals:
    void updateProp(GroupParamRepresentation*);
  };

  class VertexRepresentation :public GroupParamRepresentation
  {
    Q_OBJECT;
    Block* _model;

  public:
    VertexRepresentation(Block* model);
    ~VertexRepresentation();

    Block* getModel() const { return _model; }


    virtual LinkConnexionRepresentation* addNewInputParam(ParamDefinition def);
    virtual LinkConnexionRepresentation* addNewOutputParam(ParamDefinition def);

  protected:
    virtual void enterEvent(QEvent *);
    void createListParamsFromModel();
    virtual void updatePosition();
    virtual void moveDelta(QPoint delta);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    public slots:
    virtual void reshape();
  };

}


#endif