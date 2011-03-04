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
OPTION(ENABLE_OSX386 "Force i386 and OSX10.5 compatibility" OFF)

IF(ENABLE_XMLSEC)
    ADD_DEFINITIONS(-DXMLSEC)
    ADD_DEFINITIONS(-DLIBXML_STATIC)
    ADD_DEFINITIONS(-DLIBXSLT_STATIC)
    ADD_DEFINITIONS(-DXMLSEC_STATIC)
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
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
        SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-search_paths_first")
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
        SET(LIBS ${LIBS} "-L${LIB_DIR} -lxmlsec1-openssl -lxmlsec1 -lxslt -lxml2 -liconv")
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

    #FIND_PACKAGE(JPEG REQUIRED)
    #INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
    #SET(LIBS ${LIBS} ${JPEG_LIBRARIES})

    FIND_PACKAGE(OpenJPEG REQUIRED)
    INCLUDE_DIRECTORIES(${OPENJPEG_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${OPENJPEG_LIBRARIES})
ENDIF()
#-----------------------------------------------------------------------------

#--Add source directory-------------------------------------------------------
ADD_SUBDIRECTORY(src)
#-----------------------------------------------------------------------------
