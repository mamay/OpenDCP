cmake_minimum_required(VERSION 2.8)

MESSAGE(STATUS)
MESSAGE(STATUS "-------------------------------------------------------------------------------")
MESSAGE(STATUS "Creating GUI Build Files")
MESSAGE(STATUS "-------------------------------------------------------------------------------")

IF(CMAKE_COMPILER_IS_GNUCXX)
  ADD_DEFINITIONS(-g)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#--find qt4-------------------------------------------------------------
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
#-----------------------------------------------------------------------------

#--set application icon and information---------------------------------------
IF(APPLE)
    SET(PROGNAME OpenDCP)
    SET(MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/opendcp.icns)
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${OpenDCP_VERSION})
    SET(MACOSX_BUNDLE_VERSION ${OpenDCP_VERSION})
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${OpenDCP_VERSION})
    IF(ENABLE_OSX386)
        SET(CMAKE_OSX_ARCHITECTURES i386;x86_64)
    ELSE()
        SET(CMAKE_OSX_ARCHITECTURES x86_64)
    ENDIF()
ELSEIF(UNIX)
    SET(PROGNAME opendcp)

    IF(NOT DESKTOP_ENTRY)
        SET(DESKTOP_ENTRY opendcp.desktop)
    ENDIF(NOT DESKTOP_ENTRY)
  
    ADD_CUSTOM_COMMAND(OUTPUT ${DESKTOP_ENTRY}
        COMMAND touch ${DESKTOP_ENTRY}
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/opendcp-desktop.sh ${CMAKE_INSTALL_PREFIX} >${DESKTOP_ENTRY}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/opendcp-desktop.sh
        COMMENT "Generating desktop entry file"
    )

    ADD_CUSTOM_TARGET(DESKTOP_ENTRY_FILE ALL
        DEPENDS ${DESKTOP_ENTRY}
    )
  
    SET(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/opendcp.png)
ELSE()
    SET(PROGNAME opendcp)
ENDIF()
#-----------------------------------------------------------------------------

#--set gui source-------------------------------------------------------------
SET(OPENDCP_GUI_CXX_SRC
    main.cpp
    mainwindow.cpp
    j2k.cpp
    mxf.cpp
    xml.cpp
    generatetitle.cpp
)

SET(OPENDCP_GUI_H_SRC
    mainwindow.h
    generatetitle.h
)

SET(OPENDCP_GUI_UI
    mainwindow.ui
    generatetitle.ui
)

SET(OPENDCP_GUI_RES
     ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc
)

SET(OPENDCP_GUI_RES_CXX
     ${CMAKE_CURRENT_BINARY_DIR}/opendcp_resources.cpp 
)

#-----------------------------------------------------------------------------

#--win32 icon and info--------------------------------------------------------
IF(MING)
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/opendcpico.o
                        COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/resources/opendcp.rc 
                                -o ${CMAKE_CURRENT_BINARY_DIR}/opendcp.o)
    SET(OPENDCP_GUI_CXX_SRC ${OPENDCP_GUI_CXX_SRC} ${CMAKE_CURRENT_BINARY_DIR}/opendcp.o)
ELSE()
#    SET(OPENDCP_GUI_CXX_SRC ${OPENDCP_GUI_CXX_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/resources/opendcp.rc)
ENDIF()
#-----------------------------------------------------------------------------

#--add qt4 src----------------------------------------------------------------
QT4_ADD_RESOURCES(OPENDCP_GUI_RES_CXX ${OPEDNDCP_GUI_RES})
ADD_CUSTOM_COMMAND(OUTPUT ${OPENDCP_GUI_RES_CXX}  
    COMMAND ${QT_RCC_EXECUTABLE}
    ARGS ${rcc_options} -name ${OPENDCP_GUI_RES_CXX} -o ${OPENDCP_GUI_RES_CXX} ${OPENDCP_GUI_RES} 
    MAIN_DEPENDENCY ${OPENDCP_GUI_RES} 
    DEPENDS ${_RC_DEPENDS})
QT4_WRAP_UI(OPENDCP_FORM_H ${OPENDCP_GUI_UI})
QT4_WRAP_CPP(OPENDCP_MOC_H ${OPENDCP_GUI_H_SRC})
#-----------------------------------------------------------------------------

#--compile gui----------------------------------------------------------------
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
IF(APPLE)
    ADD_EXECUTABLE(${PROGNAME} MACOSX_BUNDLE ${OPENDCP_GUI_CXX_SRC} ${OPENDCP_GUI_RES_CXX} ${OPENDCP_FORM_H} ${OPENDCP_MOC_H})
    SET(OPENDCP_GUI_RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources)
    ADD_CUSTOM_COMMAND(TARGET ${PROGNAME} POST_BUILD
      COMMAND mkdir ARGS -p ${OPENDCP_GUI_RESOURCE_DIR} 
      COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      #COMMAND cp ARGS *.qm ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources)
    )
ELSE()
    ADD_EXECUTABLE(${PROGNAME} ${OPENDCP_GUI_CXX_SRC} ${OPENDCP_GUI_RES_CXX} ${OPENDCP_FORM_H} ${OPENDCP_MOC_H})
ENDIF()
#----------------------------------------------------------------------------

#--build---------------------------------------------------------------------
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
TARGET_LINK_LIBRARIES(${PROGNAME} ${QT_LIBRARIES} ${OPENDCP_LIB} ${ASDCP_LIBRARIES} ${LIBS} ${LINK_FLAGS})
#----------------------------------------------------------------------------

#--install-------------------------------------------------------------------
INSTALL(TARGETS ${PROGNAME} DESTINATION bin)
IF(UNIX AND NOT APPLE)
    INSTALL(FILES ${APP_ICON} DESTINATION share/icons)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/opendcp.desktop DESTINATION share/applications)
ENDIF(UNIX AND NOT APPLE)
#----------------------------------------------------------------------------
