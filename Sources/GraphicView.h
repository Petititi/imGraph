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
    Block* model_;
    std::string paramName_;
  public:
    ParamRepresentation(Block* model, std::string paramName, QWidget *father):
      QLabel(paramName.c_str(), father), model_(model), paramName_(paramName){};

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
  };

  class NodeRepresentation :public QWidget
  {
    Block* model_;
    bool isDragging;
    QPoint deltaClick;
  public:
    NodeRepresentation(Block* model);

    Block* getModel() const { return model_; }

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

  };

  class GraphRepresentation :public QLayout
  {
    std::vector<QLayoutItem*> items_;
  public:
    GraphRepresentation();

    void synchronize(charliesoft::GraphOfProcess *model);

    void clearLayout(QLayout* layout=NULL);

    virtual void addItem(QLayoutItem *);
    virtual QLayoutItem * itemAt(int index) const;
    virtual QLayoutItem * takeAt(int index);
    virtual int indexOf(QWidget *) const;
    virtual int count() const;
    virtual QSize sizeHint() const;
  };

  class MainWidget :public QWidget
  {
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    charliesoft::GraphOfProcess *model_;
  public:
    MainWidget(charliesoft::GraphOfProcess *model);

    void setModel(charliesoft::GraphOfProcess * val) { model_ = val; }
  };

  class Fenetre : public QMainWindow
  {
    Q_OBJECT;

    static Fenetre* ptr;

    charliesoft::GraphOfProcess *model_;
  public:
    static Fenetre* getInstance();
    static void releaseInstance();
    void show();
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

    private slots:
    void openFile();
    void newProject();
    bool quitProg();
    void printHelp();
  };
}


#endif