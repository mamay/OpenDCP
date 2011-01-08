# - Try to find libxmlsec
# Once done, this will define
#
#  XMLSEC_FOUND - system has libxmlsec 
#  XMLSEC_INCLUDE_DIRS - header file dir 
#  XMLSEC_LIBRARIES - lib dir 


# Include dir
find_path(XMLSEC_INCLUDE_DIR
  NAMES xmlsec.h xmlsec/xmlsec.h
)

# Finally the library itself
find_library(XMLSEC1_LIBRARY
  NAMES xmlsec1
)

find_library(XMLSEC1-OPENSSL_LIBRARY
  NAMES xmlsec1-openssl
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XMLSEC DEFAULT_MSG XMLSEC1_LIBRARY XMLSEC1-OPENSSL_LIBRARY XMLSEC_INCLUDE_DIR)

IF(XMLSEC_FOUND)
  SET(XMLSEC_LIBRARIES ${XMLSEC1_LIBRARY} ${XMLSEC1-OPENSSL_LIBRARY})
ENDIF(XMLSEC_FOUND)
