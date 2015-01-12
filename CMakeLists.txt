cmake_minimum_required ( VERSION 2.6 )

cmake_policy( SET CMP0015 OLD )
cmake_policy( SET CMP0003 NEW )

project ( imGraph )

if( MSVC )
    set( CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "" FORCE )
endif( )


# ----------------------------------------------------------------------------
#  Current version number:
# ----------------------------------------------------------------------------
set( SURANS_VERSION "1.0.0" )
string( REGEX MATCHALL "[ 0-9 ]+" SURANS_VERS_PARTS "${SURANS_VERSION}" )

list( GET SURANS_VERS_PARTS 0 SURANS_VERS_MAJOR )
list( GET SURANS_VERS_PARTS 1 SURANS_VERS_MINOR )
list( GET SURANS_VERS_PARTS 2 SURANS_VERS_PATCH )


# Alternatively you could simply include version.rc in another rc file
# if there already is one in one of the files in ${SOURCES}


include( ProjectAdd.cmake REQUIRED )

FIND_PACKAGE( OpenCV REQUIRED )# opencv_calib3d opencv_core opencv_objdetect opencv_features2d opencv_highgui opencv_imgproc opencv_flann opencv_nonfree
set ( OPENCV_CONCAT "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}" )

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(OPENGL REQUIRED)
set(QT_USE_QTOPENGL TRUE)

#include_directories(${Qt5Widgets_INCLUDES})
#add_definitions(${Qt5Widgets_DEFINITIONS})

find_package(Boost REQUIRED)
if( Boost_FOUND )
    if ("${Boost_LIBRARY_DIRS}" STREQUAL "")
        if(EXISTS "${Boost_INCLUDE_DIR}/lib32-msvc-12.0")
            set( Boost_LIBRARY_DIRS ${Boost_INCLUDE_DIR}/lib32-msvc-12.0)
        else()
            set( Boost_LIBRARY_DIRS ${Boost_INCLUDE_DIR}/lib32-msvc-9.0)
        endif()
    endif()
    include_directories( ${Boost_INCLUDE_DIR} )
	LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )
else()
    MESSAGE(STATUS "--------Boost Not Found.....")
    MESSAGE(STATUS "--------")
    MESSAGE(STATUS "--------Please download/install latest version:")
    MESSAGE(STATUS "--------")
    MESSAGE(STATUS "http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0-build2/")
    MESSAGE(STATUS "--------(pour Visual 2008, c'est boost_1_55_0-msvc-9.0-32.exe)")
endif( )

set( DETECT_LINKER_LIBS "${OpenCV_LIBS}")

find_package(OpencvInput REQUIRED)
MESSAGE(STATUS "OPENCVINPUT_INCLUDE_DIRS : ${OPENCVINPUT_INCLUDE_DIRS}")

set( DIR_ADD_INCLUDE "${CMAKE_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR} ${OPENCVINPUT_INCLUDE_DIRS})
MESSAGE(STATUS "DIR_ADD_INCLUDE : ${DIR_ADD_INCLUDE}")

set( ADD_LIB_DIR "${SuransLib}" "${CMAKE_SOURCE_DIR}/libs" ${Boost_LIBRARY_DIRS} ${QT_QTOPENGL_LIBRARY} )

set( ADD_LIBRARIES 
    "debug;glew32sd.lib;optimized;glew32s.lib"
    ${OPENCVINPUT_LIBRARIES} )

add_subdirectory( Sources )

# ----------------------------------------------------------------------------
#  Copy required OpenCV and QT5 dlls into output path
# ----------------------------------------------------------------------------
if (MSVC)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
	if(OpenCV_DIR)
		foreach(dll_name ${OpenCV_LIBS})
            set(dll "${dll_name}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}")
			find_library(OpenCV_DLL_PATH_${dll} NAMES ${dll} PATHS "${OpenCV_DIR}/bin" PATH_SUFFIXES Release )
			if (EXISTS ${OpenCV_DLL_PATH_${dll}})
				file(COPY ${OpenCV_DLL_PATH_${dll}} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Release)
			else()
				MESSAGE("[Warning] ${dll}.dll cannot be found in ${OpenCV_DIR}/bin/Release")
			endif()
			find_library(OpenCV_DLL_PATH_${dll}d ${dll}d PATHS "${OpenCV_DIR}/bin" PATH_SUFFIXES Debug )
			if (EXISTS ${OpenCV_DLL_PATH_${dll}d})
				file(COPY ${OpenCV_DLL_PATH_${dll}d} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Debug)
			else()
				MESSAGE("[Warning] ${dll}d.dll cannot be found in ${OpenCV_DIR}/bin/Debug")
			endif()
		endforeach()
	else()
		MESSAGE("[Warning] OpenCV_DIR is missing, opencv DLLs cannot be copied to executable directory") 
	endif()
    
	get_target_property(qtrelease Qt5::Core LOCATION_RELEASE) 
    get_filename_component(qtdir ${qtrelease} DIRECTORY)
    
	SET(QT5_DLLS Core Gui Widgets Test Opengl)
	foreach(dll ${QT5_DLLS})
        if (EXISTS "${qtdir}/Qt5${dll}.dll")
            file(COPY "${qtdir}/Qt5${dll}.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Release)
        endif()
        if (EXISTS "${qtdir}/Qt5${dll}d.dll")
            file(COPY "${qtdir}/Qt5${dll}d.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Debug)
        endif()
	endforeach()
	
	SET(QT5_DLLS_ENGINE icudt52 icuin52 icuuc52 icudt53 icuin53 icuuc53)
	foreach(dll ${QT5_DLLS_ENGINE})
        if (EXISTS "${qtdir}/${dll}.dll")
            file(COPY "${qtdir}/${dll}.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Release)
            file(COPY "${qtdir}/${dll}.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/Debug)
        else()
            MESSAGE("[Warning] ${dll}.dll cannot be found in ${qtdir}")
        endif()
	endforeach()
	
endif()
