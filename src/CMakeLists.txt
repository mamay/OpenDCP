cmake_minimum_required(VERSION 2.8)

# Static Libraries
IF(STATIC)
    IF(WIN32)
        SET(OPENSSL ${LIB_DIR}/libeay32.lib)
        SET(ASDCP ${LIB_DIR}/libasdcp.lib ${LIB_DIR}/libkumu.lib)
        SET(OPENJPEG ${LIB_DIR}/openjpeg.lib ${LIB_DIR}/libtiff.lib)
        SET(LIBEXPAT ${LIB_DIR}/libexpat.lib)
        SET(OPENMP ${LIB_DIR}/vcomp.lib)
        SET(XML2 ${LIB_DIR}/libiconv.lib ${LIB_DIR}/libxslt.lib ${LIB_DIR}/libxml2.lib)
        SET(XML_SEC ${LIBXML_STATIC1} ${LIB_DIR}/libxmlsec-openssl.lib ${LIB_DIR}/libxmlsec.lib)
        SET(Z_LIB ${LIB_DIR}/zlib.lib)
        IF(NOT MSVC)
            SET(M_LIB lib/libm.a)
            SET(LTDL_LIB lib/ltdl.a)
        ENDIF(NOT MSVC)
    ELSE(WIN32)
        SET(OPENSSL ${LIB_DIR}/libssl.a ${LIB_DIR}/libcrypto.a)
        SET(ASDCP ${LIB_DIR}/libasdcp.a ${LIB_DIR}/libkumu.a)
        SET(OPENJPEG ${LIB_DIR}/libopenjpeg.a ${LIB_DIR}/libtiff.a)
        SET(EXPAT ${LIB_DIR}/libexpat.a)
        SET(XML2 ${LIB_DIR}/libiconv.a ${LIB_DIR}/libxslt.a ${LIB_DIR}/libxml2.a)
        SET(XML_SEC ${LIBXML_STATIC1} ${LIB_DIR}/libxmlsec1.a ${LIB_DIR}/libxmlsec1-openssl.a)
    ENDIF(WIN32)
    SET(STATIC_LIBRARIES ${ASDCP} ${OPENJPEG} ${OPENSSL} ${EXPAT} ${OPENMP} ${XML2} ${XML_SEC})
    SET(OPENDCP_LIBRARIES ${STATIC_LIBRARIES} ${M_LIB} ${Z_LIB} ${LTDL_LIB}) 
ELSE(STATIC)
    SET(OPENDCP_LIBRARIES ${OPENJPEG_LIBRARIES} ${ASDCP_LIBRARIES} ${OPENSSL_LIBRARIES} ${XMLSEC_LIBRARIES} ${LIBXML2_LIBRARIES} ${TIFF_LIBRARIES})
ENDIF(STATIC)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

SET(COMMON_SRC opendcp_utils.c opendcp_log.c asdcp_intf.cpp)
SET(IMG_SRC image/opendcp_tif.c image/opendcp_image.c)
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

ADD_EXECUTABLE(opendcp_xml opendcp_xml.c opendcp_xml_cmd.c opendcp_xml_sign.c ${COMMON_SRC})
TARGET_LINK_LIBRARIES(opendcp_xml ${OPENDCP_LIBRARIES})

ADD_EXECUTABLE(opendcp_xml_verify opendcp_xml_verify_cmd.c opendcp_xml_verify.c ${COMMON_SRC})
TARGET_LINK_LIBRARIES(opendcp_xml_verify ${OPENDCP_LIBRARIES})

ADD_EXECUTABLE(opendcp_j2k opendcp_j2k.c opendcp_j2k_cmd.c ${COMMON_SRC} ${IMG_SRC})
TARGET_LINK_LIBRARIES(opendcp_j2k ${OPENDCP_LIBRARIES})

ADD_EXECUTABLE(opendcp_mxf opendcp_mxf_cmd.c ${COMMON_SRC})
TARGET_LINK_LIBRARIES(opendcp_mxf ${OPENDCP_LIBRARIES})

INSTALL(TARGETS opendcp_xml opendcp_xml_verify opendcp_j2k opendcp_mxf 
  DESTINATION ${OPENDCP_INSTALL_BIN_DIR} COMPONENT Applications
)

ADD_LIBRARY(opendcp opendcp_j2k.c opendcp_xml_verify.c opendcp_mxf_cmd.c opendcp_xml.c ${COMMON_SRC} ${IMG_SRC})
TARGET_LINK_LIBRARIES(opendcp ${OPENDCP_LIBRARIES})
SET_TARGET_PROPERTIES(opendcp PROPERTIES VERSION "{OpenDCP_VERSION_MAJOR}.${OpenDCP_VERSION_MINOR}.0" SOVERSION 1)
INSTALL(TARGETS opendcp DESTINATION lib)
INSTALL(FILES opendcp.h DESTINATION include)
