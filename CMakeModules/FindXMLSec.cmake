# - Try to find the LibXMLSec digital siginature library
# Once done this will define
#
#  XMLSEC1_FOUND - System has LibXMLSec
#  XMLSEC1_INCLUDE_DIR - The LibXMLSec include directory
#  XMLSEC1_LIBRARIES - The libraries needed to use LibXMLSec
#  XMLSEC1_DEFINITIONS - Compiler switches required for using LibXMLSec

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_XMLSEC1 xmlsec1 QUIET)
PKG_CHECK_MODULES(PC_XMLSEC1-OPENSSL xmlsec1-openssl QUIET)
SET(XMLSEC1_DEFINITIONS ${PC_XMLSEC1_CFLAGS_OTHER})
SET(XMLSEC1-OPENSSL_DEFINITIONS ${PC_XMLSEC1-OPENSSL_CFLAGS_OTHER})

FIND_PATH(XMLSEC1_INCLUDE_DIR NAMES xmlsec/xmlsec.h
   HINTS
   ${PC_XMLSEC1_INCLUDEDIR}
   ${PC_XMLSEC1_INCLUDE_DIRS}
   PATH_SUFFIXES xmlsec1 
   )

FIND_PATH(XMLSEC1-OPENSSL_INCLUDE_DIR NAMES xmlsec/openssl/crypto.h
   HINTS
   ${PC_XMLSEC1-OPENSSL_INCLUDEDIR}
   ${PC_XMLSEC1-OPENSSL_INCLUDE_DIRS}
   PATH_SUFFIXES xmlsec1
   )

FIND_LIBRARY(XMLSEC1_LIBRARIES NAMES xmlsec1
   HINTS
   ${PC_XMLSEC1_LIBDIR}
   ${PC_XMLSEC1_LIBRARY_DIRS}
   )

FIND_LIBRARY(XMLSEC1-OPENSSL_LIBRARIES NAMES xmlsec1-openssl
   HINTS
   ${PC_XMLSEC1_LIBDIR}
   ${PC_XMLSEC1_LIBRARY_DIRS}
   )

# handle the QUIETLY and REQUIRED arguments and set XMLSEC1_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXMLSec DEFAULT_MSG XMLSEC1_LIBRARIES XMLSEC1-OPENSSL_LIBRARIES XMLSEC1_INCLUDE_DIR XMLSEC1-OPENSSL_INCLUDE_DIR)
MARK_AS_ADVANCED(XMLSEC1_INCLUDE_DIR XMLSEC1-OPENSSL_INCLUDE_DIR XMLSEC1_LIBRARIES XMLSEC1-OPENSSL_LIBRARIES )
