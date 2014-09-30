  
QT4_ADD_RESOURCES(_RCC_OUTFILES view/window_QT.qrc)
QT4_WRAP_CPP(HEADERS_MOC "view/Window.h" "view/GraphicView.h" "view/window_QT.h" "view/Connectors.h")

new_executable( imGraph )