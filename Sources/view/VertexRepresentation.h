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

}


#endif