  
QT4_ADD_RESOURCES(_RCC_OUTFILES view/MatrixViewer.qrc)
QT4_WRAP_CPP(HEADERS_MOC "view/Window.h" "view/GraphicView.h" "view/MatrixViewer.h" "view/Connectors.h" "view/ParameterDock.h")

new_executable( imGraph )