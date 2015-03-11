include_directories( "${CMAKE_SOURCE_DIR}/Sources" )

#set( HEADERS_QT  "CorbaSimulator.h" )
#QT5_WRAP_CPP(HEADERS_MOC ${HEADERS_QT})

set(SRC_BLOCKS "${CMAKE_BINARY_DIR}/listOfBlocks.h")
file(APPEND "${SRC_BLOCKS}" "\n\n#define SOURCE_DIRECTORY \"${CMAKE_SOURCE_DIR}\"\n")
new_executable( unitTests imGraph)
if( MSVC )
    SET_TARGET_PROPERTIES(unitTests PROPERTIES LINK_FLAGS "/OPT:NOREF")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
endif( )
