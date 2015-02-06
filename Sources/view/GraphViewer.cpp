
#include "GraphViewer.h"
#include "Window.h"


#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4250 4251 4275 4800)
#endif
#include <qtoolbutton.h>
#include <QtPrintSupport/qprinter.h>
#include <QtPrintSupport/QPrintDialog.h>

#include <qwt/qwt_counter.h>
#include <qwt/qwt_picker_machine.h>
#include <qwt/qwt_plot_zoomer.h>
#include <qwt/qwt_plot_panner.h>
#include <qwt/qwt_plot_renderer.h>
#include <qwt/qwt_text.h>
#include <qwt/qwt_math.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_legend.h>

#include <boost/thread/lock_guard.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace charliesoft;
using boost::lock_guard;

class Zoomer : public QwtPlotZoomer
{
public:
  Zoomer(int xAxis, int yAxis, QWidget *canvas) :
    QwtPlotZoomer(xAxis, yAxis, canvas)
  {
    setTrackerMode(QwtPicker::AlwaysOff);
    setRubberBand(QwtPicker::NoRubberBand);

    // RightButton: zoom out by 1
    // Ctrl+RightButton: zoom out to full size

    setMousePattern(QwtEventPattern::MouseSelect2,
      Qt::RightButton, Qt::ControlModifier);
    setMousePattern(QwtEventPattern::MouseSelect3,
      Qt::RightButton);
  }
};


GraphViewer::GraphViewer()
{
  setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Window);

  d_plot = new QwtPlot(this);

  const int margin = 5;
  d_plot->setContentsMargins(margin, margin, margin, 0);

  setContextMenuPolicy(Qt::NoContextMenu);

  d_zoomer[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft,
    d_plot->canvas());
  d_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
  d_zoomer[0]->setRubberBandPen(QColor(Qt::green));
  d_zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
  d_zoomer[0]->setTrackerPen(QColor(Qt::black));

  d_zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight,
    d_plot->canvas());

  d_panner = new QwtPlotPanner(d_plot->canvas());
  d_panner->setMouseButton(Qt::MidButton);

  d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
    QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
    d_plot->canvas());
  d_picker->setStateMachine(new QwtPickerDragPointMachine());
  d_picker->setRubberBandPen(QColor(Qt::green));
  d_picker->setRubberBand(QwtPicker::CrossRubberBand);
  d_picker->setTrackerPen(QColor(Qt::black));


  myGlobalLayout = new QVBoxLayout();
  myGlobalLayout->setObjectName(QString::fromUtf8("boxLayout"));
  myGlobalLayout->setContentsMargins(0, 0, 0, 0);
  myGlobalLayout->setSpacing(0);
  myGlobalLayout->setMargin(0);

  setLayout(myGlobalLayout);

  QToolBar *toolBar = new QToolBar(this);

  QToolButton *btnZoom = new QToolButton(toolBar);
  btnZoom->setText("Zoom");
  //btnZoom->setIcon(QPixmap(zoom_xpm));
  btnZoom->setCheckable(true);
  btnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnZoom);
  connect(btnZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));

#ifndef QT_NO_PRINTER
  QToolButton *btnPrint = new QToolButton(toolBar);
  btnPrint->setText("Print");
  //btnPrint->setIcon(QPixmap(print_xpm));
  btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnPrint);
  connect(btnPrint, SIGNAL(clicked()), SLOT(print()));
#endif

  QToolButton *btnExport = new QToolButton(toolBar);
  btnExport->setText("Export");
  //btnExport->setIcon(QPixmap(print_xpm));
  btnExport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnExport);
  connect(btnExport, SIGNAL(clicked()), SLOT(exportDocument()));

  toolBar->addSeparator();

  QWidget *hBox = new QWidget(toolBar);

  (void)toolBar->addWidget(hBox);

  myGlobalLayout->addWidget(toolBar);
  /*
#ifndef QT_NO_STATUSBAR
  (void)statusBar();
#endif
  */
  myGlobalLayout->addWidget(d_plot);

  enableZoomMode(true);
  showInfo();

  connect(d_picker, SIGNAL(moved(const QPoint &)),
    SLOT(moved(const QPoint &)));
  connect(d_picker, SIGNAL(selected(const QPolygon &)),
    SLOT(selected(const QPolygon &)));

  resize(600, 400);
  show();
}

