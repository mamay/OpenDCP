cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Library Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--configure files--------------------------------------------------------------
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/opendcp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/opendcp.h)
#-------------------------------------------------------------------------------

#--set opendcplib source files--------------------------------------------------
SET(OPENDCP_SRC_FILES
     opendcp_j2k.c
     opendcp_xml.c
     opendcp_common.c
     opendcp_log.c
     asdcp_intf.cpp
     image/opendcp_tif.c
     image/opendcp_image.c
     image/opendcp_dpx.c
)

IF(ENABLE_XMLSEC)
    SET(OPENDCP_SEC_FILES opendcp_xml_sign.c)
ENDIF(ENABLE_XMLSEC)
#-------------------------------------------------------------------------------

#--set output targets and paths-------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/libopendcp/")
#-------------------------------------------------------------------------------

#--compile opendcplib-----------------------------------------------------------
IF(UNIX AND NOT APPLE)
    ADD_LIBRARY(opendcp-lib-shared SHARED ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
    SET_TARGET_PROPERTIES(opendcp-lib-shared PROPERTIES OUTPUT_NAME "opendcp")
    SET_TARGET_PROPERTIES(opendcp-lib-shared PROPERTIES PREFIX "lib")
    TARGET_LINK_LIBRARIES(opendcp-lib-shared ${ASDCP_LIBRARIES} ${LIBS})
    INSTALL(TARGETS opendcp-lib-shared DESTINATION ${LIB_INSTALL_PATH})
ELSE()
    ADD_LIBRARY(opendcp-lib-static STATIC ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
    SET_TARGET_PROPERTIES(opendcp-lib-static PROPERTIES OUTPUT_NAME "opendcp")
    SET_TARGET_PROPERTIES(opendcp-lib-static PROPERTIES PREFIX "lib")
    TARGET_LINK_LIBRARIES(opendcp-lib-static ${ASDCP_LIBRARIES} ${LIBS})
    INSTALL(TARGETS opendcp-lib-static DESTINATION ${LIB_INSTALL_PATH})
ENDIF()
#-------------------------------------------------------------------------------

#--install header---------------------------------------------------------------
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/opendcp.h DESTINATION include)
#-------------------------------------------------------------------------------
