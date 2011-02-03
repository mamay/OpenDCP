CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT (OpenDCP)
SET(CMAKE_BUILD_TYPE Release)

# Version
SET(OpenDCP_VERSION_MAJOR 0)
SET(OpenDCP_VERSION_MINOR 16)
SET(OpenDCP_VERSION "${OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}")

IF(APPLE)
    SET(OS "OS X")
    SET(CMAKE_C_FLAGS "${CMAKE_FLAGS} -arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
    SET(CMAKE_CXX_FLAGS "${CMAKE_FLAGS} -arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
    SET(AUTO_CFLAGS "CFLAGS=-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
    SET(AUTO_CXXFLAGS "CXXFLAGS=-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
    SET(AUTO_LDFLAGS "LDFLAGS=-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
ELSEIF(WIN32)
    SET(OS "Win32")
ELSE(UNIX)
    SET(OS "Linux")
ENDIF()

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "OpenDCP Version " ${OpenDCP_VERSION} " CMake - " ${OS})
MESSAGE(STATUS "-------------------------------------------------------------------------------")

# Include Path
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")

# Install Path
SET(OPENDCP_INSTALL_BIN_DIR "bin")

# Find Libraries
MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Finding Library Dependencies")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

# Build third party libraries
IF(STATIC)
    ADD_SUBDIRECTORY(contrib)
ENDIF(STATIC)

# Base Compiler Flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -D_FILE_OFFSET_BITS=64")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64")

# Windows specific compiler flags
IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DOPENMP /DWIN32 /DKM_WIN32 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DNDEBUG /EHsc /D_NDEBUG")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DOPENMP /DOPJ_STATIC /DWIN32 /DKM_WIN32 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /EHsc /DNDEBUG /D_NDEBUG")
    SET(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCPRT /NODEFAULTLIB:LIBCMT")
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
ENDIF(MSVC)

# Unix/OSX the following are standard, so always use dynamic 
IF(NOT MSVC)
    FIND_LIBRARY(M_LIB m)
    FIND_LIBRARY(Z_LIB z)
    FIND_LIBRARY(LTDL_LIB ltdl)
ENDIF(NOT MSVC)

# Use local static libraries 
IF(STATIC)
    MESSAGE(STATUS "Library Mode: Static")
    IF(APPLE)
        SET(LIB_DIR "${PROJECT_SOURCE_DIR}/build/contrib/lib")
        MESSAGE(STATUS "LIBS: ${LIB_DIR}")
    ELSEIF(WIN32)
        SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib/win32")
        MESSAGE(STATUS "LIBS: ${LIB_DIR}")
    ELSE(UNIX)
        SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib/linux")
        MESSAGE(STATUS "LIBS: ${LIB_DIR}")
    ENDIF(APPLE)
ELSE(STATIC)
    MESSAGE(STATUS "Library Mode: Dynamic")
    FIND_PACKAGE(ASDCP REQUIRED)
    FIND_PACKAGE(XMLSec REQUIRED)
    FIND_PACKAGE(OpenSSL REQUIRED)
    FIND_PACKAGE(EXPAT REQUIRED)
    FIND_PACKAGE(TIFF REQUIRED)
    FIND_PACKAGE(OpenJPEG REQUIRED)
    FIND_PACKAGE(LibXml2 REQUIRED)
ENDIF(STATIC)

# Compile opendcp source
ADD_SUBDIRECTORY(src)
