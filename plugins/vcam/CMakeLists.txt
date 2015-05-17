IF (WIN32)
	
	IF(NOT WINDOWS_SDK_DIR)
		MESSAGE(WARNING "[VCAM] WINDOWS_SDK_DIR is not specified, skip VCam plugin. WindowsSDK can be downloaded at http://www.microsoft.com/en-us/download/details.aspx?id=11310")
	ELSE()
	
		target_include_directories(imGraph PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
		include_directories(${WINDOWS_SDK_DIR}/Samples/multimedia/directshow/baseclasses ${WINDOWS_SDK_DIR}/Include)

		link_directories(${WINDOWS_SDK_DIR}/Lib ${WINDOWS_SDK_DIR}/Samples/Multimedia/DirectShow/BaseClasses/${CMAKE_BUILD_TYPE})

		add_library("VCam" MODULE Dll.cpp Filters.cpp Filters.def Filters.h targetver.h vcam.cpp)

	ENDIF()
	
ENDIF()