#
# SuperTux - root build script
# Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#


#
# INSTRUCTIONS:
# -------------
#
# Create a directory build/ and change to it. Run
#
#   cmake ..
#
# This creates a set of Makefiles to build the project. Run
#
#   make
#


## Project name to use as command prefix.

PROJECT(SUPERTUX)


### CMake configuration

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0008 NEW)
  CMAKE_POLICY(SET CMP0023 NEW)
  IF(POLICY CMP0043)
    CMAKE_POLICY(SET CMP0043 OLD)
  ENDIF(POLICY CMP0043)
ENDIF(COMMAND cmake_policy)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk/cmake)
INCLUDE(ConfigureFiles)
INCLUDE(ExternalProject)

## For autopackage
SET(APPDATADIR "${CMAKE_INSTALL_PREFIX}/share/games/supertux2")

SET(BUILD_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
SET(BUILD_CONFIG_DATA_DIR "${CMAKE_BINARY_DIR}/data")
## Check endianess

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

## Add definitions
IF(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  ADD_DEFINITIONS(-DRELEASE)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

SET(SUPERTUX_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
## These variables enable MSVC to find libraries located in "dependencies{32|64}"
IF(WIN32)
  ## Store path of dependecy folder
  SET(DEPENDENCY_FOLDER "${PROJECT_SOURCE_DIR}/dependencies")

  ## To test if the host (not the build) is x64:
  ## "$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "AMD64"
  IF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(WIN64 ON)
    SET(DEPENDENCY_FOLDER "${DEPENDENCY_FOLDER}64")
  ELSE(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(WIN64 OFF)
    SET(DEPENDENCY_FOLDER "${DEPENDENCY_FOLDER}32")
  ENDIF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)

  IF(WIN64)
    SET(SUPERTUX_SYSTEM_NAME win64)
  ELSE(WIN64)
    SET(SUPERTUX_SYSTEM_NAME win32)
  ENDIF(WIN64)

  SET(ENV{PATH} "$ENV{PATH};${DEPENDENCY_FOLDER}/include")
  SET(ENV{LIB} "${DEPENDENCY_FOLDER}/lib")
  SET(ENV{OPENALDIR} "${DEPENDENCY_FOLDER}")

  SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
  ## Enable multi-processor compilation (faster)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  ## And shut up about unsafe stuff
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ## Add an icon
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")

  ## Find all used libraries
  FILE(GLOB DLLS "${DEPENDENCY_FOLDER}/dll/*")
ENDIF(WIN32)

## Add lots of dependencies to compiler switches

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
IF(WIN32)
  OPTION(ENABLE_BOOST_STATIC_LIBS "Use boost static libraries" ON)
ELSE(WIN32)
  OPTION(ENABLE_BOOST_STATIC_LIBS "Use boost static libraries" OFF)
ENDIF(WIN32)
IF(ENABLE_BOOST_STATIC_LIBS)
  SET(Boost_USE_STATIC_LIBS TRUE)
ELSE(ENABLE_BOOST_STATIC_LIBS)
  SET(Boost_USE_STATIC_LIBS FALSE)
ENDIF(ENABLE_BOOST_STATIC_LIBS)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system date_time locale)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

find_package(PNG REQUIRED)

IF(WIN32)
  FIND_PATH(SDL2_INCLUDE_DIRS NAMES SDL.h PATHS "${DEPENDENCY_FOLDER}/include/SDL2")
  FIND_PATH(SDL2IMAGE_INCLUDE_DIRS NAMES SDL_image.h PATHS "${DEPENDENCY_FOLDER}/include/SDL2_image")
  FIND_LIBRARY(SDL2_LIBRARIES NAMES SDL2 PATHS "${DEPENDENCY_FOLDER}/lib")
  FIND_LIBRARY(SDL2IMAGE_LIBRARIES NAMES SDL2_image PATHS "${DEPENDENCY_FOLDER}/lib")
  FIND_LIBRARY(SDL2MAIN_LIBRARIES NAMES SDL2main PATHS "${DEPENDENCY_FOLDER}/lib")
FIND_PATH(PHYSFS_INCLUDE_DIR NAMES physfs.h PATHS "${DEPENDENCY_FOLDER}/include/physfs")
ELSE(WIN32)
  INCLUDE(FindPkgConfig)
  PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2>=2.0.1)
  PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
ENDIF(WIN32)
SET(HAVE_SDL TRUE)

OPTION(BUILD_TESTS "Build test cases" OFF)
OPTION(ENABLE_OPENGL "Enable OpenGL support" ON)
OPTION(GLBINDING_ENABLED "Use glbinding instead of GLEW" OFF)
OPTION(GLBINDING_DEBUG_OUTPUT "Enable glbinding debug output for each called OpenGL function" OFF)
IF(ENABLE_OPENGL)
  SET(OpenGL_GL_PREFERENCE "LEGACY")
  FIND_PACKAGE(OpenGL)
  IF(OPENGL_FOUND)
    INCLUDE_DIRECTORIES(SYSTEM ${OPENGL_INCLUDE_DIR})
    SET(HAVE_OPENGL TRUE)
  ENDIF(OPENGL_FOUND)

  IF(GLBINDING_ENABLED)
    FIND_PACKAGE(GLBINDING REQUIRED)
    IF(GLBINDING_FOUND)
      INCLUDE_DIRECTORIES(SYSTEM ${GLBINDING_INCLUDES})
      ADD_DEFINITIONS(-DUSE_GLBINDING)
      IF (GLBINDING_DEBUG_OUTPUT)
        ADD_DEFINITIONS(-DUSE_GLBINDING_DEBUG_OUTPUT)
      ENDIF()
    ENDIF()
  ELSE()
    FIND_PACKAGE(GLEW REQUIRED)
    IF(GLEW_FOUND)
      INCLUDE_DIRECTORIES(SYSTEM ${GLEW_INCLUDE_DIR})
    ENDIF(GLEW_FOUND)
  ENDIF()
ENDIF(ENABLE_OPENGL)

FIND_PACKAGE(OpenAL REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${OPENAL_INCLUDE_DIR})

FIND_PACKAGE(OggVorbis REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${VORBIS_INCLUDE_DIR})

INCLUDE(CheckSymbolExists)

FIND_PACKAGE(PhysFS)
CHECK_SYMBOL_EXISTS("PHYSFS_getPrefDir" "${PHYSFS_INCLUDE_DIR}/physfs.h" HAVE_PHYSFS_GETPREFDIR)
IF(HAVE_PHYSFS_GETPREFDIR)
	SET(USE_SYSTEM_PHYSFS ON CACHE BOOL "Use preinstalled physfs (must support getPrefDir)")
ELSE(HAVE_PHYSFS_GETPREFDIR)
	SET(USE_SYSTEM_PHYSFS OFF CACHE BOOL "Use preinstalled physfs (must support getPrefDir)")
ENDIF(HAVE_PHYSFS_GETPREFDIR)

IF(NOT USE_SYSTEM_PHYSFS)
	IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/physfs/CMakeLists.txt)
	  message(FATAL_ERROR "physfs submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/physfs/CMakeLists.txt is missing")
	ENDIF()

	IF(WIN32)
		SET(PHYSFS_BUILD_SHARED TRUE)
		SET(PHYSFS_BUILD_STATIC FALSE)
	ELSE(WIN32)
		SET(PHYSFS_BUILD_SHARED FALSE)
		SET(PHYSFS_BUILD_STATIC TRUE)
	ENDIF()

	SET(PHYSFS_PREFIX ${CMAKE_BINARY_DIR}/physfs/)
	ExternalProject_Add(physfs
	  SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/physfs/"
	  CMAKE_ARGS
	  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
	  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
	  -DCMAKE_INSTALL_PREFIX=${PHYSFS_PREFIX}
	  -DLIB_SUFFIX=${LIB_SUFFIX}
	  -DPHYSFS_BUILD_SHARED=${PHYSFS_BUILD_SHARED}
	  -DPHYSFS_BUILD_STATIC=${PHYSFS_BUILD_STATIC}
	  -DPHYSFS_BUILD_TEST=FALSE)

	IF(WIN32)
		ADD_LIBRARY(physfs_lib SHARED IMPORTED)
		SET_TARGET_PROPERTIES(physfs_lib PROPERTIES IMPORTED_LOCATION "${PHYSFS_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}physfs${CMAKE_SHARED_LIBRARY_SUFFIX}")
		SET_TARGET_PROPERTIES(physfs_lib PROPERTIES IMPORTED_IMPLIB "${PHYSFS_PREFIX}/lib${LIB_SUFFIX}/physfs${CMAKE_LINK_LIBRARY_SUFFIX}")
	ELSE()
		ADD_LIBRARY(physfs_lib STATIC IMPORTED)
		SET_TARGET_PROPERTIES(physfs_lib PROPERTIES IMPORTED_LOCATION "${PHYSFS_PREFIX}/lib${LIB_SUFFIX}/${CMAKE_STATIC_LIBRARY_PREFIX}physfs${CMAKE_STATIC_LIBRARY_SUFFIX}")
	ENDIF()
	SET(PHYSFS_INCLUDE_DIR "${PHYSFS_PREFIX}/include/")
