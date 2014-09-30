
MACRO(ocv_check_compiler_flag LANG FLAG RESULT)
  if(NOT DEFINED ${RESULT})
    if("_${LANG}_" MATCHES "_CXX_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx")
      if("${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main() { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main() { return 0; }\n")
      endif()
    elseif("_${LANG}_" MATCHES "_C_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c")
      if("${CMAKE_C_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_C_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main(void) { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main(void) { return 0; }\n")
      endif()
    elseif("_${LANG}_" MATCHES "_OBJCXX_")
      set(_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.mm")
      if("${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror " OR "${CMAKE_CXX_FLAGS} ${FLAG} " MATCHES "-Werror=unknown-pragmas ")
        FILE(WRITE "${_fname}" "int main() { return 0; }\n")
      else()
        FILE(WRITE "${_fname}" "#pragma\nint main() { return 0; }\n")
      endif()
    else()
      unset(_fname)
    endif()
    if(_fname)
      MESSAGE(STATUS "Performing Test ${RESULT}")
      TRY_COMPILE(${RESULT}
        "${CMAKE_BINARY_DIR}"
        "${_fname}"
        COMPILE_DEFINITIONS "${FLAG}"
        OUTPUT_VARIABLE OUTPUT)

      FOREACH(_regex ${OCV_COMPILER_FAIL_REGEX})
        IF("${OUTPUT}" MATCHES "${_regex}")
          SET(${RESULT} 0)
          break()
        ENDIF()
      ENDFOREACH()

      IF(${RESULT})
        SET(${RESULT} 1 CACHE INTERNAL "Test ${RESULT}")
        MESSAGE(STATUS "Performing Test ${RESULT} - Success")
      ELSE(${RESULT})
        MESSAGE(STATUS "Performing Test ${RESULT} - Failed")
        SET(${RESULT} "" CACHE INTERNAL "Test ${RESULT}")
      ENDIF(${RESULT})
    else()
      SET(${RESULT} 0)
    endif()
  endif()
ENDMACRO()

macro(ocv_check_flag_support lang flag varname)
  if("_${lang}_" MATCHES "_CXX_")
    set(_lang CXX)
  elseif("_${lang}_" MATCHES "_C_")
    set(_lang C)
  elseif("_${lang}_" MATCHES "_OBJCXX_")
    set(_lang OBJCXX)
  else()
    set(_lang ${lang})
  endif()

  string(TOUPPER "${flag}" ${varname})
  string(REGEX REPLACE "^(/|-)" "HAVE_${_lang}_" ${varname} "${${varname}}")
  string(REGEX REPLACE " -|-|=| |\\." "_" ${varname} "${${varname}}")

  ocv_check_compiler_flag("${_lang}" "${ARGN} ${flag}" ${${varname}})
endmacro()

macro(begin_new_project name)

	project(${name})
	add_definitions(-DCVAPI_EXPORTS)

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}"
	                    "${CMAKE_CURRENT_BINARY_DIR}"
	                    "${CMAKE_BINARY_DIR}"
                        "${DIR_ADD_INCLUDE}")

	file(GLOB files_srcs "*.c*")
	file(GLOB files_int_hdrs "*.h*")
	file(GLOB_RECURSE files_blocks_srcs "blocks/*.c*")
	file(GLOB_RECURSE files_blocks_int_hdrs "blocks/*.h")
	file(GLOB_RECURSE files_view_srcs "view/*.c*")
	file(GLOB_RECURSE files_view_int_hdrs "view/*.h")
	source_group("Src" FILES ${files_srcs})
	source_group("Include" FILES ${files_int_hdrs})
	source_group("Src\\blocks" FILES ${files_blocks_srcs})
	source_group("Include\\blocks" FILES ${files_blocks_int_hdrs})
	source_group("Src\\view" FILES ${files_view_srcs})
	source_group("Include\\view" FILES ${files_view_int_hdrs})

	set(the_target "${name}")

	set(file_list_ ${files_srcs} ${files_int_hdrs} ${files_blocks_srcs} ${files_blocks_int_hdrs} ${files_view_srcs} ${files_view_int_hdrs} ${cfg_file} )
	if( DEFINED HEADERS_MOC )
		set(file_list_ ${file_list_} ${HEADERS_MOC})
		MESSAGE(STATUS "HEADERS_MOC : ${HEADERS_MOC}")
	endif( )
	if( DEFINED _RCC_OUTFILES )
		set(file_list_ ${file_list_} ${_RCC_OUTFILES})
        ocv_check_flag_support(CXX -Wno-missing-declarations _have_flag)
        if(${_have_flag})
          set_source_files_properties(${_RCC_OUTFILES} PROPERTIES COMPILE_FLAGS -Wno-missing-declarations)
        endif()
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
