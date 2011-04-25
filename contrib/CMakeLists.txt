# Third Party Libraries
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(ExternalProject)

IF(MINGW)
    SET(CONFIGURE "mingw32-configure")
    SET(MAKE "mingw32-make")
    #SET(INSTALL_DIR DESTDIR=${PREFIX})
ELSE()
    SET(CONFIGURE "<SOURCE_DIR>/configure")
    SET(MAKE "make")
ENDIF()

#EXTERNALPROJECT_ADD(00_ZLIB
#    URL http://zlib.net/zlib-1.2.5.tar.gz 
#    BUILD_IN_SOURCE 1
#    PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/zlib-1.2.5.patch
#    CONFIGURE_COMMAND sh <SOURCE_DIR>/mkzlib.sh ${PREFIX} ${AUTO_CFLAGS} ${AUTO_LDFLAGS}
#    BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
#    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
#)

IF(ENABLE_XMLSEC)
    EXTERNALPROJECT_ADD(00_LTDL
        URL http://ftp.gnu.org/gnu/libtool/libtool-2.4.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no 
                           ${TARGET}
        BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
    )

    EXTERNALPROJECT_ADD(01_ICONV
        URL ${OPENDCP_URL}/libiconv-1.13.1.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                           --disable-dependency-tracking --enable-static=yes --enable-shared=no 
                           ${TARGET} 
        BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
    )

    EXTERNALPROJECT_ADD(02_XML2
        URL ${OPENDCP_URL}/libxml2-2.7.8.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --with-iconv=${PREFIX}
                          --with-threads=off
                          ${TARGET}
        BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
    )

    EXTERNALPROJECT_ADD(03_XSLT
        URL ${OPENDCP_URL}/libxslt-1.1.26.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --with-libxml-prefix=${PREFIX}
                          --with-libxml-include-prefix=${PREFIX}/include/libxml2
                          --with-libxml-libs-prefix=${PREFIX}/lib
                          ${TARGET} 
        BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
    )

    EXTERNALPROJECT_ADD(06_XMLSEC1
        URL ${OPENDCP_URL}/xmlsec1-1.2.16.tar.gz
        BUILD_IN_SOURCE 1
        PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/xmlsec1-1.1.26.patch
        CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --enable-pkgconfig=no
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --enable-crypto-dl=no
                          --enable-apps-crypto-dl=no
                          --enable-static-linking=yes
                          --with-libxml=${PREFIX}
                          --with-libxslt=${PREFIX}
                          --with-openssl=${PREFIX}
                          --with-default-crypto=openssl
                           ${TARGET} 
        BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
     )
ENDIF(ENABLE_XMLSEC)

EXTERNALPROJECT_ADD(04_EXPAT
    URL ${OPENDCP_URL}/expat-2.0.1.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no ${TARGET}
    BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
)

SET(OPENSSL_MAKE make)
IF(APPLE)
    IF(ENABLE_OSX386)
        SET(OPENSSL_CONFIGURE sh <SOURCE_DIR>/util/mkuniversal.sh ${PREFIX_ARG} configure)
        SET(OPENSSL_MAKE sh <SOURCE_DIR>/util/mkuniversal.sh ${PREFIX_ARG})
    ELSE()
        SET(OPENSSL_CONFIGURE <SOURCE_DIR>/Configure darwin64-x86_64-cc ${PREFIX_ARG})
    ENDIF()
ELSEIF(MINGW)
    SET(OPENSSL_CONFIGURE <SOURCE_DIR>/config mingw ${PREFIX_ARG})
ELSE()
    SET(OPENSSL_CONFIGURE <SOURCE_DIR>/config ${PREFIX_ARG})
ENDIF()

EXTERNALPROJECT_ADD(05_OPENSSL
    URL http://www.openssl.org/source/openssl-0.9.8r.tar.gz
    BUILD_IN_SOURCE 1
    PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/openssl-0.9.8r.patch
    CONFIGURE_COMMAND ${OPENSSL_CONFIGURE}
    BUILD_COMMAND ${OPENSSL_MAKE} -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
)

IF(PREFIX)
    SET(OPENSSL_PREFIX ${PREFIX})
    SET(EXPAT_PREFIX ${PREFIX})
ELSE()
    SET(OPENSSL_PREFIX "yes")
    SET(EXPAT_PREFIX "yes")
ENDIF()

EXTERNALPROJECT_ADD(07_ASDCPLIB
    URL ${OPENDCP_URL}/asdcplib-1.7.40.tar.gz
    PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/asdcplib-1.7.40.patch
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG}
                      ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking
                      --with-openssl=${PREFIX}
                      --with-expat=${EXPAT_PREFIX}
                      --enable-static=yes --enable-shared=no
                      ${TARGET} 
    BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
)

EXTERNALPROJECT_ADD(08_TIFF
    URL ${OPENDCP_URL}/tiff-3.9.4.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
                      --enable-jpeg=no
                      ${TARGET} 
    BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
)

EXTERNALPROJECT_ADD(09_OpenJPEG
    URL ${OPENDCP_URL}/openjpeg_v1_4_sources_r697.tar.gz
    PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4.patch
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE} ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no --enable-png=no --enable-tiff=no
                      ${TARGET} 
    BUILD_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND ${MAKE} -f <SOURCE_DIR>/Makefile install ${INSTALL_DIR}
)
