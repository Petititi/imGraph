#include "view/GraphicView.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800)
#endif
#include <QApplication>
#include "opencv2/features2d.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace charliesoft;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Window::getInstance()->show();
  return app.exec();
}