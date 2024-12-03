cmake_minimum_required(VERSION 3.14)

project(Game)

set(EXE_NAME Game)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

if(WIN32)
	set(CMAKE_STATIC_LIBRARY_PREFIX "")
	set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(Freetype CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED) # OpenAL-soft
find_package(SndFile CONFIG REQUIRED)

set(THIRD_LIBS
	glfw
	Freetype::Freetype
	OpenAL::OpenAL
	SndFile::sndfile
)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Bin/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

macro(CopyFilesToOutput _folderName)

	file(COPY ${CMAKE_SOURCE_DIR}/${_folderName}/ DESTINATION ${OUTPUT_DIR}/${_folderName}/)

endmacro(CopyFilesToOutput)

CopyFilesToOutput(Assets)
CopyFilesToOutput(Shaders)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message("GNU")
	add_compile_options(
		-Wall
		-Werror
		-Wno-switch
		-static
		-Wno-overloaded-virtual
	)
endif()

if(MSVC)
	message("MSVC")
	add_compile_options(
		/utf-8
		/W4
		/WX
		/permissive
		$<$<CONFIG:Debug>:/MTd>
		$<$<CONFIG:Release>:/MT>
	)
endif()

macro(AddFiles _dest)

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	foreach(loop ${ARGV})
		string(REGEX REPLACE "/" "_" _filesvar ${loop})
		
		file(GLOB ${_filesvar} ./${loop}/*.*)
		source_group("${loop}" FILES ${${_filesvar}})

		string(APPEND ${_dest} "${${_filesvar}};")
	endforeach()

endmacro(AddFiles)

AddFiles(CODE_FILES
	Code
	Code/GUI
	Code/Components
	Code/Components/Editor
	Code/Components/Machine
)

AddFiles(ASSET_FILES
	Assets
	Assets/testfolder
)

AddFiles(SHADER_FILES
	Shaders
)

add_executable(${EXE_NAME}
	${CODE_FILES}
	${ASSET_FILES}
	${SHADER_FILES}
	
	${CMAKE_SOURCE_DIR}/Framework/
	${CMAKE_SOURCE_DIR}/ThirdInclude/
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/Framework/
	${CMAKE_SOURCE_DIR}/ThirdInclude/
)

if(WIN32)
	if(MSVC)
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
		$<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/Framework_x64_debug.lib>
		$<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/Framework_x64_release.lib>
		$<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/box2dd.lib>
		$<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/box2d.lib>
		${THIRD_LIBS}
	)
	else()
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
		${CMAKE_SOURCE_DIR}/Framework_mingw.lib
		${CMAKE_SOURCE_DIR}/box2d_mingw.lib
		${THIRD_LIBS}
	)
	endif()
else()
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/libFramework.a
	${CMAKE_SOURCE_DIR}/libbox2d.a
	${THIRD_LIBS}
)
endif()

if(MSVC)
	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_DIR}
	)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
		VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME}
	)
endif()

# Edit & Continue
if(MSVC)
	target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "/ZI")
	target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()
