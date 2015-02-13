#ifndef _WINDOWS_IMGRAPH_HEADER_
#define _WINDOWS_IMGRAPH_HEADER_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996 4244 4251 4275 4800 4503)
#endif
#include <QGraphicsView>
#include <QResizeEvent>
#include <QPainter>
#include <QRect>
#include <QString>
#include <QDialog>
#include <QStatusBar>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QMainWindow>
#include <QComboBox>
#include <QDial>
#include <QPainterPath>
#include <QTreeWidget>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

#include <map>

namespace charliesoft
{
  class GraphLayout;
  class MainWidget;
  class GlobalConfig;
  class VertexRepresentation;
  class GroupParamRepresentation;


  class DraggableContainer : public QTreeWidget
  {
  public:
    DraggableContainer() :QTreeWidget(){};
  protected:
    virtual void mousePressEvent(QMouseEvent *);
  };

  class Window : public QMainWindow
  {
    Q_OBJECT;

  protected:
    QVector<QAction*> vect_QActions;
    QToolBar* _toolbar;


    void createToolbar(QToolBar* toolBar);
  public:
    static Window* getInstance();
    static void releaseInstance();
    static void synchroMainGraph();

    void show();
    void addTab(MainWidget* tmp, QString tabName);

    void setStatusMessage(QString msg){
      statusBar()->showMessage(msg);
    };

    MainWidget* getMainWidget() const;
    std::string getKey(QTreeWidgetItem* w) {
      try{ return keysName_.at(w); }
      catch (std::out_of_range){ return ""; };
    };

    void redraw();

    bool event(QEvent *event);

    void showListAlgoDock(bool listOfAlgo)
    {
      if (_listOfDock != NULL && _listOfDock->count() > 1)
      {
        if (listOfAlgo)
          _listOfDock->setCurrentIndex(0);
        else
          _listOfDock->setCurrentIndex(1);
      }
    }
  private:
    static Window* ptr;

    Window();
    ~Window();

    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuAide;
    QTabWidget* _tabWidget;
    QTabBar* _listOfDock;

    QDockWidget * _dock;
    QDockWidget * _property_dock;
    DraggableContainer * _dock_content;
    std::vector<QTreeWidgetItem *> dock_categories;
    std::map<QTreeWidgetItem*, std::string> keysName_;

    void fillDock(int idDock);

    void closeEvent(QCloseEvent *event);

    private slots:
    void openFile();
    void newProject();
    void saveProject();
    void startGraph();
    void stopGraph();
    void switchPause();
    void saveAsProject();
    void createSubgraph();
    bool quitProg();
    void printHelp();

    void updatePropertyDock(GroupParamRepresentation*);

    void closeTab_(int index)
    {
      // Delete the page widget, which automatically removes
      // the tab as well.
      delete _tabWidget->widget(index);
    }

  };
}


#endif