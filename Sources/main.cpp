#include "GraphicView.h"
#include <QApplication>

using namespace charliesoft;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Window::getInstance()->show();
  return app.exec();
}