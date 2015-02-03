
#ifndef __GRAPHVIEWER_H__
#define __GRAPHVIEWER_H__

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc.hpp"

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QGraphicsView>
#include <QSizePolicy>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QSettings>
#include <qtimer.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QWaitCondition>
#include <QKeyEvent>
#include <QMetaObject>
#include <QPointer>
#include <QSlider>
#include <QLabel>
#include <QIODevice>
#include <QShortcut>
#include <QStatusBar>
#include <QVarLengthArray>
#include <QFileInfo>
#include <QDate>
#include <QFileDialog>
#include <QToolBar>
#include <QAction>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QDialog>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <qwt/qwt_plot.h>

#include <vector>

#ifdef _WIN32
#pragma warning(pop)
#endif

class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtPlotCurve;
class QwtPlot;
class QPoint;
class QPolygon;

class GraphViewer : public QWidget
{
  Q_OBJECT

    boost::mutex _mtx_synchro;
public:
  GraphViewer();


  void updateCurve(int id, const double *xData, const double *yData, int size);
  void updateCurve(int id, const double *xData, int size);

  private Q_SLOTS:
  void moved(const QPoint &);
  void selected(const QPolygon &);

#ifndef QT_NO_PRINTER
  void print();
#endif

  void exportDocument();
  void enableZoomMode(bool);
private:
  void showInfo(QString text = QString::null);

  QVBoxLayout* myGlobalLayout;

  std::vector<QwtPlotCurve *> _curves;

  QwtPlot *d_plot;

  QwtPlotZoomer *d_zoomer[2];
  QwtPlotPicker *d_picker;
  QwtPlotPanner *d_panner;
};

#endif
