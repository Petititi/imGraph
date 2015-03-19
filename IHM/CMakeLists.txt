include_directories( "${CMAKE_SOURCE_DIR}/Sources" )

if( MSVC )
    set(listOfBlocks "")
	file(GLOB_RECURSE files_blocks_list "../Sources/blocks/*.c*")
    foreach(block_name ${files_blocks_list})
        FILE(READ "${block_name}" contents)

        #STRING(REGEX REPLACE "\n" ";" contents "${contents}")
        string( REGEX MATCHALL "BLOCK_BEGIN_INSTANTIATION\(([^\)]+)\)" _matched "${contents}" )
        
        foreach(block_full_name ${_matched})
            string(SUBSTRING "${block_full_name}" 26 -1 final_name)
            list (APPEND listOfBlocks "${final_name}")
        endforeach()
    endforeach()
    
    #create listOfBlocks.h
    set(SRC_BLOCKS "${CMAKE_BINARY_DIR}/listOfBlocks.h")
    
    file(WRITE "${SRC_BLOCKS}" "//This file is automatically created by CMake (IHM/CMakeLists.txt).\n//Don't make any changes!!!\n\n\n" )
    file(APPEND "${SRC_BLOCKS}" "namespace charliesoft\n{\n")
    foreach(block ${listOfBlocks})
        file(APPEND "${SRC_BLOCKS}" "  int ${block}_InMyLibrary();\n")
    endforeach()
    file(APPEND "${SRC_BLOCKS}" "}\n\n")
    foreach(block ${listOfBlocks})
        file(APPEND "${SRC_BLOCKS}" "static int dummy_${block} = charliesoft::${block}_InMyLibrary();\n")
    endforeach()
elseif()
    file(WRITE "${SRC_BLOCKS}" "//This file is automatically created by CMake (IHM/CMakeLists.txt).\n//Don't make any changes!!!\n\n\n" )
endif( )

QT5_ADD_RESOURCES(_RCC_OUTFILES ../Sources/view/MatrixViewer.qrc)
QT5_WRAP_CPP(HEADERS_MOC "view/Window.h" "view/GraphicView.h" "view/ImagePreview.h" "view/Connectors.h" "view/ParameterDock.h" "view/VertexRepresentation.h")

new_executable( imGraphIHM imGraph )
qt5_use_modules(imGraphIHM Core Gui OpenGL Network Widgets Test PrintSupport Svg)


if( MSVC )
    set(listOfBlocks "")
    SET_TARGET_PROPERTIES(imGraphIHM PROPERTIES LINK_FLAGS "/OPT:NOREF")
endif( )