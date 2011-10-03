CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#--build Type-----------------------------------------------------------------
PROJECT(OpenDCP)
#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)
STRING(TOLOWER ${PROJECT_NAME} PACKAGE_NAME)
#-----------------------------------------------------------------------------

#--version--------------------------------------------------------------------
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 0)
SET(VERSION_BUILD 21)
SET(OPENDCP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD})
ADD_DEFINITIONS(-DOPENDCP_VERSION="${OPENDCP_VERSION}")
#-----------------------------------------------------------------------------

#--check 32 or 64-bit---------------------------------------------------------
IF(UNIX)
    IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
        SET(ARCH x86_64)
    ELSE()
        SET(ARCH i386)
    ENDIF()
ENDIF()
#-----------------------------------------------------------------------------

#--options--------------------------------------------------------------------
OPTION(ENABLE_XMLSEC  "Enable XML digital singatures and security features" ON)
OPTION(ENABLE_i386    "Enable i386" OFF)
OPTION(ENABLE_OSX10.5 "Enable OSX 10.5 compatibility" OFF)
OPTION(ENABLE_OPENMP  "Enable OPENMP multithreading" ON)
OPTION(ENABLE_GUI     "Enable GUI compiling" OFF)
OPTION(MAKE_RPM       "Create RPM package" OFF)
OPTION(MAKE_DEB       "Create DEB package" OFF)

IF(ENABLE_XMLSEC)
    ADD_DEFINITIONS(-DXMLSEC)
    ADD_DEFINITIONS(-DLIBXML_STATIC)
    ADD_DEFINITIONS(-DLIBXSLT_STATIC)
    ADD_DEFINITIONS(-DXMLSEC_STATIC)
ENDIF()

IF(ENABLE_OPENMP)
    ADD_DEFINITIONS(-DOPENMP)
    SET(OPENMP -fopenmp)
ENDIF()

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
#-----------------------------------------------------------------------------

#--cmake options--------------------------------------------------------------
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#-----------------------------------------------------------------------------

#--set Base Compiler Flags----------------------------------------------------
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ${OPENMP}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ${OPENMP}")
#SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O3 -D_FILE_OFFSET_BITS=64 ${OPENMP})
#SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -O3 -D_FILE_OFFSET_BITS=64 ${OPENMP})
#-----------------------------------------------------------------------------

#--set OS Specific Compiler Flags---------------------------------------------
IF(APPLE)
    SET(OS APPLE)
    IF(ENABLE_i386)
        SET(ARCH_FLAG "-arch i386 -arch x86_64")
    ENDIF()
    IF(ENABLE_OSX10.5)
        SET(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
        SET(OSXMIN -mmacosx-version-min=10.5)
        SET(OSXSDK -isysroot /Developer/SDKs/MacOSX10.5.sdk)
    ENDIF()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OSXMIN} -O3 -mfpmath=sse -mtune=native")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OSXMIN}")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-search_paths_first")
    SET(AUTO_CFLAGS "CFLAGS=${ARCH_FLAG} ${OSXMIN} ${OSXSDK} -Wl,-search_paths_first -O3 -mfpmath=sse -mtune=native")
    SET(AUTO_CXXFLAGS "CXXFLAGS=${ARCH_FLAG} ${OSXMIN} ${OSXSDK} -Wl,-search_paths_first")
    SET(AUTO_LDFLAGS "LDFLAGS=-Wl,-search_paths_first")
