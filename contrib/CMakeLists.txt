# Third Party Libraries
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(ExternalProject)

#EXTERNALPROJECT_ADD(00_ZLIB
#    URL ${OPENDCP_URL}/zlib-1.2.5.tar.gz
#    BUILD_IN_SOURCE 1
#    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG}
#                      ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
#                      --static
#    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
#    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
#)

IF(ENABLE_XMLSEC)
#EXTERNALPROJECT_ADD(00_LTDL
#    URL ${OPENDCP_URL}/libtool-2.4.tar.gz
#    BUILD_IN_SOURCE 1
#    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
#                       --disable-dependency-tracking --enable-static=yes --enable-shared=no 
#                       ${TARGET}
#    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
#    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
#)

IF(DL_ICONV)
    EXTERNALPROJECT_ADD(01_ICONV
        URL ${OPENDCP_URL}/libiconv-1.13.1.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                           --disable-dependency-tracking --enable-static=yes --enable-shared=no 
                           ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    )
ENDIF(DL_ICONV)

IF(DL_XML2)
    EXTERNALPROJECT_ADD(02_XML2
        URL ${OPENDCP_URL}/libxml2-2.7.8.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --with-iconv=${PREFIX}
                          ${TARGET}
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
    )
ENDIF(DL_XML2)

IF(DL_XSLT)
    EXTERNALPROJECT_ADD(03_XSLT
        URL ${OPENDCP_URL}/libxslt-1.1.26.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --with-libxml-prefix=${PREFIX}
                          --with-libxml-include-prefix=${PREFIX}/include/libxml2
                          --with-libxml-libs-prefix=${PREFIX}/lib
                          ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PREFIX}
    )
ENDIF(DL_XSLT)

IF(DL_XMLSEC)
    EXTERNALPROJECT_ADD(06_XMLSEC1
        URL ${OPENDCP_URL}/xmlsec1-1.2.16.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          --enable-crypto-dl=yes
                          --enable-apps-crypto-dl=yes
                          --enable-static-linking=yes
                          --with-libxml=${PREFIX}
                          --with-libxslt=${PREFIX}
                          --with-openssl=${PREFIX}
                          ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PREFIX}
    )
ENDIF(DL_XMLSEC)
ENDIF(ENABLE_XMLSEC)

IF(DL_EXPAT)
    EXTERNALPROJECT_ADD(04_EXPAT
        URL ${OPENDCP_URL}/expat-2.0.1.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no ${TARGET}
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PREFIX}
    )
ENDIF(DL_EXPAT)

IF(DL_OPENSSL)
    IF(APPLE)
        IF(FORCE_I386)
            SET(OPENSSL_CONFIGURE <SOURCE_DIR>/config ${PREFIX_ARG})
        ELSE()
            SET(OPENSSL_CONFIGURE <SOURCE_DIR>/Configure darwin64-x86_64-cc ${PREFIX_ARG})
        ENDIF()
    ELSE()
        SET(OPENSSL_CONFIGURE <SOURCE_DIR>/config ${PREFIX_ARG})
    ENDIF()

    EXTERNALPROJECT_ADD(05_OPENSSL
        URL ${OPENDCP_URL}/openssl-1.0.0c_opendcp.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${OPENSSL_CONFIGURE}
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        #BUILD_COMMAND make "CFLAG=-arch i386 -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk" -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    )
ENDIF(DL_OPENSSL)

IF(DL_ASDCPLIB)
    IF(PREFIX)
        SET(OPENSSL_PREFIX ${PREFIX})
        SET(EXPAT_PREFIX ${PREFIX})
    ELSE()
        SET(OPENSSL_PREFIX "yes")
        SET(EXPAT_PREFIX "yes")
    ENDIF()

    EXTERNALPROJECT_ADD(07_ASDCPLIB
        URL ${OPENDCP_URL}/asdcplib-1.7.40_opendcp.tar.gz
        #PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/asdcplib-1.7.40_configure.patch
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/asdcplib-1.7.40_opendcp/configure ${PREFIX_ARG}
                          ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking
                          --with-openssl=${OPENSSL_PREFIX}
                          --with-expat=${EXPAT_PREFIX}
                          --enable-static=yes --enable-shared=no
                          ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PREFIX}
    )
ENDIF(DL_ASDCPLIB)

IF(DL_TIFF)
    EXTERNALPROJECT_ADD(08_TIFF
        URL ${OPENDCP_URL}/tiff-3.9.4.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no
                          ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
        #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PREFIX}
    )
ENDIF(DL_TIFF)

IF(DL_OPENJPEG)
    IF(PREFIX)
        SET(OPENJPEG_LIBRARIES "${PREFIX}/lib/libopenjpeg.a")
    ELSE()
        SET(OPENJPEG_LIBRARIES "/usr/lib/openjpeg.a")
    ENDIF()

    EXTERNALPROJECT_ADD(09_OpenJPEG
        URL ${OPENDCP_URL}/openjpeg_v1_4_sources_r697.tar.gz
        PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4_configure.patch
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${PREFIX_ARG} ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                          --disable-dependency-tracking --enable-static=yes --enable-shared=no --enable-png=no --enable-tiff=no
                          ${TARGET} 
        BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
        INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    )
ENDIF(DL_OPENJPEG)
