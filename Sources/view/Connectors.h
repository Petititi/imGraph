#ifndef _CONNECTORS_HEADER_
#define _CONNECTORS_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <QGraphicsLineItem>
#include <QLabel>
#include <QPainter>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

namespace charliesoft
{
  class VertexRepresentation;
  class LinkConnexionRepresentation;
  class SubBlock;

  class LinkPath : public QGraphicsLineItem
  {
    bool _selected;
    QPolygonF _arrowHead;
    LinkConnexionRepresentation* _src;
    LinkConnexionRepresentation* _dst;
  public:
    LinkPath(LinkConnexionRepresentation* src,
      LinkConnexionRepresentation* dst,
      QGraphicsItem *parent=0, QGraphicsScene *scene=0);

    bool intersect(const QRect& pos) const;
    QPainterPath shape() const;

    void draw(QPainter *painter, const QStyleOptionGraphicsItem *,
      QWidget *);

    bool isSelected() const { return _selected; }
    void setSelected(bool val) { _selected = val; }
  };

  class LinkConnexionRepresentation :public QLabel
  {
    Q_OBJECT;
  protected:
    bool _isInput;
    bool _shouldShow;
    VertexRepresentation* _vertex;
  public:
    LinkConnexionRepresentation(std::string text, bool isInput, QWidget *father);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    bool isInput() const { return _isInput; }

    virtual bool shouldShow() const { return _shouldShow; }
    virtual void setVisibility(bool visible);

    QPoint getWorldAnchor();
  signals:
    void creationLink(QPoint startPos);
    void releaseLink(QPoint endPos);
    void askSynchro();
  };

  class ConditionLinkRepresentation :public LinkConnexionRepresentation
  {
    Q_OBJECT;

    ConditionOfRendering* _model;
    bool _isLeftCond;
  public:
    ConditionLinkRepresentation(ConditionOfRendering* model, bool isLeftCond, QWidget *father);
    bool isLeftCond() const { return _isLeftCond; }
    ConditionOfRendering* getModel() const { return _model; }
  signals:
    void askSynchro();
  };

  class SubGraphParamRepresentation :public LinkConnexionRepresentation
  {
    Q_OBJECT;

    SubBlock* _model;
    ParamDefinition _param;
    bool _isLeftCond;
  public:
    SubGraphParamRepresentation(SubBlock* model, const ParamDefinition& def, bool isInput, QWidget *father);

    const ParamDefinition& getDefinition() const{ return _param; };
    SubBlock* getModel() const { return _model; }
  };

  class ParamRepresentation :public LinkConnexionRepresentation
  {
    Q_OBJECT;

    Block* _model;
    ParamDefinition _param;
    bool _isSubParam;
    bool _defaultValue;
  public:
    ParamRepresentation(Block* model, ParamDefinition param, bool isInput, QWidget *father);

    virtual bool shouldShow() const { return _param._show; }
    virtual void setVisibility(bool visible);

    bool isDefaultVal() const { return _defaultValue; }
    void useDefault(bool defaultVal){ _defaultValue = defaultVal; };
    std::string getParamName() const { return _param._name; }
    ParamValue* getParamValue() const { 
      if (_model != NULL)return _model->getParam(_param._name, _isInput); else return NULL;
    }
    std::string getParamHelper() const;
    std::vector<std::string> getParamListChoice() const;
    Block* getModel() const { return _model; };
    void isSubParam(bool param1);
  };

}


#endif