ELSEIF(WIN32)
    SET(OS WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPJ_STATIC -DKM_WIN32 -static") 
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPJ_STATIC -DKM_WIN32 -static")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
ELSE(UNIX)
    SET(OS UNIX)
    IF(ARCH STREQUAL "x86_64")
        SET(AUTO_CFLAGS "CFLAGS=-fPIC -O3")
    ENDIF()
ENDIF()
#-----------------------------------------------------------------------------

#--build Paths----------------------------------------------------------------
SET(LIB_INSTALL_PATH "lib")

SET(OPENDCP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libopendcp)
INCLUDE_DIRECTORIES(${OPENDCP_INCLUDE_DIR})
SET(OPENDCP_LIB ${PROJECT_BINARY_DIR}/libopendcp/libopendcp.a)

SET(ASDCP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libasdcp)
INCLUDE_DIRECTORIES(${ASDCP_INCLUDE_DIR})
SET(ASDCP_LIBRARIES -L${PROJECT_BINARY_DIR}/libasdcp -lopendcp-asdcp -lopendcp-kumu)

IF(UNIX AND NOT APPLE AND ARCH STREQUAL "x86_64")
    SET(LIB_INSTALL_PATH "lib64")
ENDIF()
#-----------------------------------------------------------------------------

#--contrib Paths--------------------------------------------------------------
SET(OPENDCP_URL http://opendcp.googlecode.com/files)
SET(PREFIX ${PROJECT_BINARY_DIR}/contrib)
SET(PREFIX_ARG --prefix=${PREFIX})
#-----------------------------------------------------------------------------

MESSAGE(STATUS)
MESSAGE(STATUS "===============================================================================")
MESSAGE(STATUS "OpenDCP Version ${OPENDCP_VERSION} CMake - ${OS} (${ARCH})")
MESSAGE(STATUS "===============================================================================")

#--set os specifc linking mode------------------------------------------------
IF(APPLE)
    #-- Apple includes some old versions of certain libraries, so we want to compile the
    #-- recent versions of those. In other cases, we can use dynamic libraries.
    INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/contrib/include)
    SET(LIB_DIR ${PREFIX}/lib)

    SET(COMPILE_ZLIB 0)
    SET(LIBS ${LIBS} -lz)
    
    SET(COMPILE_OPENSSL 1)
    SET(LIBS ${LIBS} -L${LIB_DIR} -lssl -lcrypto)

    IF(ENABLE_XMLSEC)
        INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/contrib/include/libxml2)
        INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/contrib/include/libxslt)
        INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/contrib/include/xmlsec1)
        SET(COMPILE_XMLSEC 1)
        SET(LIBS ${LIBS} -L${LIB_DIR} -lxmlsec1-openssl -lxmlsec1 -lxslt -lxml2)
    ENDIF(ENABLE_XMLSEC)

    SET(COMPILE_EXPAT 1)
    SET(LIBS ${LIBS} -L${LIB_DIR} -lexpat)
   
    SET(COMPILE_TIFF 1)
    SET(LIBS ${LIBS} -L${LIB_DIR} -ltiff)

    SET(COMPILE_OPENJPEG 1)
    SET(LIBS ${LIBS} -L${LIB_DIR} -lopenjpeg)

    ADD_SUBDIRECTORY(contrib)
ELSEIF(WIN32)
    #-- Win32 Compiling
    SET(SYSROOT_PATH /usr/i686-pc-mingw32/sys-root/mingw)
    SET(LIB_DIR ${SYSROOT_PATH}/lib)

    IF(ENABLE_XMLSEC)
        INCLUDE_DIRECTORIES(${SYSROOT_PATH}/include/libxml2)
        INCLUDE_DIRECTORIES(${SYSROOT_PATH}/include/libxslt)
        INCLUDE_DIRECTORIES(${SYSROOT_PATH}/include/xmlsec1)
        SET(COMPILE_XMLSEC 1)
        SET(LIBS ${LIBS} ${LIB_DIR}/libxmlsec1-openssl.a ${LIB_DIR}/libxmlsec1.a)
        SET(LIBS ${LIBS} ${LIB_DIR}/libxslt.a ${LIB_DIR}/libxml2.a)
    ENDIF(ENABLE_XMLSEC)

    SET(COMPILE_EXPAT 1)
    SET(LIBS ${LIBS} ${LIB_DIR}/libexpat.a)

    SET(COMPILE_OPENJPEG 1)
    SET(LIBS ${LIBS} ${LIB_DIR}/libopenjpeg.a)

    SET(LIBS ${LIBS} ${LIB_DIR}/libtiff.a ${LIB_DIR}/libjpeg.a)
    SET(LIBS ${LIBS} ${LIB_DIR}/libcrypto.a ${LIB_DIR}/libssl.a ${LIB_DIR}/libz.a)
    SET(LIBS ${LIBS} -L${LIB_DIR} -lz -lws2_32)

    ADD_SUBDIRECTORY(contrib)
ELSEIF(UNIX)
    SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

    FIND_PACKAGE(ZLIB REQUIRED)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${ZLIB_LIBRARIES})

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

    #-- download and build openjpeg local because the versions
    #-- between linux distributions are not compatible
    SET(COMPILE_OPENJPEG 1)
    INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/contrib/include)
    SET(LIBS ${LIBS} ${PREFIX}/lib/libopenjpeg.a)

    #FIND_PACKAGE(OpenJPEG REQUIRED)
    #INCLUDE_DIRECTORIES(${OPENJPEG_INCLUDE_DIR})
    #SET(LIBS ${LIBS} ${OPENJPEG_LIBRARIES})

    ADD_SUBDIRECTORY(contrib)
ENDIF()
#-----------------------------------------------------------------------------

#--add source directories-----------------------------------------------------
ADD_SUBDIRECTORY(libasdcp)
ADD_SUBDIRECTORY(libopendcp)
ADD_SUBDIRECTORY(cli)
IF(ENABLE_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF()
#-----------------------------------------------------------------------------

#--build packages-------------------------------------------------------------
ADD_SUBDIRECTORY(packages)
#-----------------------------------------------------------------------------
