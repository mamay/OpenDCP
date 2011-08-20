CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#--Build Type-----------------------------------------------------------------
PROJECT (OpenDCP)
SET(CMAKE_BUILD_TYPE Release)
#-----------------------------------------------------------------------------

#--Version--------------------------------------------------------------------
SET(OpenDCP_VERSION_MAJOR 0)
SET(OpenDCP_VERSION_MINOR 20)
SET(OpenDCP_VERSION "${OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}")
ADD_DEFINITIONS(-DOPENDCP_VERSION="${OpenDCP_VERSION}")
#-----------------------------------------------------------------------------

#--Options--------------------------------------------------------------------
OPTION(ENABLE_XMLSEC "Enable XML digital singatures and security features" ON)
OPTION(ENABLE_OSX386 "Enable i386" OFF)
OPTION(ENABLE_OSX10.5 "Enable OSX 10.5 compatibility" OFF)
OPTION(ENABLE_OPENMP "Enable OPENMP multithreading" ON)
OPTION(ENABLE_STATIC "Enable static link to libraries" OFF)

IF(ENABLE_XMLSEC)
    ADD_DEFINITIONS(-DXMLSEC)
    ADD_DEFINITIONS(-DLIBXML_STATIC)
    ADD_DEFINITIONS(-DLIBXSLT_STATIC)
    ADD_DEFINITIONS(-DXMLSEC_STATIC)
ENDIF()

IF(ENABLE_OPENMP)
    ADD_DEFINITIONS(-DOPENMP)
    SET(OPENMP "-fopenmp")
ENDIF()
#-----------------------------------------------------------------------------

#--Set Base Compiler Flags----------------------------------------------------
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 ${OPENMP}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -D_FILE_OFFSET_BITS=64 ${OPENMP}")
#-----------------------------------------------------------------------------

#--Set OS Specific Compiler Flags---------------------------------------------
IF(APPLE)
    SET(OS "APPLE")
    IF(ENABLE_OSX386)
        SET(ARCH "-arch i386 -arch x86_64")
    ENDIF()
    IF(ENABLE_OSX10.5)
        SET(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
        SET(OSXMIN "-mmacosx-version-min=10.5")
        SET(OSXSDK "-isysroot /Developer/SDKs/MacOSX10.5.sdk")
    ENDIF()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OSXMIN}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OSXMIN}")
    SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-search_paths_first")
    SET(AUTO_CFLAGS "CFLAGS=${ARCH} ${OSXMIN} ${OSXSDK} -Wl,-search_paths_first")
    SET(AUTO_CXXFLAGS "CXXFLAGS=${ARCH} ${OSXMIN} ${OSXSDK} -Wl,-search_paths_first")
    SET(AUTO_LDFLAGS "LDFLAGS=-Wl,-search_paths_first")
ELSEIF(WIN32)
    SET(OS "WIN32")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPJ_STATIC -static") 
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPJ_STATIC -static")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
ELSE(UNIX)
    SET(OS "UNIX")
ENDIF()
#-----------------------------------------------------------------------------

#--Build Paths----------------------------------------------------------------
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libopendcp")
SET(OPENDCP_INSTALL_BIN_DIR "bin")
SET(OPENDCP_LIB "${PROJECT_BINARY_DIR}/libopendcp/libopendcp.a")
#-----------------------------------------------------------------------------

#--Contrib Paths--------------------------------------------------------------
SET(OPENDCP_URL "http://opendcp.googlecode.com/files")
#-----------------------------------------------------------------------------

MESSAGE(STATUS)
MESSAGE(STATUS "===============================================================================")
MESSAGE(STATUS "OpenDCP Version " ${OpenDCP_VERSION} " CMake - " ${OS})
MESSAGE(STATUS "===============================================================================")

#--Set linking mode-----------------------------------------------------------
IF(ENABLE_STATIC)
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    MESSAGE(STATUS "Find Static Library Dependencies")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")

    SET(PREFIX "${PROJECT_BINARY_DIR}/contrib")
    SET(PREFIX_ARG "--prefix=${PREFIX}")
    ADD_SUBDIRECTORY(contrib)

    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/libxml2")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/libxslt")
    INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/contrib/include/xmlsec1")

    SET(LIB_DIR "${PREFIX}/lib")
    MESSAGE(STATUS "LIB_DIR: ${LIB_DIR}")
    SET(LIBS ${LIBS} "-L${LIB_DIR} -lasdcp -lkumu" )
    SET(LIBS ${LIBS} "-L${LIB_DIR} -lopenjpeg -ltiff")
    SET(LIBS ${LIBS} "-L${LIB_DIR} -lexpat")
    IF(ENABLE_XMLSEC)
        SET(LIBS ${LIBS} "-L${LIB_DIR} -lxmlsec1-openssl -lxmlsec1 -lxslt -lxml2")
    ENDIF(ENABLE_XMLSEC)
    SET(LIBS ${LIBS} "-L${LIB_DIR} -lssl -lcrypto" )
    SET(LIBS ${LIBS} "-lz")
    IF (MINGW)
        SET(LIBS ${LIBS} "-lws2_32")
    ENDIF()
ELSE()
    MESSAGE(STATUS)
    MESSAGE(STATUS "-------------------------------------------------------------------------------")
    MESSAGE(STATUS "Finding Dynamic Library Dependencies")
    MESSAGE(STATUS "-------------------------------------------------------------------------------")

    SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/")

    FIND_PACKAGE(ZLIB REQUIRED)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${ZLIB_LIBRARIES})

    FIND_PACKAGE(ASDCP REQUIRED)
    INCLUDE_DIRECTORIES(${ASDCP_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${ASDCP_LIBRARIES})

    FIND_PACKAGE(OpenSSL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${OPENSSL_LIBRARIES})

    IF(ENABLE_XMLSEC)
        FIND_PACKAGE(LibXml2 REQUIRED)
        INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
        SET(LIBS ${LIBS} ${LIBXML2_LIBRARIES})

        FIND_PACKAGE(LibXslt REQUIRED)
        INCLUDE_DIRECTORIES(${LIBXSLT_INCLUDE_DIR})
        SET(LIBS ${LIBS} ${LIBXSLT_LIBRARIES})

        FIND_PACKAGE(XMLSec REQUIRED)
        INCLUDE_DIRECTORIES(${XMLSEC1_INCLUDE_DIR})
        INCLUDE_DIRECTORIES(${XMLSEC1-OPENSSL_INCLUDE_DIR})
        SET(LIBS ${LIBS} ${XMLSEC1_LIBRARIES})
        SET(LIBS ${LIBS} ${XMLSEC1-OPENSSL_LIBRARIES})
    ENDIF(ENABLE_XMLSEC)

    FIND_PACKAGE(EXPAT REQUIRED)
    INCLUDE_DIRECTORIES(${XMLSEC1_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${EXPAT_LIBRARIES})

    FIND_PACKAGE(TIFF REQUIRED)
    INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${TIFF_LIBRARIES})

    FIND_PACKAGE(OpenJPEG REQUIRED)
    INCLUDE_DIRECTORIES(${OPENJPEG_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${OPENJPEG_LIBRARIES})
ENDIF()
#-----------------------------------------------------------------------------

#--Add source directory-------------------------------------------------------
ADD_SUBDIRECTORY(libopendcp)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(gui)
#-----------------------------------------------------------------------------
