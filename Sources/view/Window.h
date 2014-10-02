#ifndef _WINDOWS_IMGRAPH_HEADER_
#define _WINDOWS_IMGRAPH_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
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

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

#include <map>

namespace charliesoft
{
  class GraphRepresentation;
  class MainWidget;
  class GlobalConfig;

  class DraggableWidget : public QLabel
  {
  public:
    DraggableWidget(QString text, QWidget* p);
  };

  class DraggableContainer : public QWidget
  {
  public:
    DraggableContainer(QWidget* p) :QWidget(p){};
  protected:
    virtual void mousePressEvent(QMouseEvent *);
  };

  class Window : public QMainWindow
  {
    Q_OBJECT;

    static Window* ptr;

    charliesoft::GraphOfProcess *_model;
  public:
    static Window* getInstance();
    static void releaseInstance();
    charliesoft::GraphOfProcess * getModel() const { return _model; }
    static GraphRepresentation* getGraphLayout();
    void show();
    MainWidget* getMainWidget() const { return mainWidget_; }
    std::string getKey(QWidget* w){ return keysName_[w]; };

    void redraw();

    bool event(QEvent *event);
  private:
    Window();
    ~Window();

    GraphRepresentation* mainLayout_;
    QMenu *menuFichier;
    QMenu *menuAide;
    MainWidget* mainWidget_;//input, imgProcess, videoProcess, mathOperator, output

    QTabWidget* tabWidget_;
    QDockWidget * dock_;
    std::vector<QVBoxLayout *> docks_content_;
    std::map<QWidget*, std::string> keysName_;

    void fillDock(int idDock);

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