# - Try to find libxmlsec
# Once done, this will define
#
#  XMLSEC1_FOUND - system has libxmlsec 
#  XMLSEC1_INCLUDE_DIR - header file dir 
#  XMLSEC1_LIBRARIES - lib dir 


# Include dir
find_path(XMLSEC1_INCLUDE_DIR
  NAMES xmlsec.h PREFIXES xmlsec xmlsec1/xmlsec
)

# Finally the library itself
find_library(XMLSEC1_LIBRARY
  NAMES xmlsec1
)

find_library(XMLSEC1-OPENSSL_LIBRARY
  NAMES xmlsec1-openssl
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XMLSEC1 DEFAULT_MSG XMLSEC1_LIBRARY XMLSEC1-OPENSSL_LIBRARY XMLSEC1_INCLUDE_DIR)

IF(XMLSEC1_FOUND)
  SET(XMLSEC1_LIBRARIES ${XMLSEC1_LIBRARY} ${XMLSEC1-OPENSSL_LIBRARY})
ENDIF(XMLSEC1_FOUND)
