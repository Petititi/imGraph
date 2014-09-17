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

#include "blocks/Block.h"

#include <map>
#include "Window.h"
#include "Configuration.h"

namespace charliesoft
{

  class LinkPath : public QPainterPath
  {
    bool selected_;
  public:
    LinkPath() :QPainterPath(){ selected_ = false; };

    bool intersect(const QRect& pos) const;

    void draw(QPainter& p)
    {
      if (selected_)
        p.setPen(QPen(QColor(255, 0, 0), 2));
      else
        p.setPen(QPen(Qt::black, 2));
      p.drawPath(*this);
    }

    bool isSelected() const { return selected_; }
    void setSelected(bool val) { selected_ = val; }
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
    std::string getParamHelper() const { return param_._helper; }
    Block* getModel() const { return _model; }

  };

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
    QPoint startClick_;
    LinkConnexionRepresentation* _paramActiv;
    int heightOfConditions;

    std::map<BlockLink, LinkPath*> links_;
    std::vector< std::pair<ConditionOfRendering*, ConditionLinkRepresentation*> > linksConditions_;
    std::map<std::string, ParamRepresentation*> listOfInputChilds_;
    std::map<std::string, ParamRepresentation*> listOfOutputChilds_;
  public:
    VertexRepresentation(Block* model);
    ~VertexRepresentation();

    Block* getModel() const { return _model; }
    void setParamActiv(LinkConnexionRepresentation*);

    std::map<BlockLink, LinkPath*> getLinks() const { return links_; }

    void addLink(BlockLink l, LinkPath* p){
      links_[l] = p;
    };
    void removeLink(BlockLink l);

    void changeStyleProperty(const char* propertyName, QVariant val);
    void setSelected(bool isSelected);
    static void resetSelection();
    static std::vector<VertexRepresentation*> getSelection(){
      return selectedBlock_;
    };
    ParamRepresentation* getParamRep(std::string paramName, bool input);
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
  };

  class ConditionConfigurator :public QDialog
  {
    Q_OBJECT;
    VertexRepresentation* _vertex;

    QPushButton* OKbutton_;
    QPushButton* Cancelbutton_;
    QComboBox* _condition_left;
    QComboBox* _condition_type;
    QComboBox* _condition_right;

    QLineEdit* _value_left;
    QLineEdit* _value_right;

    QGridLayout* comboBoxLayout;
  public:
    ConditionConfigurator(VertexRepresentation* vertex);
    public slots:
    void accept_button();
    void reject_button();
    void updateLeft(int);
    void updateRight(int);

  signals:
    void askSynchro();
  };

  class ParamsConfigurator :public QDialog
  {
    Q_OBJECT;

    typedef boost::bimap<QCheckBox*, ParamRepresentation* >::value_type Modif_map_type;
    typedef boost::bimap<ParamRepresentation*, QObject* >::value_type Val_map_type;

    std::map<QObject*, QLineEdit*> openFiles_;
    std::map<QGroupBox*, ParamRepresentation*> inputGroup_;
    std::map<QGroupBox*, ParamRepresentation*> outputGroup_;
    boost::bimap< QCheckBox*, ParamRepresentation* > inputModificator_;
    boost::bimap<ParamRepresentation*, QObject* > inputValue_;
    std::map<std::string, ParamRepresentation*>& in_param_;
    std::map<std::string, ParamRepresentation*>& out_param_;
    VertexRepresentation* _vertex;

    QPushButton* OKbutton_;
    QPushButton* Cancelbutton_;
    QTabWidget* tabWidget_;
    std::vector<QVBoxLayout *> tabs_content_;

    void addParamOut(ParamRepresentation  *p);
    void addParamIn(ParamRepresentation  *p);
  public:
    ParamsConfigurator(VertexRepresentation* vertex,
      std::map<std::string, ParamRepresentation*>& in_param,
      std::map<std::string, ParamRepresentation*>& out_param);

  signals:
    void changeVisibility(bool isVisible);
    void askSynchro();

    public slots:
    void switchEnable(int);
    void openFile();
    void configCondition();
    void accept_button();
    void reject_button();
  };

  class GraphRepresentation :public QLayout
  {
    Q_OBJECT;

    std::map<Block*, QLayoutItem*> items_;
    std::map<BlockLink, LinkPath*> links_;
    std::vector<Block*> orderedBlocks_;
  public:

    void removeLinks(VertexRepresentation* vertex);
    void removeSelectedLinks();
    void addLink(const BlockLink& link);
    void updateLink(const BlockLink& link);

    void clearLayout(QLayout* layout = NULL);
    std::map<Block*, QLayoutItem*> getItems() const { return items_; }
    std::map<BlockLink, LinkPath*> getLinks() const { return links_; }

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
    void synchronize(charliesoft::GraphOfProcess *model);
  };

  class MainWidget :public QWidget
  {
    Q_OBJECT

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

    charliesoft::GraphOfProcess *_model;
  public:
    MainWidget(charliesoft::GraphOfProcess *model);

    void setModel(charliesoft::GraphOfProcess * val) { _model = val; }


    signals:
    void askSynchro(charliesoft::GraphOfProcess *model);

    public slots:
    void initLinkCreation(QPoint start);
    void endLinkCreation(QPoint end);

  };

}


#endif