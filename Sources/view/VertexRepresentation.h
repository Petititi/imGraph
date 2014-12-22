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
  class GroupParamRepresentation :public QWidget
  {
    Q_OBJECT;
  protected:
    QWidget* _blockRepresentation;
    QWidget* _conditionsRepresentation;
    QFrame* _lineTitle;
    QLabel* _vertexTitle;
    QLabel* _conditionTitle;
    QLabel* _conditionsValues;

    bool _isDragging;
    bool _isMoving;
    bool _hasDynamicParams;
    QPoint startClick_;
    LinkConnexionRepresentation* _paramActiv;
    int heightOfConditions;

    std::map<BlockLink, LinkPath*> _links;
    std::vector< std::pair<ConditionOfRendering*, ConditionLinkRepresentation*> > _linksConditions;
    std::map<std::string, LinkConnexionRepresentation*> _listOfInputChilds;
    std::map<std::string, LinkConnexionRepresentation*> _listOfInputSubParams;
    std::map<std::string, LinkConnexionRepresentation*> _listOfOutputChilds;
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
    std::map<std::string, LinkConnexionRepresentation*>& getListOfInputChilds() { return _listOfInputChilds; }
    std::map<std::string, LinkConnexionRepresentation*>& getListOfSubParams() { return _listOfInputSubParams; }
    std::map<std::string, LinkConnexionRepresentation*>& getListOfOutputChilds() { return _listOfOutputChilds; }
  protected:
    ConditionLinkRepresentation* getCondition(ConditionOfRendering*, bool isLeft);

    virtual void moveDelta(QPoint delta);
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
    void createListParamsFromModel();
    virtual void moveDelta(QPoint delta);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    public slots:
    virtual void reshape();
  };

}


#endif