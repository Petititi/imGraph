#ifndef _GRAPHICVIEW_HEADER_
#define _GRAPHICVIEW_HEADER_

#ifdef _WIN32
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

namespace charliesoft
{
  class GlobalConfig
  {
  public:
    void loadConfig();
    void saveConfig();

    std::string lastProject_;
    std::string prefLang_;
    std::string styleSheet_;
    bool isMaximized;
    QRect lastPosition;
  };

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

  class ParamRepresentation :public QLabel
  {
    Q_OBJECT;

    Block* model_;
    ParamDefinition param_;
    bool isInput_;
  public:
    ParamRepresentation(Block* model, ParamDefinition param, bool isInput, QWidget *father);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    bool shouldShow() const { return param_.show_; }
    void setVisibility(bool visible);
    std::string getParamName() const { return param_.name_; }
    ParamValue* getParamValue() const { return model_->getParam(param_.name_, isInput_); }
    std::string getParamHelper() const { return param_.helper_; }
    Block* getModel() const { return model_; }
    bool isInput() const { return isInput_; }

    QPoint getWorldAnchor();
  signals:
    void creationLink(QPoint startPos);
    void releaseLink(QPoint endPos);
  };

  class VertexRepresentation :public QWidget
  {
    QFrame* lineTitle;
    QLabel* vertexTitle_;
    Block* model_;
    bool isDragging_;
    QPoint deltaClick_;
    ParamRepresentation* paramActiv_;

    std::map<BlockLink, LinkPath*> links_;
    std::map<std::string, ParamRepresentation*> listOfInputChilds_;
    std::map<std::string, ParamRepresentation*> listOfOutputChilds_;
  public:
    VertexRepresentation(Block* model);
    ~VertexRepresentation();

    Block* getModel() const { return model_; }
    void setParamActiv(ParamRepresentation*);

    std::map<BlockLink, LinkPath*> getLinks() const { return links_; }

    void addLink(BlockLink l, LinkPath* p){
      links_[l] = p;
    };
    void removeLink(BlockLink l);

    void reshape();

    void changeStyleProperty(const char* propertyName, QVariant val);
    void setSelected(bool isSelected);
    static void resetSelection();
    static std::vector<VertexRepresentation*> getSelection(){
      return selectedBlock_;
    };
    ParamRepresentation* getParamRep(std::string paramName, bool input);
  protected:
    static std::vector<VertexRepresentation*> selectedBlock_;
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
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
    VertexRepresentation* vertex_;

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

    public slots:
    void switchEnable(int);
    void openFile();
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
    GraphRepresentation();

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

    ParamRepresentation* startParam_;
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

    charliesoft::GraphOfProcess *model_;
  public:
    MainWidget(charliesoft::GraphOfProcess *model);

    void setModel(charliesoft::GraphOfProcess * val) { model_ = val; }


    signals:
    void askSynchro(charliesoft::GraphOfProcess *model);

    public slots:
    void initLinkCreation(QPoint start);
    void endLinkCreation(QPoint end);

  };

}


#endif