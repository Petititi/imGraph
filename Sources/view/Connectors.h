#ifndef _CONNECTORS_HEADER_
#define _CONNECTORS_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <QPainterPath>
#include <QLabel>
#include <QPainter>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

namespace charliesoft
{
  class LinkPath : public QPainterPath
  {
    bool _selected;
  public:
    LinkPath() :QPainterPath(){ _selected = false; };

    bool intersect(const QRect& pos) const;

    void draw(QPainter& p)
    {
      if (_selected)
        p.setPen(QPen(QColor(255, 0, 0), 2));
      else
        p.setPen(QPen(Qt::black, 2));
      p.drawPath(*this);
    }

    bool isSelected() const { return _selected; }
    void setSelected(bool val) { _selected = val; }
  };

  class LinkConnexionRepresentation :public QLabel
  {
    Q_OBJECT;
  protected:
    bool _isInput;
  public:
    LinkConnexionRepresentation(std::string text, bool isInput, QWidget *father);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    bool isInput() const { return _isInput; }

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

  class ParamRepresentation :public LinkConnexionRepresentation
  {
    Q_OBJECT;

    Block* _model;
    ParamDefinition param_;

  public:
    ParamRepresentation(Block* model, ParamDefinition param, bool isInput, QWidget *father);

    bool shouldShow() const { return param_._show; }
    void setVisibility(bool visible);
    std::string getParamName() const { return param_._name; }
    ParamValue* getParamValue() const { return _model->getParam(param_._name, _isInput); }
    std::string getParamHelper() const;
    std::vector<std::string> getParamListChoice() const;
    Block* getModel() const { return _model; };
  };

}


#endif