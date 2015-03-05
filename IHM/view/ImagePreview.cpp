#include "ImagePreview.h"
#include "VertexRepresentation.h"

using namespace charliesoft;
using std::vector;

namespace charliesoft
{
  void ImageViewer::mouseMoveEvent(QMouseEvent *event)
  {
    MainVertexBlock *me = dynamic_cast<MainVertexBlock *>(parent());
    if (me != NULL)
    {
      if (event->buttons() == Qt::NoButton)
        me->updateMouseState(event->pos());
    }
    event->ignore();
  };

  void ImageViewer::enterEvent(QEvent *event)
  {
    MainVertexBlock *me = dynamic_cast<MainVertexBlock *>(parent());
    if (me != NULL)
    {
      setMouseTracking(true);
      QEnterEvent *ee = dynamic_cast<QEnterEvent *>(event);
      if (ee != NULL)
        me->updateMouseState(ee->pos());
    }
  };
  void ImageViewer::leaveEvent(QEvent *event)
  {
    MainVertexBlock *me = dynamic_cast<MainVertexBlock *>(parent());
    if (me != NULL)
      setMouseTracking(false);
  };
}