cmake_minimum_required(VERSION 2.8)

#--Set Libraries--------------------------------------------------------------
IF(ENABLE_STATIC)
    #SET(OPENSSL -lssl -lcrypto)
    SET(OPENSSL -L${PREFIX}/ssl/lib -lssl -lcrypto)
    SET(ASDCP -L${LIB_DIR} -lasdcp -lkumu)
    SET(OPENJPEG ${LIB_DIR}/libopenjpeg.a ${LIB_DIR}/libtiff.a)
    SET(EXPAT ${LIB_DIR}/libexpat.a)
    IF(ENABLE_XMLSEC)
        SET(XML2 ${LIB_DIR}/libiconv.a ${LIB_DIR}/libxslt.a ${LIB_DIR}/libxml2.a)
        SET(XML_SEC ${XML2} ${LIB_DIR}/libxmlsec1.a ${LIB_DIR}/libxmlsec1-openssl.a -lltdl)
    ENDIF(ENABLE_XMLSEC)
    SET(STATIC_LIBRARIES ${ASDCP} ${OPENJPEG} ${OPENSSL} ${EXPAT} ${OPENMP} ${XML2} ${XML_SEC})
    SET(CONTRIB_LIBRARIES ${STATIC_LIBRARIES}) 
    SET(CONTRIB_LIBRARIES ${STATIC_LIBRARIES} ${M_LIB} ${Z_LIB}) 
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
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/bin/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/lib/")
SET(OPENDCP_LIB "${PROJECT_SOURCE_DIR}/build/lib/libopendcp.a")

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