ENDIF()

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${PHYSFS_INCLUDE_DIR})


## The curl headers are in the standard include path in dependencies
## so it's not required to add them here
FIND_PACKAGE(CURL REQUIRED)
IF(CURL_FOUND)
	INCLUDE_DIRECTORIES(SYSTEM ${CURL_INCLUDE_DIR})
	SET(HAVE_LIBCURL TRUE)
ENDIF(CURL_FOUND)

#FIND_PACKAGE(ICONV REQUIRED)
#INCLUDE_DIRECTORIES(SYSTEM ${ICONV_INCLUDE_DIR})
#LINK_LIBRARIES(${ICONV_LIBRARY})

IF(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	## Find revision of WC
	MARK_AS_ADVANCED(GIT_EXECUTABLE)
	FIND_PROGRAM(GIT_EXECUTABLE git)
	IF(NOT GIT_EXECUTABLE EQUAL "GIT_EXECUTABLE-NOTFOUND")
		INCLUDE(GetGitRevisionDescription)
		git_describe(VERSION_STRING_GIT "--tags" "--match" "?[0-9]*.[0-9]*.[0-9]*")
		STRING(REPLACE "v" "" VERSION_LIST ${VERSION_STRING_GIT})
		STRING(REGEX REPLACE "(-|_|\\.)" ";" VERSION_LIST ";${VERSION_LIST}")
        ENDIF()
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/.git")

GET_FILENAME_COMPONENT(BASEDIR ${CMAKE_SOURCE_DIR} NAME)
IF("${VERSION_LIST}" STREQUAL "")
	IF(${BASEDIR} MATCHES "supertux2-[0-9\\.]*")
		STRING(REGEX REPLACE "(\\.|_|-)" ";" VERSION_LIST ${BASEDIR})
	ENDIF()
ENDIF()

FILE(GLOB ORIG_TGZ ../*.orig.tar.gz)
IF("${VERSION_LIST}" STREQUAL "" AND (NOT "${ORIG_TGZ}" STREQUAL ""))
	GET_FILENAME_COMPONENT(BASEDIR ${ORIG_TGZ} NAME)
	STRING(REGEX REPLACE "(\\.|_|-)" ";" VERSION_LIST ${BASEDIR})
ENDIF()

LIST(LENGTH VERSION_LIST VERSION_LIST_SIZE)

IF(${VERSION_LIST_SIZE} GREATER 0)
	LIST(GET VERSION_LIST 1 MAJOR_VERSION_GIT)
	LIST(GET VERSION_LIST 2 MINOR_VERSION_GIT)
	LIST(GET VERSION_LIST 3 PATCH_VERSION_GIT)

	IF("${VERSION_STRING_GIT}" STREQUAL "")
		SET(VERSION_STRING_GIT "${MAJOR_VERSION_GIT}.${MINOR_VERSION_GIT}.${PATCH_VERSION_GIT}")
	ENDIF()

	CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/version.cmake.in" "${CMAKE_SOURCE_DIR}/version.cmake")
ENDIF()
IF(NOT EXISTS "${CMAKE_SOURCE_DIR}/version.cmake")
	 MESSAGE( SEND_ERROR "Cound not find GIT or valid version.cmake. Version information will be invalid." )
ENDIF(NOT EXISTS "${CMAKE_SOURCE_DIR}/version.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/version.cmake")


SET(SUPERTUX_VERSION ${SUPERTUX_VERSION_STRING})

configure_file(version.h.in ${CMAKE_BINARY_DIR}/version.h )

SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/version.h
                            PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/main.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/supertux/title_screen.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/src/addon/addon_manager.cpp
                            PROPERTIES OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/version.h")

## Configure doxygen
# Adapted from https://tty1.net/blog/2014/cmake-doxygen_en.html
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Build API documentation using Doxygen" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

## Check platform-dependent build options

INCLUDE(ConfigureChecks)


## Also build external/squirrel

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt)
  message(FATAL_ERROR "squirrel submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/squirrel/CMakeLists.txt is missing")
ENDIF()

SET(SQUIRREL_PREFIX ${CMAKE_BINARY_DIR}/squirrel/)
ExternalProject_Add(squirrel
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/squirrel/"
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_INSTALL_PREFIX=${SQUIRREL_PREFIX}
  -DINSTALL_INC_DIR=include)

IF(WIN32)
	ADD_LIBRARY(squirrel_lib SHARED IMPORTED)
	SET_TARGET_PROPERTIES(squirrel_lib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}squirrel${CMAKE_SHARED_LIBRARY_SUFFIX}")
	SET_TARGET_PROPERTIES(squirrel_lib PROPERTIES IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/squirrel${CMAKE_LINK_LIBRARY_SUFFIX}")
	ADD_LIBRARY(sqstdlib_lib SHARED IMPORTED)
	SET_TARGET_PROPERTIES(sqstdlib_lib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}sqstdlib${CMAKE_SHARED_LIBRARY_SUFFIX}")
	SET_TARGET_PROPERTIES(sqstdlib_lib PROPERTIES IMPORTED_IMPLIB "${SQUIRREL_PREFIX}/lib/sqstdlib${CMAKE_LINK_LIBRARY_SUFFIX}")
	
	#For debug run purposes
	CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/mk/msvc/run_supertux.bat.in" "${PROJECT_BINARY_DIR}/run_supertux.bat")
ELSE()
	ADD_LIBRARY(squirrel_lib STATIC IMPORTED)
	SET_TARGET_PROPERTIES(squirrel_lib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}squirrel_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
	ADD_LIBRARY(sqstdlib_lib STATIC IMPORTED)
	SET_TARGET_PROPERTIES(sqstdlib_lib PROPERTIES IMPORTED_LOCATION "${SQUIRREL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqstdlib_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM ${SQUIRREL_PREFIX}/include)

## Also build external/tinygettext

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygettext/CMakeLists.txt)
  message(FATAL_ERROR "tinygettext submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygettext/CMakeLists.txt is missing")
ENDIF()

set(TINYGETTEXT_PREFIX ${CMAKE_BINARY_DIR}/tinygettext/)
ExternalProject_Add(tinygettext
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/tinygettext/"
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DBUILD_SHARED_LIBS=OFF
  -DHAVE_SDL=ON
  -DCMAKE_INSTALL_PREFIX=${TINYGETTEXT_PREFIX}
  -DSDL2_INCLUDE_DIRS=${SDL2_INCLUDE_DIRS}
  -DLIB_SUFFIX=${LIB_SUFFIX})

ADD_LIBRARY(tinygettext_lib STATIC IMPORTED)
SET_TARGET_PROPERTIES(tinygettext_lib PROPERTIES IMPORTED_LOCATION "${TINYGETTEXT_PREFIX}/lib${LIB_SUFFIX}/${CMAKE_STATIC_LIBRARY_PREFIX}tinygettext${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${TINYGETTEXT_PREFIX}/include)

## Some additional include paths

include_directories(${CMAKE_BINARY_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/findlocale/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/obstack/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/include/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_SavePNG/)

IF(WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES -DNOMINMAX)
  ADD_DEFINITIONS(-DWIN32)
ENDIF(WIN32)

## Build list of sources for supertux binary

FILE(GLOB SUPERTUX_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/obstack/*.c external/findlocale/findlocale.c)

FILE(GLOB SUPERTUX_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*/*.cpp src/supertux/menu/*.cpp src/video/sdl/*.cpp)
FILE(GLOB SUPERTUX_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt)
  message(FATAL_ERROR "sexp-cpp submodule is not checked out or ${CMAKE_CURRENT_SOURCE_DIR}/external/sexp-cpp/CMakeLists.txt is missing")
ENDIF()
FILE(GLOB SEXP_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/sexp-cpp/src/*.cpp)
ADD_LIBRARY(sexp STATIC ${SEXP_SOURCES_CXX})

file(GLOB SAVEPNG_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} external/SDL_SavePNG/savepng.c)
add_library(savepng STATIC ${SAVEPNG_SOURCES_CXX})
target_include_directories(savepng SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(savepng PUBLIC ${PNG_LIBRARIES})

IF(HAVE_OPENGL)
  FILE(GLOB SUPERTUX_OPENGL_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/video/gl/*.cpp)
  SET(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_OPENGL_SOURCES})
ENDIF(HAVE_OPENGL)

## Sort source lists to have deterministic linking order
LIST(SORT SUPERTUX_SOURCES_C)
LIST(SORT SUPERTUX_SOURCES_CXX)
LIST(SORT SUPERTUX_RESOURCES)

IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
  SET(SUPERTUX_SOURCES_CXX ${SUPERTUX_SOURCES_CXX} ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)
ENDIF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp)

## Compile everything at once (roughly equivalent to cat *.cpp | gcc)

OPTION(COMPILE_AMALGATION "Compile all the files together at once (experimental)" OFF)
IF(COMPILE_AMALGATION)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "// Includes all source files of the project\n")
  FOREACH(ST_FILE ${SUPERTUX_SOURCES_CXX})
    FILE(RELATIVE_PATH CXX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src "${CMAKE_CURRENT_SOURCE_DIR}/${ST_FILE}")
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp "#include \"${CXX_FILE}\"\n")
  ENDFOREACH(ST_FILE)
  ## Build instead of CXX sources
  SET(SUPERTUX_SOURCES_CXX ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
ELSE(COMPILE_AMALGATION)
  IF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
    FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
  ENDIF(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/amalgation.cpp)
ENDIF(COMPILE_AMALGATION)

## Debug options

OPTION(WERROR "Stop on first compiler warning" OFF)
OPTION(WARNINGS "Enable long list of warnings for compiler to check" OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  SET(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_C_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  SET(CMAKE_LD_FLAGS_PROFILE "-lgmon" CACHE STRING "Profile flags")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -funit-at-a-time")
  IF(WERROR)
    ADD_DEFINITIONS(-Werror)
  ELSE(WERROR)
    REMOVE_DEFINITIONS(-Werror)
  ENDIF(WERROR)
  IF(WARNINGS)
    # temporarily disabled:
    #   -Wsign-conversion -Wfloat-equal -Wundef -Wswitch-default
    #   -Wswitch-enum -Wsign-promo -Wcast-qual -Woverloaded-virtual -Wmissing-format-attribute
    #   -Wpadded -Wabi -Winline -Wunsafe-loop-optimizations -Wstrict-overflow=5
    # fails on MinGW:
    #   -ansi
    SET(SUPERTUX2_EXTRA_WARNING_FLAGS "-fdiagnostics-show-option -pedantic -Wno-long-long -Wcast-align -Wdisabled-optimization -Winit-self -Winvalid-pch -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wredundant-decls -Wstack-protector -Wformat=2 -Weffc++ -Wctor-dtor-privacy  -Wno-unused-parameter -Wshadow -Wnon-virtual-dtor -Wcast-qual -Wold-style-cast")
    if(CMAKE_COMPILER_IS_GNUCXX)
      SET(SUPERTUX2_EXTRA_WARNING_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS} -Wconversion -Wlogical-op -Wstrict-null-sentinel -Wint-in-bool-context")
    else()
      SET(SUPERTUX2_EXTRA_WARNING_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS} -Wconversion -Wno-sign-conversion")
    endif()
  ENDIF(WARNINGS)
endif()

## If xgettext is available, generate messages.pot for sources

OPTION(GENERATE_MESSAGESPOT "Generate messages.pot files" OFF)
IF(GENERATE_MESSAGESPOT)
  MARK_AS_ADVANCED(
    XGETTEXT_EXECUTABLE
  )
  FIND_PROGRAM(XGETTEXT_EXECUTABLE xgettext)
  IF(NOT XGETTEXT_EXECUTABLE)
    MESSAGE(STATUS "Warning: xgettext not found - will not update messages.pot")
  ELSE(NOT XGETTEXT_EXECUTABLE)
    MESSAGE(STATUS "Found xgettext: ${XGETTEXT_EXECUTABLE}")

    SET(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/data/locale/messages.pot)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${MESSAGES_POT_FILE}
      COMMAND ${XGETTEXT_EXECUTABLE}
      ARGS --keyword=_ --language=C++ --output=${MESSAGES_POT_FILE} ${SUPERTUX_SOURCES_CXX}
      DEPENDS ${SUPERTUX_SOURCES_CXX}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
    )
    LIST(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})

    FILE(GLOB SUPERTUX_LEVEL_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} data/levels/*)
    FOREACH(SUPERTUX_LEVEL_DIR ${SUPERTUX_LEVEL_DIRS})
      FILE(GLOB SUPERTUX_LEVELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SUPERTUX_LEVEL_DIR}/*.stl ${SUPERTUX_LEVEL_DIR}/*.stwm ${SUPERTUX_LEVEL_DIR}/*.txt)

      ## Do not add target if SUPERTUX_LEVELS is empty. This is needed for cmake <=2.4
      IF(SUPERTUX_LEVELS)
      SET(MESSAGES_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${SUPERTUX_LEVEL_DIR}/messages.pot)
      ADD_CUSTOM_COMMAND(
        OUTPUT ${MESSAGES_POT_FILE}
        COMMAND ${XGETTEXT_EXECUTABLE}
        ARGS --keyword="_:1" --language=Lisp --force-po --sort-by-file "--output=${MESSAGES_POT_FILE}" ${SUPERTUX_LEVELS}
        DEPENDS ${SUPERTUX_LEVELS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating POT file ${MESSAGES_POT_FILE}"
      )
      LIST(APPEND MESSAGES_POT_FILES ${MESSAGES_POT_FILE})
      ENDIF(SUPERTUX_LEVELS)

    ENDFOREACH(SUPERTUX_LEVEL_DIR)

    ADD_CUSTOM_TARGET(
      supertux2-messages ALL
      DEPENDS ${MESSAGES_POT_FILES}
    )

  ENDIF(NOT XGETTEXT_EXECUTABLE)
ENDIF(GENERATE_MESSAGESPOT)

## Some additional compiler switches
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ADD_DEFINITIONS(-DMACOSX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


## On Windows, add an icon

IF(WIN32)
#  SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
  IF(MINGW)
    ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons -i${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc -o ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
    SET(SUPERTUX_SOURCES_C ${SUPERTUX_SOURCES_C} ${CMAKE_CURRENT_BINARY_DIR}/supertux_rc.o)
  ENDIF(MINGW)
ENDIF(WIN32)

## Build miniswig and generate miniswig wrapper

OPTION(GENERATE_WRAPPER "Build miniswig and generate the wrapper" OFF)
IF(GENERATE_WRAPPER)
  ADD_SUBDIRECTORY(tools/miniswig)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_CXX_COMPILER}
    ARGS -x "c++" -E -CC -DSCRIPTING_API src/scripting/wrapper.interface.hpp -o ${CMAKE_CURRENT_BINARY_DIR}/miniswig.tmp -I${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMAND tools/miniswig/miniswig
    ARGS --input miniswig.tmp --output-cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.cpp --output-hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.hpp --module supertux --select-namespace scripting
    DEPENDS tools/miniswig/miniswig
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/src/scripting/wrapper.interface.hpp
  )
ENDIF(GENERATE_WRAPPER)

## Generate supertux executable in the right place

#SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

## Add target for supertux binary

add_library(supertux2_c OBJECT ${SUPERTUX_SOURCES_C})
add_library(supertux2_lib STATIC ${CMAKE_BINARY_DIR}/version.h ${SUPERTUX_SOURCES_CXX} ${SUPERTUX_RESOURCES} $<TARGET_OBJECTS:supertux2_c>)
IF(WIN32)
    add_executable(supertux2 WIN32 src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.rc)
    TARGET_LINK_LIBRARIES(supertux2 ${SDL2MAIN_LIBRARIES})
ELSE()
    add_executable(supertux2 src/main.cpp)
ENDIF(WIN32)
target_link_libraries(supertux2 supertux2_lib)
set_target_properties(supertux2 PROPERTIES OUTPUT_NAME supertux2)
set_target_properties(supertux2_lib PROPERTIES OUTPUT_NAME supertux2)
set_target_properties(supertux2_lib PROPERTIES COMPILE_FLAGS "${SUPERTUX2_EXTRA_WARNING_FLAGS}")

IF(WIN32)
  ## Copy dlls on windows
  ADD_CUSTOM_COMMAND(TARGET supertux2_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${DEPENDENCY_FOLDER}/dll"
      $<TARGET_FILE_DIR:supertux2_lib>)
#  ADD_CUSTOM_COMMAND(TARGET supertux2 POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#     "${DEPENDENCY_FOLDER}/DigiCertHighAssuranceEVRootCA.pem"
#     $<TARGET_FILE_DIR:supertux2_lib>)
ENDIF(WIN32)

## Link supertux binary with squirrel and other libraries

ADD_DEPENDENCIES(supertux2_lib tinygettext)
ADD_DEPENDENCIES(supertux2_lib squirrel)

TARGET_INCLUDE_DIRECTORIES(supertux2_lib SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(supertux2_lib SYSTEM PUBLIC ${SDL2IMAGE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${SDL2_LIBRARIES})
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${SDL2IMAGE_LIBRARIES})

TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC squirrel_lib)
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC sqstdlib_lib)
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC tinygettext_lib)
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC sexp)
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC savepng)
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${OPENAL_LIBRARY})
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${OGGVORBIS_LIBRARIES})
TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${Boost_LIBRARIES})
IF(USE_SYSTEM_PHYSFS)
    TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${PHYSFS_LIBRARY})
ELSE()
    ADD_DEPENDENCIES(supertux2_lib physfs)
    TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC physfs_lib)
    IF(APPLE)
        TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC "-framework CoreFoundation")
        TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC "-framework Foundation")
        TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC "-framework IOKit")
    ENDIF()
ENDIF()

IF(HAVE_OPENGL)
    TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${OPENGL_LIBRARY})
    IF(GLBINDING_FOUND)
      TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${GLBINDING_LIBRARIES})
    ELSE()
      TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${GLEW_LIBRARY})
    ENDIF()
ENDIF(HAVE_OPENGL)
IF(HAVE_LIBCURL)
  TARGET_LINK_LIBRARIES(supertux2_lib PUBLIC ${CURL_LIBRARY})
ENDIF(HAVE_LIBCURL)

if(BUILD_TESTS)
  find_package(Threads REQUIRED)

  # build gtest
  # ${CMAKE_CURRENT_SOURCE_DIR} in include_directories is needed to generate -isystem instead of -I flags
  add_library(gtest_main STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/src/gtest_main.cc)
  target_include_directories(gtest_main SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/include/)
  add_library(gtest STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/src/gtest-all.cc)
  target_include_directories(gtest SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/)
  target_include_directories(gtest SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/include/)

  # build SuperTux tests
  file(GLOB TEST_SUPERTUX_SOURCES tests/*.cpp)
  add_executable(test_supertux2 ${TEST_SUPERTUX_SOURCES})
  target_compile_options(test_supertux2 PRIVATE ${WARNINGS_CXX_FLAGS})
  target_link_libraries(test_supertux2
    gtest gtest_main
    supertux2_lib
    ${CMAKE_THREAD_LIBS_INIT})

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_supertux2
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_supertux2)
endif()

## Install stuff

OPTION(DISABLE_CPACK_BUNDLING "Build an .app bundle without CPack" OFF)
IF(WIN32 AND NOT UNIX)

  SET(INSTALL_SUBDIR_BIN "bin" CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "data" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "doc" CACHE STRING "Installation subdir for docs")

  INSTALL(FILES ${DLLS} DESTINATION ${INSTALL_SUBDIR_BIN})

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico DESTINATION ".")

  OPTION(PACKAGE_VCREDIST "Package the VCREDIST libraries with the program" OFF)

  IF(PACKAGE_VCREDIST)
    include(InstallRequiredSystemLibraries)
    INSTALL(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${INSTALL_SUBDIR_BIN})
    include(FindWindowsSDK)
    IF(${CMAKE_VS_PLATFORM_NAME} STREQUAL "Win32")
        SET(UCRT_PATH "${WINDOWSSDK_PREFERRED_DIR}/Redist/ucrt/DLLs/x86")
    ELSE()
        SET(UCRT_PATH "${WINDOWSSDK_PREFERRED_DIR}/Redist/ucrt/DLLs/x64")
    ENDIF()
    FILE(GLOB UCRT_DLLS "${UCRT_PATH}/*")
    INSTALL(FILES ${UCRT_DLLS} DESTINATION ${INSTALL_SUBDIR_BIN})
  ENDIF()

ELSE(WIN32 AND NOT UNIX)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

  SET(INFOPLIST_CFBUNDLEEXECUTABLE "supertux2")

  SET(INSTALL_SUBDIR_BIN "SuperTux.app/Contents/MacOS" CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "SuperTux.app/Contents/Resources/data" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "SuperTux.app/Contents/Resources" CACHE STRING "Installation subdir for docs")

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in ${CMAKE_BINARY_DIR}/tools/darwin/info.plist)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/tools/darwin/info.plist DESTINATION "SuperTux.app/Contents/")
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/receipt DESTINATION "SuperTux.app/Contents/_MASReceipt/")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns DESTINATION "SuperTux.app/Contents/Resources/")

ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)

  SET(INSTALL_SUBDIR_BIN "games" CACHE STRING "Installation subdir for binaries")
  SET(INSTALL_SUBDIR_SHARE "share/games/supertux2" CACHE STRING "Installation subdir for data")
  SET(INSTALL_SUBDIR_DOC "share/doc/supertux2" CACHE STRING "Installation subdir for docs")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.desktop DESTINATION "share/applications")

  SET(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${INSTALL_SUBDIR_BIN}/supertux2")

  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.png ${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.xpm DESTINATION "share/pixmaps/")
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.svg DESTINATION "share/icons/hicolor/scalable/apps")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND DISABLE_CPACK_BUNDLING)
ENDIF(WIN32 AND NOT UNIX)

INSTALL(TARGETS supertux2 DESTINATION ${INSTALL_SUBDIR_BIN})

IF(WIN32)
	GET_PROPERTY(SQUIRREL_LIB_PATH TARGET squirrel_lib PROPERTY IMPORTED_LOCATION)
	GET_PROPERTY(SQSTDLIB_LIB_PATH TARGET sqstdlib_lib PROPERTY IMPORTED_LOCATION)
	GET_PROPERTY(PHYSFS_LIB_PATH TARGET physfs_lib PROPERTY IMPORTED_LOCATION)
	INSTALL(FILES ${SQUIRREL_LIB_PATH} ${SQSTDLIB_LIB_PATH} ${PHYSFS_LIB_PATH} DESTINATION ${INSTALL_SUBDIR_BIN})
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL.md ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt ${CMAKE_CURRENT_SOURCE_DIR}/NEWS.md DESTINATION ${INSTALL_SUBDIR_DOC})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/credits.stxt DESTINATION ${INSTALL_SUBDIR_SHARE})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/supertux2.appdata.xml DESTINATION "share/metainfo" )

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/images
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/fonts
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/music
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/scripts
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/speech
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/sounds
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/locale
                  DESTINATION ${INSTALL_SUBDIR_SHARE})

IF(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
  INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
                    DESTINATION ${INSTALL_SUBDIR_SHARE}
                    PATTERN "data/levels/test" EXCLUDE
                    PATTERN "data/levels/test_old" EXCLUDE
                    PATTERN "data/levels/incubator" EXCLUDE
                    PATTERN "data/levels/misc/menu.stl.in" EXCLUDE)
ELSE()
  INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
                    DESTINATION ${INSTALL_SUBDIR_SHARE}
                    PATTERN "data/levels/misc/menu.stl.in" EXCLUDE)
ENDIF()

INSTALL(FILES "${CMAKE_BINARY_DIR}/data/levels/misc/menu.stl" DESTINATION "${INSTALL_SUBDIR_SHARE}/levels/misc")

## Create config.h now that INSTALL_SUBDIR_* have been set.

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

## Configure main menu logo
IF("${SUPERTUX_VERSION_STRING}" MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+$")
    SET(LOGO_FILE "logo_final.sprite")
ELSE()
    SET(LOGO_FILE "logo.sprite")
ENDIF()
configure_file(data/levels/misc/menu.stl.in ${CMAKE_BINARY_DIR}/data/levels/misc/menu.stl )


## CPack/Installation-specific stuff

INCLUDE(InstallRequiredSystemLibraries)
SET(DIRS ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/external/squirrel ${CMAKE_CURRENT_BINARY_DIR}/external/tinygettext)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT DISABLE_CPACK_BUNDLING)
    SET(INFOPLIST_CFBUNDLEEXECUTABLE "SuperTux")

    FIND_PACKAGE(png)
    foreach (_file ${PNG_LIBRARIES})
        get_filename_component(_resolvedFile "${_file}" REALPATH)
        get_filename_component(_name "${_file}" NAME)
	INSTALL(FILES ${_resolvedFile} DESTINATION "MacOS" RENAME ${_name})
    endforeach()
    FIND_PACKAGE(jpeg)
    foreach (_file ${JPEG_LIBRARIES})
        get_filename_component(_resolvedFile "${_file}" REALPATH)
        get_filename_component(_name "${_file}" NAME)
	INSTALL(FILES ${_resolvedFile} DESTINATION "MacOS" RENAME ${_name})
    endforeach()

    INSTALL(CODE "
       if(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/\" MATCHES \".*\\\\.app.*\")
       include(BundleUtilities)
       fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
       endif()
       ")

    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/info.plist.in" "${CMAKE_BINARY_DIR}/tools/darwin/info.plist")

    SET(CPACK_BUNDLE_NAME "SuperTux")
    SET(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/tools/darwin/info.plist")
    SET(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.icns")
    SET(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/startup.sh")
    SET(CPACK_DMG_VOLUME_NAME "SuperTux ${SUPERTUX_VERSION_STRING}")
    SET(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/dsstore_setup.scpt")
    SET(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/tools/darwin/background.png")
ENDIF()



SET(CPACK_PACKAGE_NAME "SuperTux")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jump'n'Run Game featuring Tux")
SET(CPACK_PACKAGE_VENDOR "SuperTux Devel Team")
SET(CPACK_PACKAGE_CONTACT "SuperTux Devel Team <supertux-devel@lists.lethargik.org>")
SET(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/;${CMAKE_BINARY_DIR};/\\\\..*")
SET(CPACK_DEBIAN_PACKAGE_NAME "supertux2")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.5), libgcc1 (>= 1:4.1), libgl1-mesa-glx | libgl1, libogg0 (>= 1.1.3), libopenal0a, libphysfs-1.0-0, libsdl-image1.2 (>= 1.2.5), libsdl1.2debian (>= 1.2.10-1), libstdc++6 (>= 4.1.2), libvorbis0a (>= 1.1.2), libvorbisfile3 (>= 1.1.2), libcurl3 (>= 7.16)")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_RPM_PACKAGE_NAME "supertux2")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux\n SuperTux is a classic 2D jump 'n run sidescroller game in a similar\n style like the original SuperMario games. This release of SuperTux\n features 9 enemies, 26 playable levels, software and OpenGL rendering\n modes, configurable joystick and keyboard input, new music and\n completely redone graphics.\n .\n This is a development snapshot of SuperTux. It may suffer from\n critical bugs and has not been fully tested. \n .\n Homepage: http://supertux.lethargik.org/")
SET(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License (GPL)")
SET(CPACK_RPM_PACKAGE_GROUP "Amusements/Games/Action/Arcade")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${SUPERTUX_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${SUPERTUX_VERSION_MINOR})

IF(SUPERTUX_VERSION_TWEAK)
    SET(CPACK_PACKAGE_VERSION_PATCH "${SUPERTUX_VERSION_PATCH}.${SUPERTUX_VERSION_TWEAK}")
ELSE()
    SET(CPACK_PACKAGE_VERSION_PATCH ${SUPERTUX_VERSION_PATCH})
ENDIF(SUPERTUX_VERSION_TWEAK)
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SUPERTUX_VERSION_STRING}-${SUPERTUX_SYSTEM_NAME}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SUPERTUX_VERSION_STRING}-Source")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux")
SET(CPACK_PACKAGE_EXECUTABLES "supertux2" "SuperTux")
IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "supertux2")
  SET(CPACK_NSIS_DISPLAY_NAME "SuperTux")
  SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
  SET(CPACK_WIX_PROGRAM_MENU_FOLDER "")
  SET(CPACK_WIX_UPGRADE_GUID "93E16F4E-0A68-422A-8ADC-47BE5B9433B2")
  SET(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/data/images/engine/icons/supertux.ico")

  SET(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/mk/wix/banner.png")
  SET(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/mk/wix/dialog.png")
  SET(CPACK_GENERATOR WIX)
ELSE(WIN32 AND NOT UNIX)
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)

# move some config clutter to the advanced section
MARK_AS_ADVANCED(
  INSTALL_SUBDIR_BIN
  INSTALL_SUBDIR_SHARE
  INSTALL_SUBDIR_DOC
)

MARK_AS_ADVANCED(
  CMAKE_BACKWARDS_COMPATIBILITY
  CMAKE_BUILD_TYPE
  CMAKE_INSTALL_PREFIX
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH
  CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_SYSROOT
)

MARK_AS_ADVANCED(
  APPDATADIR
)

MARK_AS_ADVANCED(
  Boost_INCLUDE_DIR
  Boost_LIBRARIES
)

MARK_AS_ADVANCED(
  GLEW_INCLUDE_DIR
  GLEW_LIBRARY
)

MARK_AS_ADVANCED(
  SDL2_INCLUDE_DIRS
  SDL2_LIBRARIES
)

MARK_AS_ADVANCED(
  SDL2IMAGE_INCLUDE_DIRS
  SDL2IMAGE_LIBRARIES
)

MARK_AS_ADVANCED(
  OPENAL_INCLUDE_DIR
  OPENAL_LIBRARY
)

MARK_AS_ADVANCED(
  OGG_LIBRARY
  VORBISENC_LIBRARY
  VORBISFILE_LIBRARY
  VORBIS_INCLUDE_DIR
  VORBIS_LIBRARY
)

MARK_AS_ADVANCED(
  PHYSFS_INCLUDE_DIR
  PHYSFS_LIBRARY
)
