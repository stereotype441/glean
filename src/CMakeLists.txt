cmake_minimum_required(VERSION 2.8)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)

project (glean)

find_package(OpenGL REQUIRED)
find_package(TIFF)
find_package(GLUT)

# Put all executables into the bin subdirectory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Do the same for MSVC, regardless of the build type. This only works correctly
# for CMake 2.8.1 and above.
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)

if (NOT MSVC)
	check_c_compiler_flag ("-Wall" C_COMPILER_FLAG_WALL)
	if (C_COMPILER_FLAG_WALL)
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	endif (C_COMPILER_FLAG_WALL)
	check_cxx_compiler_flag("-Wall" CXX_COMPILER_FLAG_WALL)
	if (CXX_COMPILER_FLAG_WALL)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	endif (CXX_COMPILER_FLAG_WALL)
else ()
	# -Wall or (/Wall) is actually supported by MSVC and would be detected
	# by CHECK_C_COMPILER_FLAG above, but is very pedantic, causing
	# thousand of warnings when including windows.h.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W4")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
	
	add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif ()

if (APPLE)
	# We rely on frameworks which where deprecated on x86-64
	set (CMAKE_OSX_ARCHITECTURES i386)
endif ()

if (NOT TIFF_FOUND)
	set (TIFF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/tiff ${CMAKE_SOURCE_DIR}/tiff)
	set (TIFF_LIBRARY bundled_tiff)
	add_subdirectory (tiff)
endif ()

set (GLEXT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/glext)

if (WIN32)
	add_definitions (-D__WIN__ -D__MS__)
elseif (APPLE)
	add_definitions (-D__AGL__ -D__UNIX__)
	find_library (CARBON_LIBRARY Carbon)
else ()
	add_definitions (-D__X11__ -D__UNIX__)
endif ()

include_directories (
	libs/dsurf
	libs/image
	libs/lex
	libs/rand
	libs/stats
	libs/timer
	glh
	${GLEXT_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${TIFF_INCLUDE_DIR}
)

add_subdirectory(libs)
add_subdirectory(glean)
add_subdirectory(tools)
