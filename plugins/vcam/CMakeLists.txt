if (WIN32)
	
	IF(NOT WINDOWS_SDK_DIR)
		set(WINDOWS_SDK_DIR "C:/Program Files/Microsoft SDKs/Windows/v7.0")
	ENDIF()

	include_directories(${WINDOWS_SDK_DIR}/Samples/multimedia/directshow/baseclasses ${WINDOWS_SDK_DIR}/Include)

	link_directories(${WINDOWS_SDK_DIR}/Lib ${WINDOWS_SDK_DIR}/Samples/Multimedia/DirectShow/BaseClasses/${CMAKE_BUILD_TYPE})

	add_library("VCam" MODULE Dll.cpp Filters.cpp Filters.def Filters.h targetver.h vcam.cpp)


endif()