void GraphViewer::updateCurve(int id, const double *xData, const double *yData, int size)
{
  boost::unique_lock<boost::mutex> lock(_mtx_synchro);
  QwtPlotCurve *curve;
  bool shouldAttach = false;
  if (_curves.size() <= static_cast<size_t>(id) || id<0)
  {
    shouldAttach = true;
    curve = new QwtPlotCurve();
    _curves.push_back(curve);
  }
  else
    curve = _curves[id];

  curve->setTitle("Some Points");
  Qt::GlobalColor color = Qt::blue;
  if (id == 1)
    color = Qt::green;
  if (id == 2)
    color = Qt::red;
  curve->setPen(color, 2);
  curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
  /*
  QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
    QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(4, 4));
  curve->setSymbol(symbol);*/

  curve->setSamples(xData, yData, size);

  if (shouldAttach)
    curve->attach(d_plot);
  else
  {
    //TODO: use emit...
    if (QApplication::instance()->thread() == QThread::currentThread())
    {
      QMetaObject::invokeMethod(d_plot,
        "replot",
        Qt::AutoConnection);
    }
    else
    {
      QMetaObject::invokeMethod(d_plot,
        "replot",
        Qt::BlockingQueuedConnection);
    }
  }
}

void GraphViewer::updateCurve(int id, const double *yData, int size)
{
  double* xData = new double[size];
  for (int i = 0; i < size; i++)
    xData[i] = i;
  updateCurve(id, xData, yData, size);
  delete xData;

}

#ifndef QT_NO_PRINTER

void GraphViewer::print()
{
  QPrinter printer(QPrinter::HighResolution);

  QString docName = d_plot->title().text();
  if (!docName.isEmpty())
  {
    docName.replace(QRegExp(QString::fromLatin1("\n")), tr(" -- "));
    printer.setDocName(docName);
  }

  printer.setCreator("Bode example");
  printer.setOrientation(QPrinter::Landscape);

  QPrintDialog dialog(&printer);
  if (dialog.exec())
  {
    QwtPlotRenderer renderer;

    if (printer.colorMode() == QPrinter::GrayScale)
    {
      renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
      renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
      renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame);
      renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
    }

    renderer.renderTo(d_plot, printer);
  }
}

#endif

void GraphViewer::exportDocument()
{
  QwtPlotRenderer renderer;
  renderer.exportTo(d_plot, "bode.pdf");
}

void GraphViewer::enableZoomMode(bool on)
{
  d_panner->setEnabled(on);

  d_zoomer[0]->setEnabled(on);
  d_zoomer[0]->zoom(0);

  d_zoomer[1]->setEnabled(on);
  d_zoomer[1]->zoom(0);

  d_picker->setEnabled(!on);

  showInfo();
}

void GraphViewer::showInfo(QString text)
{
  if (text == QString::null)
  {
    if (d_picker->rubberBand())
      text = "Cursor Pos: Press left mouse button in plot region";
    else
      text = "Zoom: Press mouse button and drag";
  }
  /*
#ifndef QT_NO_STATUSBAR
  statusBar()->showMessage(text);
#endif*/
}

void GraphViewer::moved(const QPoint &pos)
{
  QString info;
  info.sprintf("Freq=%g, Ampl=%g, Phase=%g",
    d_plot->invTransform(QwtPlot::xBottom, pos.x()),
    d_plot->invTransform(QwtPlot::yLeft, pos.y()),
    d_plot->invTransform(QwtPlot::yRight, pos.y())
    );
  showInfo(info);
}

void GraphViewer::selected(const QPolygon &)
{
  showInfo();
}
