/*
    OpenDCP: Builds DCP files for Digital Cinema Packages
    Copyright (c) 2010 Terrence Meiczinger, All Rights Reserved

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

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "opendcp.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

static int dcp_log_level = LOG_INFO;

void dcp_log_callback(int level, const char* fmt, va_list vl)
{
    char msg[1024];

    if (dcp_log_level>=level) {
        snprintf(msg, sizeof(msg), "[%s] ", DCP_LOG[level]);
        vsnprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), fmt, vl);
        fprintf(stderr,"%s\n",msg);
    }
}

void dcp_log(int level, const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    dcp_log_callback(level, fmt, vl);
    va_end(vl);
}

void dcp_set_log_level(int level)
{
    dcp_log_level = level;
}
