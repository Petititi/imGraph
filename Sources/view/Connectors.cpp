
#include "Connectors.h"

#include <boost/algorithm/string.hpp>

#include "Window.h"
#include "GraphicView.h"

namespace charliesoft
{
  bool lineIntersect(QLineF& line1, double x1, double y1, double x2, double y2)
  {
    QLineF line;
    line.setP1(QPointF(x1, y1));
    line.setP2(QPointF(x2, y2));
    QPointF intersection;
    return line.intersect(line1, &intersection) == QLineF::BoundedIntersection;
  }

  bool LinkPath::intersect(const QRect& rect) const
  {
    QLineF line;
    const QPainterPath::Element &p1 = elementAt(0);
    const QPainterPath::Element &p2 = elementAt(1);
    if (rect.contains((int)p1.x, (int)p1.y) || rect.contains((int)p2.x, (int)p2.y))
      return true;
    line.setP1(QPointF(p1.x, p1.y));
    line.setP2(QPointF(p2.x, p2.y));
    return lineIntersect(line, rect.x(), rect.y(), rect.x() + rect.width(), rect.y()) ||
      lineIntersect(line, rect.x(), rect.y(), rect.x(), rect.y() + rect.height()) ||
      lineIntersect(line, rect.x(), rect.y() + rect.height(), rect.x() + rect.width(), rect.y() + rect.height());
  }

  ConditionLinkRepresentation::ConditionLinkRepresentation(ConditionOfRendering* model, bool isLeftCond, QWidget *father) :
    LinkConnexionRepresentation(_STR(isLeftCond ? "CONDITION_BLOCK_LEFT" : "CONDITION_BLOCK_RIGHT"), isLeftCond, father), _model(model){
    setObjectName("ParamRepresentation");
    setToolTip(_QT("CONDITION_BLOCK_HELP"));

    connect(this, SIGNAL(askSynchro()), (VertexRepresentation*)father->parentWidget(), SLOT(reshape()));
  }

  ParamRepresentation::ParamRepresentation(Block* model, ParamDefinition param, bool isInput, QWidget *father) :
    LinkConnexionRepresentation(_STR(param._name), isInput, father), _model(model), param_(param){
    setObjectName("ParamRepresentation");
    if (!param._show) this->hide();
    setToolTip(_QT(param._helper));
    VertexRepresentation* parent = (VertexRepresentation*)father->parentWidget();
    //connect(this, SIGNAL(askSynchro()), parent, SLOT(reshape()));
  };

  std::string ParamRepresentation::getParamHelper() const {
    size_t pos = _STR(param_._helper).find_first_of('|');
    if (pos == std::string::npos)
      return param_._helper;
    else
      return _STR(param_._helper).substr(0, pos);
  }

  std::vector<std::string> ParamRepresentation::getParamListChoice() const
  {
    std::vector<std::string> out;
    size_t pos = _STR(param_._helper).find_first_of('|');
    if (pos == std::string::npos)
      return out;
    std::string params = _STR(param_._helper).substr(pos+1);
    boost::split(out, params, boost::is_any_of("^"));
    return out;
  }

  void ParamRepresentation::setVisibility(bool visible)
  {
    if (param_._show == visible)
      return;//Nothing to do...

    param_._show = visible;
    emit askSynchro();
  }

  LinkConnexionRepresentation::LinkConnexionRepresentation(std::string text, bool isInput, QWidget *father) :
    QLabel(text.c_str(), father), _isInput(isInput)
  {

  };
  QPoint LinkConnexionRepresentation::getWorldAnchor()
  {
    QPoint p = parentWidget()->mapTo(Window::getInstance()->getMainWidget(), mapTo(parentWidget(), pos()) / 2);
    if (_isInput)
      return QPoint(p.x(), (int)(p.y() + height() / 2.));
    else
      return QPoint(p.x() + width(), (int)(p.y() + height() / 2.));
  }

  void LinkConnexionRepresentation::mousePressEvent(QMouseEvent *e)
  {
    QWidget* parent = parentWidget();
    if (parent == NULL) return;//Nothing to do...
    if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(parent->parentWidget()))
      vertex->setParamActiv(this);
    //get the position of widget inside main widget and send signal of a new link creation:
    emit creationLink(getWorldAnchor());
    e->ignore();
  }
  void LinkConnexionRepresentation::mouseReleaseEvent(QMouseEvent *e)
  {
    if (VertexRepresentation* vertex = dynamic_cast<VertexRepresentation*>(parentWidget()))
      vertex->setParamActiv(NULL);

    emit releaseLink(Window::getInstance()->getMainWidget()->mapFromGlobal(e->globalPos()));
  };
  void LinkConnexionRepresentation::mouseDoubleClickEvent(QMouseEvent *)
  {

  };
  void LinkConnexionRepresentation::mouseMoveEvent(QMouseEvent *me)
  {
    me->ignore();
  };
}