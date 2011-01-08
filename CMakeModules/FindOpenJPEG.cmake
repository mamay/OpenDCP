# - Try to find libOPENJPEG
# Once done, this will define
#
#  OPENJPEG_FOUND - system has asdcplib
#  OPENJPEG_INCLUDE_DIRS - the asdcplib include directories
#  OPENJPEG_LIBRARIES - link these to use asdcplib


# Include dir
find_path(OPENJPEG_INCLUDE_DIR
  NAMES openjpeg.h
)

# Finally the library itself
find_library(OPENJPEG_LIBRARY
  NAMES openjpeg 
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENJPEG DEFAULT_MSG OPENJPEG_LIBRARY OPENJPEG_INCLUDE_DIR)

IF(OPENJPEG_FOUND)
  SET(OPENJPEG_LIBRARIES ${OPENJPEG_LIBRARY})
ENDIF(OPENJPEG_FOUND)
