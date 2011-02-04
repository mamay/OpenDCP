cmake_minimum_required(VERSION 2.8)

#--Set Libraries--------------------------------------------------------------
IF(ENABLE_STATIC)
    IF(WIN32)
        SET(OPENSSL ${LIB_DIR}/libeay32.lib)
        SET(ASDCP ${LIB_DIR}/libasdcp.lib ${LIB_DIR}/libkumu.lib)
        SET(OPENJPEG ${LIB_DIR}/openjpeg.lib ${LIB_DIR}/libtiff.lib)
        SET(LIBEXPAT ${LIB_DIR}/libexpat.lib)
        SET(OPENMP ${LIB_DIR}/vcomp.lib)
        SET(Z_LIB ${LIB_DIR}/zlib.lib)
        IF(ENABLE_XMLSEC)
            SET(XML2 ${LIB_DIR}/libiconv.lib ${LIB_DIR}/libxslt.lib ${LIB_DIR}/libxml2.lib)
            SET(XML_SEC ${XML2} ${LIB_DIR}/libxmlsec-openssl.lib ${LIB_DIR}/libxmlsec.lib)
        ENDIF(ENABLE_XMLSEC)
        IF(NOT MSVC)
            SET(M_LIB lib/libm.a)
            SET(LTDL_LIB lib/ltdl.a)
        ENDIF(NOT MSVC)
    ELSE(WIN32)
        SET(OPENSSL ${LIB_DIR}/libssl.a ${LIB_DIR}/libcrypto.a)
        SET(ASDCP ${LIB_DIR}/libasdcp.a ${LIB_DIR}/libkumu.a)
        SET(OPENJPEG ${LIB_DIR}/libopenjpeg.a ${LIB_DIR}/libtiff.a)
        SET(EXPAT ${LIB_DIR}/libexpat.a)
        IF(ENABLE_XMLSEC)
            SET(XML2 ${LIB_DIR}/libiconv.a ${LIB_DIR}/libxslt.a ${LIB_DIR}/libxml2.a)
            SET(XML_SEC ${XML2} ${LIB_DIR}/libxmlsec1.a ${LIB_DIR}/libxmlsec1-openssl.a)
        ENDIF(ENABLE_XMLSEC)
    ENDIF(WIN32)
    SET(STATIC_LIBRARIES ${ASDCP} ${OPENJPEG} ${OPENSSL} ${EXPAT} ${OPENMP} ${XML2} ${XML_SEC})
    SET(CONTRIB_LIBRARIES ${STATIC_LIBRARIES} ${M_LIB} ${Z_LIB} ${LTDL_LIB}) 
ELSE(ENABLE_STATIC)
    SET(CONTRIB_LIBRARIES ${OPENJPEG_LIBRARIES} ${ASDCP_LIBRARIES} ${OPENSSL_LIBRARIES} ${XMLSEC_LIBRARIES} ${LIBXML2_LIBRARIES} ${TIFF_LIBRARIES})
ENDIF(ENABLE_STATIC)
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
    SET(OPENCDP_SEC_FILES opendcp_xml_verify.c opendcp_xml_sign.c)
ENDIF(ENABLE_XMLSEC)
#-----------------------------------------------------------------------------
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/bin/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/lib/")
SET(OPENDCP_LIB "${PROJECT_SOURCE_DIR}/build/lib/libopendcp.a")

ADD_LIBRARY(opendcp ${OPENDCP_SEC_FILES} ${OPENDCP_SRC_FILES})
TARGET_LINK_LIBRARIES(opendcp ${OPENDCP_LIBRARIES})
SET_TARGET_PROPERTIES(opendcp PROPERTIES VERSION "{OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}.0" SOVERSION 1)
INSTALL(TARGETS opendcp DESTINATION lib)
INSTALL(FILES opendcp.h DESTINATION include)

ADD_EXECUTABLE(opendcp_xml opendcp_xml_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_xml ${CONTRIB_LIBRARIES} ${OPENDCP_LIB})

ADD_EXECUTABLE(opendcp_j2k opendcp_j2k_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_j2k ${CONTRIB_LIBRARIES} ${OPENDCP_LIB})

ADD_EXECUTABLE(opendcp_mxf opendcp_mxf_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_mxf ${CONTRIB_LIBRARIES} ${OPENDCP_LIB})

IF(ENABLE_XMLSEC)
    ADD_EXECUTABLE(opendcp_xml_verify opendcp_xml_verify_cmd.c opendcp_force_cpp.cpp)
    TARGET_LINK_LIBRARIES(opendcp_xml_verify ${CONTRIB_LIBRARIES} ${OPENDCP_LIB})
ENDIF(ENABLE_XMLSEC)

INSTALL(TARGETS ${OPENDCP_TARGETS} 
  DESTINATION ${OPENDCP_INSTALL_BIN_DIR} COMPONENT Applications
)
