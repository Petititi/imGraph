#ifndef _WINDOWS_IMGRAPH_HEADER_
#define _WINDOWS_IMGRAPH_HEADER_

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
#include "blocks/Block.h"

#include <map>

namespace charliesoft
{
  class GraphRepresentation;
  class MainWidget;
  class GlobalConfig;
  class Window : public QMainWindow
  {
    Q_OBJECT;

    static Window* ptr;

    charliesoft::GraphOfProcess *model_;
  public:
    static Window* getInstance();
    static void releaseInstance();
    static GraphRepresentation* getGraphLayout();
    void show();
    MainWidget* getMainWidget() const { return mainWidget_; }
  private:
    Window();
    ~Window();

    GlobalConfig* config_;

    GraphRepresentation* mainLayout_;
    QMenu *menuFichier;
    QMenu *menuAide;
    MainWidget* mainWidget_;//input, imgProcess, signalProcess, mathOperator, output

    std::vector<QDockWidget *> docks_;
    std::vector<QVBoxLayout *> docks_content_;

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