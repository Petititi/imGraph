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
#include <QTreeWidget>

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
  class VertexRepresentation;


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

  public:
    static Window* getInstance();
    static void releaseInstance();
    static void synchroMainGraph();

    void show();
    charliesoft::MainWidget* getMainWidget() const;
    std::string getKey(QTreeWidgetItem* w) {
      try{ return keysName_.at(w); }
      catch (std::out_of_range){ return ""; };
    };

    void redraw();

    bool event(QEvent *event);
  private:
    void addTab(MainWidget* tmp, QString tabName);
    static Window* ptr;

    Window();
    ~Window();

    QMenu *menuFichier;
    QMenu *menuAide;
    QTabWidget* _tabWidget;

    QDockWidget * dock_;
    QDockWidget * property_dock_;
    DraggableContainer * _dock_content;
    std::vector<QTreeWidgetItem *> dock_categories;
    std::map<QTreeWidgetItem*, std::string> keysName_;

    void fillDock(int idDock);

    void mousePressEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

    private slots:
    void openFile();
    void newProject();
    void saveProject();
    void saveAsProject();
    bool quitProg();
    void printHelp();

    void updatePropertyDock(VertexRepresentation*);

    void closeTab_(int index)
    {
      // Delete the page widget, which automatically removes
      // the tab as well.
      delete _tabWidget->widget(index);
    }

  };
}


#endif