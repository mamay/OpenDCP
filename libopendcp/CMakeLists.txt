cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Library Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--Set opendcplib source files------------------------------------------------
SET(OPENDCP_SRC_FILES opendcp_j2k.c opendcp_xml.c opendcp_common.c opendcp_log.c asdcp_intf.cpp image/opendcp_tif.c image/opendcp_image.c image/opendcp_dpx.c)
IF(ENABLE_XMLSEC)
    SET(OPENDCP_SEC_FILES opendcp_xml_verify.c opendcp_xml_sign.c)
ENDIF(ENABLE_XMLSEC)
#-----------------------------------------------------------------------------

#--Set output targets and paths-----------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/libopendcp/")
#-----------------------------------------------------------------------------

#--compile opendcplib---------------------------------------------------------
#ADD_LIBRARY(opendcp-shared SHARED ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
ADD_LIBRARY(opendcp-static STATIC ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
SET_TARGET_PROPERTIES(opendcp-static PROPERTIES OUTPUT_NAME "opendcp")
SET_TARGET_PROPERTIES(opendcp-static PROPERTIES PREFIX "lib")
TARGET_LINK_LIBRARIES(opendcp-static ${LIBS})
INSTALL(TARGETS opendcp-static DESTINATION lib)
INSTALL(FILES opendcp.h DESTINATION include)
#-----------------------------------------------------------------------------
