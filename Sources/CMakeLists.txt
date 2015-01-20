file(GLOB_RECURSE files_QWT_headers "../include/qwt/*.h*")
QT5_ADD_RESOURCES(_RCC_OUTFILES view/MatrixViewer.qrc)
QT5_WRAP_CPP(HEADERS_MOC "view/Window.h" "view/GraphicView.h" "view/GuiReceiver.h" "view/GraphViewer.h" "view/MatrixViewer.h" "view/Connectors.h" "view/ParameterDock.h" "view/SmallDialogs.h" "view/VertexRepresentation.h" ${files_QWT_headers})

new_executable( imGraph )
qt5_use_modules(imGraph Core Gui OpenGL Network Widgets Test PrintSupport Svg)