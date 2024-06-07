cmake_minimum_required(VERSION 3.14)

project(Game)

set(EXE_NAME Game)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Bin/Windows-x64/)

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
	)
endif()

if(MSVC)
	message("MSVC")
	add_compile_options(
		/utf-8
		/W4
		/WX
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
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/Framework/
	${CMAKE_SOURCE_DIR}/ThirdInclude/
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/Framework_x64_debug.lib
	${CMAKE_SOURCE_DIR}/box2d_debug.lib

	${CMAKE_SOURCE_DIR}/glfw3.lib
	${CMAKE_SOURCE_DIR}/freetype.lib
	${CMAKE_SOURCE_DIR}/OpenAL32.lib
	${CMAKE_SOURCE_DIR}/sndfile.lib
)

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