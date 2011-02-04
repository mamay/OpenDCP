# Third Party Libraries
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(ExternalProject)

SET(OPENDCP_URL "http://opendcp.org/opendcp_contrib")

#EXTERNALPROJECT_ADD(00_ZLIB
#    URL ${PROJECT_SOURCE_DIR}/contrib/zlib-1.2.5.tar.gz
#    BUILD_IN_SOURCE 1
#    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PROJECT_SOURCE_DIR}/build/contrib --static
#    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
#    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
#)

IF(ENABLE_XMLSEC)
EXTERNALPROJECT_ADD(01_ICONV
    URL ${OPENDCP_URL}/libiconv-1.13.1.tar.gz
    #URL ${PROJECT_SOURCE_DIR}/contrib/libiconv-1.13.1.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PROJECT_SOURCE_DIR}/build/contrib ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                       --disable-dependency-tracking --enable-static=yes --enable-shared=no 
                       target=i386 --build=i386 --host=i386
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(02_XML2
    URL ${PROJECT_SOURCE_DIR}/contrib/libxml2-2.7.8.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PROJECT_SOURCE_DIR}/build/contrib ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
                      --target=i386 --build=i386 --host=i386
                      --with-iconv=${PROJECT_SOURCE_DIR}/build/contrib
                      #LIBS=${PROJECT_SOURCE_DIR}/build/contrib/lib/libiconv.a
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(03_XSLT
    URL ${PROJECT_SOURCE_DIR}/contrib/libxslt-1.1.26.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix= ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
                      --with-libxml-prefix=${PROJECT_SOURCE_DIR}/build/contrib/
                      --with-libxml-include-prefix=${PROJECT_SOURCE_DIR}/build/contrib/include/libxml2
                      --with-libxml-libs-prefix=${PROJECT_SOURCE_DIR}/build/contrib/lib
                      --target=i386 --build=i386 --host=i386
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(06_XMLSEC1
    URL ${PROJECT_SOURCE_DIR}/contrib/xmlsec1-1.2.16.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix= ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
                      --with-libxml=${PROJECT_SOURCE_DIR}/build/contrib/
                      --with-libxslt=${PROJECT_SOURCE_DIR}/build/contrib/
                      --with-openssl=${PROJECT_SOURCE_DIR}/build/contrib/
                      --target=i386 --build=i386 --host=i386
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)
ENDIF(ENABLE_XMLSEC)

EXTERNALPROJECT_ADD(04_EXPAT
    URL ${PROJECT_SOURCE_DIR}/contrib/expat-2.0.1.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix= ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(05_OPENSSL
    URL ${PROJECT_SOURCE_DIR}/contrib/openssl-1.0.0c.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PROJECT_SOURCE_DIR}/build/contrib 
                      #CFLAGS=-isysroot /Developer/SDKs/MacOSX10.5.sdk
                      #CXXFLAGS=-isysroot /Developer/SDKs/MacOSX10.5.sdk
    #CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PROJECT_SOURCE_DIR}/build/contrib ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS} --disable-dependency-tracking 
    #CONFIGURE_COMMAND <SOURCE_DIR>/Configure darwin64-x86_64-cc --prefix=
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
    #INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(07_ASDCPLIB
    URL ${PROJECT_SOURCE_DIR}/contrib/asdcplib-1.7.40.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/asdcplib-1.7.40/configure --prefix= ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                       --disable-dependency-tracking --with-openssl --with-expat --enable-static=yes --enable-shared=no
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(08_TIFF
    URL ${PROJECT_SOURCE_DIR}/contrib/tiff-3.9.4.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix= ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
)

EXTERNALPROJECT_ADD(09_OpenJPEG
    URL ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4_sources_r697.tar.gz
    PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4_configure.patch
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PROJECT_SOURCE_DIR}/build/contrib ${AUTO_CFLAGS} ${AUTO_CXXFLAGS} ${AUTO_LDFLAGS}
                      --disable-dependency-tracking --enable-static=yes --enable-shared=no --enable-png=no --enable-tiff=no
                      --target=i386 --build=i386 --host=i386
    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install
)

#EXTERNALPROJECT_ADD(09_OpenJPEG
#    URL ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4_sources_r697.tar.gz
#    PATCH_COMMAND patch -t -N < ${PROJECT_SOURCE_DIR}/contrib/openjpeg_v1_4.patch
#    BUILD_IN_SOURCE 1
#    CONFIGURE_COMMAND cmake <SOURCE_DIR>/OpenJPEG 
#                      -DBUILD_SHARED_LIBS=OFF
#                      -DOPENJPEG_INSTALL_PACKAGE_DIR=${PROJECT_SOURCE_DIR}/build/contrib
#                      -DBUILD_CODEC=OFF
#                      -CMAKE_C_FLAGS= -arch i386
#                      -CMAKE_CXX_FLAGS= -arch i386
#                      <SOURCE_DIR> -DCMAKE_INSTALL_PREFIX=/
#    BUILD_COMMAND make -f <SOURCE_DIR>/Makefile
#    INSTALL_COMMAND make -f <SOURCE_DIR>/Makefile install DESTDIR=${PROJECT_SOURCE_DIR}/build/contrib
#)
