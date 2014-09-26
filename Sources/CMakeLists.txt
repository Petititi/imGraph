  
QT4_ADD_RESOURCES(_RCC_OUTFILES window_QT.qrc)
QT4_WRAP_CPP(HEADERS_MOC "Window.h" "GraphicView.h" "window_QT.h" "Connectors.h")

new_executable( imGraph )