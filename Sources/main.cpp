#include "GraphicView.h"
#include <QApplication>

using namespace charliesoft;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Fenetre::getInstance()->show();
  return app.exec();
}