#ifndef _GRAPHICVIEW_HEADER_
#define _GRAPHICVIEW_HEADER_

#include <QGraphicsView>
#include <QResizeEvent>
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
#include <QPainterPath>
#include "Block.h"

#include <map>

namespace charliesoft
{
  class GlobalConfig
  {
  public:
    void loadConfig();
    void saveConfig();

    std::string lastProject_;
    std::string prefLang_;
    bool isMaximized;
    QRect lastPosition;
  };


  class ParamRepresentation :public QLabel
  {
    Q_OBJECT

    Block* model_;
    std::string paramName_;
    bool isInput_;
  public:
    ParamRepresentation(Block* model, std::string paramName, bool isInput, QWidget *father):
      QLabel(paramName.c_str(), father), model_(model), paramName_(paramName), isInput_(isInput){};

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    std::string getParamName() const { return paramName_; }
    Block* getModel() const { return model_; }
    bool isInput() const { return isInput_; }

    QPoint getWorldAnchor();
  signals:
    void creationLink(QPoint startPos);
    void releaseLink(QPoint endPos);
  };

  class NodeRepresentation :public QWidget
  {
    Block* model_;
    bool isDragging_;
    QPoint deltaClick_;
    ParamRepresentation* paramActiv_;

    std::map<std::string, ParamRepresentation*> listOfInputChilds_;
    std::map<std::string, ParamRepresentation*> listOfOutputChilds_;

    std::map<BlockLink, QPainterPath*> links_;
    std::map<NodeRepresentation*, BlockLink> back_links_;
  public:
    NodeRepresentation(Block* model);

    Block* getModel() const { return model_; }
    void setParamActiv(ParamRepresentation*);

    void setLink(const BlockLink& linkInfo);
    void paintLinks(QPainter& p);

  protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    void notifyBackLink(const BlockLink& linkInfo, NodeRepresentation* otherNode)
    {
      back_links_[otherNode] = linkInfo;
    };
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

    void drawLinks(QPainter& p);
    NodeRepresentation* getNodeRepresentation(Block* b);

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

  class Fenetre : public QMainWindow
  {
    Q_OBJECT;

    static Fenetre* ptr;

    charliesoft::GraphOfProcess *model_;
  public:
    static Fenetre* getInstance();
    static void releaseInstance();
    static GraphRepresentation* getGraphLayout();
    void show();
    MainWidget* getMainWidget() const { return mainWidget_; }
  private:
    Fenetre();
    ~Fenetre();

    GlobalConfig* config_;

    GraphRepresentation* mainLayout_;
    QMenu *menuFichier;
    QMenu *menuAide;
    MainWidget* mainWidget_;

    void mousePressEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

  signals:
    void askSynchro(charliesoft::GraphOfProcess *model);

    private slots:
    void openFile();
    void newProject();
    bool quitProg();
    void printHelp();
  };
}


#endif