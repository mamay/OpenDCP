CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#--Build Type-----------------------------------------------------------------
PROJECT (OpenDCP)
SET(CMAKE_BUILD_TYPE Release)
#-----------------------------------------------------------------------------

#--Version--------------------------------------------------------------------
SET(OpenDCP_VERSION_MAJOR 0)
SET(OpenDCP_VERSION_MINOR 17)
SET(OpenDCP_VERSION "${OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}")
ADD_DEFINITIONS(-DOPENDCP_VERSION="${OpenDCP_VERSION}")
#-----------------------------------------------------------------------------

#--Options--------------------------------------------------------------------
OPTION(ENABLE_XMLSEC "Enable XML digital singatures and security features" OFF)
OPTION(ENABLE_STATIC "Enable static link to libraries" OFF)
OPTION(ENABLE_OSX386 "Force i386 and OSX10.5 compatibility" ON)

IF(ENABLE_XMLSEC)
    ADD_DEFINITIONS(-DXMLSEC)
ENDIF()
#-----------------------------------------------------------------------------

#--Set Base Compiler Flags----------------------------------------------------
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 -fopenmp")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 -fopenmp")
SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
#-----------------------------------------------------------------------------

#--Set OS Specific Compiler Flags---------------------------------------------
IF(APPLE)
    SET(OS "APPLE")
    IF(ENABLE_OSX386)
        SET(ARCH "-arch i386")
        SET(TARGET "--target=i386")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-search_paths_first")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-search_paths_first")
        SET(AUTO_CFLAGS "CFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(AUTO_CXXFLAGS "CXXFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(AUTO_LDFLAGS "LDFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-search_paths_first")
    ENDIF()
ELSEIF(WIN32)
    SET(OS "WIN32")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPJ_STATIC") 
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPJ_STATIC")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
ELSE(UNIX)
    SET(OS "UNIX")
ENDIF()
#-----------------------------------------------------------------------------

#--Build Paths----------------------------------------------------------------
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
SET(OPENDCP_INSTALL_BIN_DIR "bin")
#-----------------------------------------------------------------------------

#--Contrib Paths--------------------------------------------------------------
SET(OPENDCP_URL "http://opendcp.googlecode.com/files")
#-----------------------------------------------------------------------------

MESSAGE(STATUS)
MESSAGE(STATUS "===============================================================================")
MESSAGE(STATUS "OpenDCP Version " ${OpenDCP_VERSION} " CMake - " ${OS})
MESSAGE(STATUS "===============================================================================")

#--Find standard libaries-----------------------------------------------------
FIND_LIBRARY(Z_LIB z REQUIRED)
#-----------------------------------------------------------------------------

#--Set linking mode-----------------------------------------------------------
IF(ENABLE_STATIC)
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/libxml2")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/libxslt")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/xmlsec1")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    MESSAGE(STATUS "Find Static Library Dependencies")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    SET(PREFIX "${PROJECT_BINARY_DIR}/contrib")
    SET(PREFIX_ARG "--prefix=${PREFIX}")
    ADD_SUBDIRECTORY(contrib)
    SET(LIB_DIR "${PROJECT_BINARY_DIR}/contrib/lib")
    MESSAGE(STATUS "LIB_DIR: ${LIB_DIR}")
ELSE()
    MESSAGE(STATUS)
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    MESSAGE(STATUS "Finding Dynamic Library Dependencies")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/")
    FIND_PACKAGE(ASDCP REQUIRED)
    FIND_PACKAGE(OpenSSL REQUIRED)
    IF(ENABLE_XMLSEC)
        FIND_PACKAGE(XMLSec REQUIRED)
        FIND_PACKAGE(LibXml2 REQUIRED)
    ENDIF(ENABLE_XMLSEC)
    FIND_PACKAGE(EXPAT REQUIRED)
    FIND_PACKAGE(TIFF REQUIRED)
    FIND_PACKAGE(JPEG REQUIRED)
    FIND_PACKAGE(OpenJPEG REQUIRED)
ENDIF()
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LIBXSLT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${XMLSEC1_INCLUDE_DIR})
INCLUDE_DIRECTORIES("/usr/include/xmlsec1")
#--Add source directory-------------------------------------------------------
ADD_SUBDIRECTORY(src)
#-----------------------------------------------------------------------------
