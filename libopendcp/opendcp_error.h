/*
    OpenDCP: Builds Digital Cinema Packages
    Copyright (c) 2010-2011 Terrence Meiczinger, All Rights Reserved

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _OPEN_ERROR_DCP_H_
#define _OPEN_ERROR_DCP_H_

enum DCP_ERROR_MESSAGES {
    DCP_NO_ERROR   = 0,
    DCP_ERROR,
    DCP_FILE_OPEN_ERROR,
    DCP_NO_PICTURE_TRACK,
    DCP_MULTIPLE_PICTURE_TRACK,
    DCP_ASSET_NO_DURATION,
    DCP_INVALID_ESSENCE,
    DCP_SPECIFCATION_MISMATCH,
  
    /* J2K */
    J2K_ERROR,

    /* MXF */
    MXF_CALC_DIGEST_FAILED,
    MXF_COULD_NOT_DETECT_ESSENCE_TYPE,
    MXF_UNKOWN_ESSENCE_TYPE,
    MXF_MPEG2_FILE_OPEN_ERROR,
    MXF_J2K_FILE_OPEN_ERROR,
    MXF_WAV_FILE_OPEN_ERROR,
    MXF_TT_FILE_OPEN_ERROR,
    MXF_FILE_WRITE_ERROR,
    MXF_FILE_FINALIZE_ERROR,
    MXF_PARSER_RESET_ERROR,

    /* XML */

    /* COMMON */
    STRING_LENGTH_NOTEQUAL,

    DCP_ERROR_MAX
};

static const char *ERR_STRING[] = {
    "NONE",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

/* retrieve error string */
char *error_string(int error_code);

#endif // _OPEN_ERROR_DCP_H_

#ifdef __cplusplus
}
#endif
