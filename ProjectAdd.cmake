
macro(begin_new_project name)

	project(${name})
	add_definitions(-DCVAPI_EXPORTS)

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}"
	                    "${CMAKE_CURRENT_BINARY_DIR}"
	                    "${CMAKE_BINARY_DIR}"
                        "${DIR_ADD_INCLUDE}")

	file(GLOB files_srcs "*.c*")
	file(GLOB files_int_hdrs "*.h*")
	source_group("Src" FILES ${files_srcs})
	source_group("Include" FILES ${files_int_hdrs})

	set(the_target "${name}")

	set(file_list_ ${files_srcs} ${files_int_hdrs} ${cfg_file} )
	if( DEFINED HEADERS_MOC )
		set(file_list_ ${file_list_} ${HEADERS_MOC})
		MESSAGE(STATUS "HEADERS_MOC : ${HEADERS_MOC}")
	endif( )
	link_directories( ${ADD_LIB_DIR} )
endmacro()

macro(end_new_project name)

	# For dynamic link numbering convenions
	set_target_properties(${the_target} PROPERTIES
	    VERSION ${DETECT_VERSION}
	    SOVERSION ${DETECT_VERSION}
	    OUTPUT_NAME "${the_target}"
	    )
	# Additional target properties
	set_target_properties(${the_target} PROPERTIES
	    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
	    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
	    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
	    )

	if(MSVC)
	    if(CMAKE_CROSSCOMPILING)
	        set_target_properties(${the_target} PROPERTIES
	            LINK_FLAGS "/NODEFAULTLIB:secchk"
	            )
	    endif()
	    set_target_properties(${the_target} PROPERTIES
	        LINK_FLAGS "/NODEFAULTLIB:libc;libcmt"
	        )
	endif()

	# Dependencies of this target:
	if(ARGN)
		add_dependencies(${the_target} ${ARGN})
	endif()
	
	# Add the required libraries for linking:
	target_link_libraries(${the_target} ${DETECT_LINKER_LIBS} ${QT_LIBRARIES} ${ADD_LIBRARIES} ${ARGN})

endmacro()


macro(new_library name)

	begin_new_project(${name})
	
	add_library(${the_target} ${LIB_TYPE} ${file_list_})
MESSAGE(STATUS "add_library : (${the_target} ${LIB_TYPE} ${file_list_})")
	
	end_new_project(${name} ${ARGN})
endmacro()


macro(new_executable name)

	begin_new_project(${name})
	
	add_executable(${the_target} ${file_list_})
	
	end_new_project(${name} ${ARGN})
	
endmacro()
