cmake_minimum_required(VERSION 2.8)

#--Set Libraries--------------------------------------------------------------
IF(ENABLE_STATIC)
    SET(OPENSSL -L${LIB_DIR} -lssl -lcrypto)
    SET(ASDCP -L${LIB_DIR} -lasdcp -lkumu)
    SET(OPENJPEG -L${LIB_DIR} -lopenjpeg -ltiff)
    SET(EXPAT -L${LIB_DIR} -lexpat)
    IF(ENABLE_XMLSEC)
        SET(XML_SEC -L${LIB_DIR} -liconv -lxml2 -lxslt -lxmlsec1 -lxmlsec1-openssl)
    ENDIF(ENABLE_XMLSEC)
    SET(CONTRIB_LIBRARIES ${ASDCP} ${OPENJPEG} ${OPENSSL} ${EXPAT} ${XML_SEC} ${Z_LIB})
ELSE()
    SET(CONTRIB_LIBRARIES ${OPENJPEG_LIBRARIES} ${ASDCP_LIBRARIES} ${OPENSSL_LIBRARIES} ${XMLSEC_LIBRARIES} ${LIBXML2_LIBRARIES} ${TIFF_LIBRARIES} ${EXPAT_LIBRARIES})
ENDIF()
#-----------------------------------------------------------------------------

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--Set opendcplib source files------------------------------------------------
SET(OPENDCP_TARGETS opendcp_xml opendcp_j2k opendcp_mxf)
SET(OPENDCP_SRC_FILES opendcp_j2k.c opendcp_xml.c  opendcp_utils.c opendcp_log.c asdcp_intf.cpp image/opendcp_tif.c image/opendcp_image.c)
IF(ENABLE_XMLSEC)
    SET(OPENDCP_TARGETS ${OPENDCP_TARGETS} opendcp_xml_verify)
    SET(OPENDCP_SEC_FILES opendcp_xml_verify.c opendcp_xml_sign.c)
ENDIF(ENABLE_XMLSEC)
#-----------------------------------------------------------------------------
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib/")
SET(OPENDCP_LIB "${PROJECT_BINARY_DIR}/lib/libopendcp.a")

ADD_LIBRARY(opendcp STATIC ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
TARGET_LINK_LIBRARIES(opendcp ${CONTRIB_LIBRARIES})
INSTALL(TARGETS opendcp DESTINATION lib)
INSTALL(FILES opendcp.h DESTINATION include)

ADD_EXECUTABLE(opendcp_xml opendcp_xml_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_xml ${OPENDCP_LIB} ${CONTRIB_LIBRARIES})

ADD_EXECUTABLE(opendcp_j2k opendcp_j2k_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_j2k ${OPENDCP_LIB} ${CONTRIB_LIBRARIES})

ADD_EXECUTABLE(opendcp_mxf opendcp_mxf_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_mxf ${OPENDCP_LIB} ${CONTRIB_LIBRARIES})

IF(ENABLE_XMLSEC)
    ADD_EXECUTABLE(opendcp_xml_verify opendcp_xml_verify_cmd.c opendcp_force_cpp.cpp)
    TARGET_LINK_LIBRARIES(opendcp_xml_verify ${OPENDCP_LIB} ${CONTRIB_LIBRARIES})
ENDIF(ENABLE_XMLSEC)

INSTALL(TARGETS ${OPENDCP_TARGETS} 
  DESTINATION ${OPENDCP_INSTALL_BIN_DIR} COMPONENT Applications
)
