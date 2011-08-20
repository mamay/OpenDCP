cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

#--set cli source files-------------------------------------------------------
SET(OPENDCP_TARGETS opendcp_xml opendcp_j2k opendcp_mxf)
IF(ENABLE_XMLSEC)
    SET(OPENDCP_TARGETS ${OPENDCP_TARGETS} opendcp_xml_verify)
    SET(OPENDCP_SEC_FILES opendcp_xml_verify.c opendcp_xml_sign.c)
ENDIF(ENABLE_XMLSEC)
#-----------------------------------------------------------------------------

#--set output targets and paths-----------------------------------------------
SET(OPENDCP_TARGETS opendcp_xml opendcp_j2k opendcp_mxf opendcp_largefile)
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin/")
SET(OPENDCP_LIB "${PROJECT_BINARY_DIR}/lib/libopendcp.a")
#-----------------------------------------------------------------------------

#--compile cli tools----------------------------------------------------------
ADD_EXECUTABLE(opendcp_largefile opendcp_largefile.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_largefile ${OPENDCP_LIB} ${LIBS})

ADD_EXECUTABLE(opendcp_xml opendcp_xml_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_xml ${OPENDCP_LIB} ${LIBS})

ADD_EXECUTABLE(opendcp_j2k opendcp_j2k_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_j2k ${OPENDCP_LIB} ${LIBS})

ADD_EXECUTABLE(opendcp_mxf opendcp_mxf_cmd.c opendcp_force_cpp.cpp)
TARGET_LINK_LIBRARIES(opendcp_mxf ${OPENDCP_LIB} ${LIBS})

IF(ENABLE_XMLSEC)
    ADD_EXECUTABLE(opendcp_xml_verify opendcp_xml_verify_cmd.c opendcp_force_cpp.cpp)
    TARGET_LINK_LIBRARIES(opendcp_xml_verify ${OPENDCP_LIB} ${LIBS})
ENDIF(ENABLE_XMLSEC)
#-----------------------------------------------------------------------------

#--install cli tools----------------------------------------------------------
SET_TARGET_PROPERTIES(${OPENDCP_TARGETS} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
INSTALL(TARGETS ${OPENDCP_TARGETS} 
  DESTINATION ${OPENDCP_INSTALL_BIN_DIR} COMPONENT Applications
)
#-----------------------------------------------------------------------------
