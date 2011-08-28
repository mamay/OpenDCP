cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Package Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--cpackage-------------------------------------------------------------------
SET(CPACK_PACKAGE_NAME ${PACKAGE_NAME})
SET(CPACK_PACKAGE_VERSION ${OPENDCP_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_BUILD})
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}.${ARCH}")
STRING(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenDCP Digital Cinema Package Tool")
SET(CPACK_PACKAGE_DESCRIPTION "This tool is used to created digital cinema packages (DCP)")
SET(CPACK_PACKAGE_VENDOR "OpenDCP")
#--debian package-------------------------------------------------------------
IF(MAKE_DEB)
    SET(CPACK_DEBIAN_PACKAGE_SECTION "Video")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Terrence Meiczinger <terrence72@gmail.com>")
    SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "This tool is used to created digital cinema packages (DCP)")
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl (>= 0.9.8),libxml2 (>= 2.7),libxslt1.1 (>= 1.1.26),xmlsec1 (>= 1.2),libxmlsec1-openssl (>= 1.2),libtiff4 (>= 3.9),libqt4-core (>= 4.6),libqt4-gui (>= 4.6)")
    SET(CPACK_GENERATOR "DEB")
ENDIF()
#--rpm package----------------------------------------------------------------
IF(MAKE_RPM)
    SET(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
    SET(CPACK_RPM_PACKAGE_DESCRIPTION "This tool is used to created digital cinema packages (DCP)")
    SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${ARCH})
    SET(CPACK_RPM_PACKAGE_REQUIRES "openssl >= 0.9.8,libxml2 >= 2.7,libxslt >= 1.1.26,xmlsec1 >= 1.2.16,xmlsec1-openssl >= 1.2.16,libtiff >= 3.9.4,qt >= 4.7.0")
    SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    SET(CPACK_GENERATOR "RPM")
ENDIF()
#-----------------------------------------------------------------------------
INCLUDE(CPack)
#-----------------------------------------------------------------------------
