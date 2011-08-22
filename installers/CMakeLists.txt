cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Package Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--cpackage-------------------------------------------------------------------
SET(CPACK_PACKAGE_VERSION ${OPENDCP_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${OPENDCP_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${OPENDCP_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${OPENDCP_VERSION_BUILD})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenDCP Digital Cinema Package Tool")
SET(CPACK_PACKAGE_DESCRIPTION "This tool is used to created digital cinema packages (DCP)")
SET(CPACK_PACKAGE_VENDOR "OpenDCP")
#--debian package-------------------------------------------------------------
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_VERSION ${OPENDCP_VERSION})
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Terrence Meiczinger")
SET(CPACK_GENERATOR "RPM")
SET(CPACK_RPM_PACKAGE_REQUIRES "openssl >= 0.9.8,libxml2 >= 2.7,libxslt >= 1.1.26,xmlsec1 >= 1.2.16,xmlsec1-openssl >= 1.2.16,libtiff >= 3.9.4,qt >= 4.7.0")
SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
#-----------------------------------------------------------------------------
INCLUDE(CPack)
#-----------------------------------------------------------------------------
