#ifndef _GRAPHICVIEW_HEADER_
#define _GRAPHICVIEW_HEADER_

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
    void setVisibility(bool visible) { param_.show_ = visible; }
    std::string getParamName() const { return param_.name_; }
    ParamValue* getParamValue() const { return model_->getParam(param_.name_); }
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

    std::map<std::string, ParamRepresentation*> listOfInputChilds_;
    std::map<std::string, ParamRepresentation*> listOfOutputChilds_;

    std::map<BlockLink, QPainterPath*> links_;
    std::vector< std::pair<VertexRepresentation*, BlockLink> > back_links_;
  public:
    VertexRepresentation(Block* model);

    Block* getModel() const { return model_; }
    void setParamActiv(ParamRepresentation*);

    void setEdge(const BlockLink& linkInfo);
    void paintLinks(QPainter& p);

    void reshape();

  protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    void notifyBackLink(const BlockLink& linkInfo, VertexRepresentation* otherVertex)
    {
      back_links_.push_back(std::pair<VertexRepresentation*, BlockLink>(otherVertex, linkInfo));
    };
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
    std::vector<Block*> orderedBlocks_;
  public:
    GraphRepresentation();

    void clearLayout(QLayout* layout=NULL);

    virtual void addItem(QLayoutItem *);
    virtual QLayoutItem * itemAt(int index) const;
    virtual QLayoutItem * takeAt(int index);
    virtual int indexOf(QWidget *) const;
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
    bool creatingLink_;

    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

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