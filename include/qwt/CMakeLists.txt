include_directories( "${CMAKE_SOURCE_DIR}/IHM" )

file(GLOB_RECURSE files_QWT_headers "*.h*")
file(GLOB_RECURSE files_QWT_srcs "*.c*")
source_group("Src\\qwt" FILES ${files_QWT_srcs})


QT5_WRAP_CPP(HEADERS_MOC ${files_QWT_headers})

new_library( QWT )
qt5_use_modules(QWT Core Gui OpenGL Network Widgets Test PrintSupport Svg)