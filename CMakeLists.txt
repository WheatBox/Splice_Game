cmake_minimum_required(VERSION 3.14)

project(Game)

set(EXE_NAME Game)

include(utils.cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/Third/)
set(THIRD_PARTY_PLATFORM_DIR "")

set(OUTPUT_DIR_DEBUG ${CMAKE_SOURCE_DIR}/Bin/Debug/)
set(OUTPUT_DIR_RELEASE ${CMAKE_SOURCE_DIR}/Bin/Release/)

find_package(glfw3 CONFIG REQUIRED)

set(THIRD_LIBS
	glfw
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR_DEBUG})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR_RELEASE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR_DEBUG})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR_RELEASE})

CopyFilesToOutput(Assets)
CopyFilesToOutput(Shaders)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message("GNU")
	add_compile_options(
		-Wall
		-Werror
		-Wno-switch
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
	)
endif()

AddFiles(CODE_FILES
	Code
	Code/GUI
	Code/Components
	Code/Components/Editor
	Code/Components/Machine
)

AddFiles(SHADER_FILES
	Shaders
)

add_executable(${EXE_NAME}
	${CODE_FILES}
	${SHADER_FILES}
	${CMAKE_SOURCE_DIR}/Framework/
	${CMAKE_SOURCE_DIR}/ThirdInclude/
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/Framework/
	${CMAKE_SOURCE_DIR}/ThirdInclude/
)

set(LIBDIR ${CMAKE_SOURCE_DIR}/Lib/)
if(WIN32)
	if(MSVC)
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
		$<$<CONFIG:Debug>:${LIBDIR}/Framework_d.lib>
		$<$<CONFIG:Release>:${LIBDIR}/Framework.lib>
		$<$<CONFIG:Debug>:${LIBDIR}/box2dd.lib>
		$<$<CONFIG:Release>:${LIBDIR}/box2d.lib>
		${THIRD_LIBS}
	)
	else()
	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
		${LIBDIR}/Framework_mingw.lib
		${LIBDIR}/box2d_mingw.lib
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
		VS_DEBUGGER_WORKING_DIRECTORY "$<IF:$<CONFIG:Debug>,${OUTPUT_DIR_DEBUG},${OUTPUT_DIR_RELEASE}>"
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