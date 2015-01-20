
#include "GraphViewer.h"
#include "Window.h"


#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
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

#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace charliesoft;

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



GraphViewer::GraphViewer() :QwtPlot(NULL)
{
  setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Window);
  setCanvasBackground(Qt::white);
  setAxisScale(QwtPlot::yLeft, 0.0, 10.0);
  insertLegend(new QwtLegend());


  /*
  d_zoomer[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, this);
  d_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
  d_zoomer[0]->setRubberBandPen(QColor(Qt::green));
  d_zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
  d_zoomer[0]->setTrackerPen(QColor(Qt::white));

  d_zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, this);

  d_panner = new QwtPlotPanner(this);
  d_panner->setMouseButton(Qt::MidButton);

  d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
    QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, this);
  d_picker->setStateMachine(new QwtPickerDragPointMachine());
  d_picker->setRubberBandPen(QColor(Qt::green));
  d_picker->setRubberBand(QwtPicker::CrossRubberBand);
  d_picker->setTrackerPen(QColor(Qt::white));

  QToolBar *toolBar = new QToolBar(this);

  QToolButton *btnZoom = new QToolButton(toolBar);
  btnZoom->setText("Zoom");
  btnZoom->setCheckable(true);
  btnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnZoom);
  connect(btnZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));

#ifndef QT_NO_PRINTER
  QToolButton *btnPrint = new QToolButton(toolBar);
  btnPrint->setText("Print");
  btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnPrint);
  connect(btnPrint, SIGNAL(clicked()), SLOT(print()));
#endif

  QToolButton *btnExport = new QToolButton(toolBar);
  btnExport->setText("Export");
  btnExport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolBar->addWidget(btnExport);
  connect(btnExport, SIGNAL(clicked()), SLOT(exportDocument()));

  toolBar->addSeparator();

  QWidget *hBox = new QWidget(toolBar);

  QHBoxLayout *layout = new QHBoxLayout(hBox);
  layout->setSpacing(0);
  layout->addWidget(new QWidget(hBox), 10); // spacer
  layout->addWidget(new QLabel("Damping Factor", hBox), 0);
  layout->addSpacing(10);

  QwtCounter *cntDamp = new QwtCounter(hBox);
  cntDamp->setRange(0.0, 5.0);
  cntDamp->setSingleStep(0.01);
  cntDamp->setValue(0.0);

  layout->addWidget(cntDamp, 0);

  (void)toolBar->addWidget(hBox);
  
  addWidget(toolBar);
#ifndef QT_NO_STATUSBAR
  (void)statusBar();
#endif

  enableZoomMode(true);
  showInfo();


  connect(d_picker, SIGNAL(moved(const QPoint &)),
    SLOT(moved(const QPoint &)));
  connect(d_picker, SIGNAL(selected(const QPolygon &)),
    SLOT(selected(const QPolygon &)));
    */






  QwtPlotGrid *grid = new QwtPlotGrid();
  grid->attach(this);

  QwtPlotCurve *curve = new QwtPlotCurve();
  curve->setTitle("Some Points");
  curve->setPen(Qt::blue, 4);
  curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

  QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
    QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(8, 8));
  curve->setSymbol(symbol);

  QPolygonF points;
  points << QPointF(0.0, 4.4) << QPointF(1.0, 3.0)
    << QPointF(2.0, 4.5) << QPointF(3.0, 6.8)
    << QPointF(4.0, 7.9) << QPointF(5.0, 7.1);
  curve->setSamples(points);

  setAxisTitle(QwtPlot::xBottom, "X Axis");
  setAxisTitle(QwtPlot::yLeft, "Y Axis");

  curve->attach(this);



  resize(600, 400);
  show();

}


#ifndef QT_NO_PRINTER

void GraphViewer::print()
{
  QPrinter printer(QPrinter::HighResolution);

  QString docName = title().text();
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

    renderer.renderTo(this, printer);
  }
}

#endif

void GraphViewer::exportDocument()
{
  QwtPlotRenderer renderer;
  renderer.exportTo(this, "bode.pdf");
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

}

void GraphViewer::moved(const QPoint &pos)
{
  QString info;
  info.sprintf("Freq=%g, Ampl=%g, Phase=%g",
    invTransform(QwtPlot::xBottom, pos.x()),
    invTransform(QwtPlot::yLeft, pos.y()),
    invTransform(QwtPlot::yRight, pos.y())
    );
  showInfo(info);
}

void GraphViewer::selected(const QPolygon &)
{
  showInfo();
}