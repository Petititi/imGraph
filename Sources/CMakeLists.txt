file(GLOB_RECURSE files_QWT_headers "../include/qwt/*.h*")
include_directories( "${CMAKE_SOURCE_DIR}/Sources" )

QT5_ADD_RESOURCES(_RCC_OUTFILES view/MatrixViewer.qrc)
QT5_WRAP_CPP(HEADERS_MOC "blocks/ParamValue.h" "view/GuiReceiver.h" "view/GraphViewer.h" "view/MatrixViewer.h" "view/SmallDialogs.h")

new_library( imGraph QWT )
qt5_use_modules(imGraph Core Gui OpenGL Network Widgets Test PrintSupport Svg)