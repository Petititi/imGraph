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
using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Window::getInstance()->show();
  return app.exec();
}


/*! \mainpage Surans
*
\section sec1 Dependencies
<p>The project is primarily developed on Windows, but we seek to make ImGraph a cross-platform project. The majority of the code should already be compatible with Linux and macOS, but it has not been tested...<br/>
We also use some of the c++11 improvement, so you will need at least visual studio 2013 or g++ (>=4.6).
</p>
\subsection OpenCV
OpenCV is an image/video processing library which was designed for computational efficiency and with a strong focus on real-time applications.
Written in optimized C/C++, the library can take advantage of multi-core processing.<br/>
As the version 3.0 is still not fully functionnal, we use an intermediate version from git://code.opencv.org/opencv.git, with the SHA-1 revision : 9eca3ec8f651e55c25094e4e76446f4bd904826c.<br/>
\subsection CMake
Cmake is a cross-platform, open-source build system. You can <a href="http://www.cmake.org/cmake/resources/software.html">download Cmake here</a>.
You can follow <a href="http://www.cmake.org/cmake-tutorial/">this tutorial</a> to understand the full potential of this program.
\subsection QT
Qt is a cross-platform application framework that is widely used for IHM that can be run on various software and hardware platforms
with little or no change in the codebase, while having the power and speed of native applications. ImGraph was primarily developped with Qt 4.8.5 but was successfully compiled with Qt 5!
\subsection Boost
Boost is a set of libraries for the C++ programming language. Many of Boost's founders are on the C++ standards committee, and several Boost libraries have been accepted for incorporation into both Technical Report 1 and the C++11 standard<br/>
We use the 1.55 version (some problem exist with 1.57), you can found some <a href="http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0-build2/">prebuild binaries here</a>.
\subsection InputLoader
This is a private library used to easily load/store videos (from webcam, files or folder).\n
You can <a href="https://synogsm.lsis.univ-tln.fr/indefero/index.php/p/inputloader/source/tree/master/">download it here</a>. You should build and install it before building ImGraph!

\section sec2 Project Structure
To be done!
*/