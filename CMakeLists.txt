CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#--Build Type-----------------------------------------------------------------
PROJECT (OpenDCP)
SET(CMAKE_BUILD_TYPE Release)
#-----------------------------------------------------------------------------

#--Version--------------------------------------------------------------------
SET(OpenDCP_VERSION_MAJOR 0)
SET(OpenDCP_VERSION_MINOR 17)
SET(OpenDCP_VERSION "${OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}")
#-----------------------------------------------------------------------------

#--Options--------------------------------------------------------------------
OPTION(ENABLE_XMLSEC "Enable XML digital singatures and security features" OFF)
OPTION(ENABLE_STATIC "Enable static link to libraries" OFF)
OPTION(FORCE_I386    "Force i386 (mostly for OSX)" OFF)

IF(FORCE_I386)
    SET(ARCH "-arch i386")
    SET(TARGET "--target=i386")
ENDIF(FORCE_I386)

#-----------------------------------------------------------------------------

#--Set Base Compiler Flags----------------------------------------------------
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 -fopenmp")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 -fopenmp")
#-----------------------------------------------------------------------------

#--Set OS Specific Compiler Flags---------------------------------------------
IF(APPLE)
    SET(OS "OS X")
    IF(FORCE_I386)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(AUTO_CFLAGS "CFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(AUTO_CXXFLAGS "CXXFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(AUTO_LDFLAGS "LDFLAGS=${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
    ENDIF(FORCE_I386)
ELSEIF(WIN32)
    SET(OS "Win32")
    IF(MSVC)
        SET(CMAKE_CXX_FLAGS "/DOPENMP /DWIN32 /DKM_WIN32 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DNDEBUG /EHsc /D_NDEBUG")
        SET(CMAKE_C_FLAGS "/DOPENMP /DOPJ_STATIC /DWIN32 /DKM_WIN32 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /EHsc /DNDEBUG /D_NDEBUG")
        SET(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCPRT /NODEFAULTLIB:LIBCMT")
    ENDIF(MSVC)
ELSE(UNIX)
    SET(OS "Linux")
ENDIF()
#-----------------------------------------------------------------------------

#--Build Paths----------------------------------------------------------------
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
SET(OPENDCP_INSTALL_BIN_DIR "bin")
#-----------------------------------------------------------------------------

MESSAGE(STATUS)
MESSAGE(STATUS "===============================================================================")
MESSAGE(STATUS "OpenDCP Version " ${OpenDCP_VERSION} " CMake - " ${OS})
MESSAGE(STATUS "===============================================================================")

#--Find standard libaries-----------------------------------------------------
IF(NOT MSVC)
    #FIND_LIBRARY(M_LIB m REQUIRED)
    FIND_LIBRARY(Z_LIB z REQUIRED)
    FIND_LIBRARY(LTDL_LIB ltdl REQUIRED)
ENDIF(NOT MSVC)
#-----------------------------------------------------------------------------

#--Set linking mode-----------------------------------------------------------
IF(ENABLE_STATIC)
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    MESSAGE(STATUS "Find Static Library Dependencies")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    IF(MSVC)
        SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib/win32")
        MESSAGE(STATUS "LIBS: ${LIB_DIR}")
    ELSE(MSVC)
        ADD_SUBDIRECTORY(contrib)
        SET(LIB_DIR "${PROJECT_SOURCE_DIR}/build/contrib/lib")
        MESSAGE(STATUS "LIBS: ${LIB_DIR}")
    ENDIF(MSVC)
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
        ADD_DEFINITIONS(-DXMLSEC)
    ENDIF(ENABLE_XMLSEC)
    FIND_PACKAGE(EXPAT REQUIRED)
    FIND_PACKAGE(TIFF REQUIRED)
    FIND_PACKAGE(OpenJPEG REQUIRED)
ENDIF()
#-----------------------------------------------------------------------------

#--Add source directory-------------------------------------------------------
ADD_SUBDIRECTORY(src)
#-----------------------------------------------------------------------------
