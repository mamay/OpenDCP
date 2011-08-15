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

#include <string.h>
#ifdef WIN32
#include "win32/opendcp_win32_getopt.h"
#else
#include <getopt.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#include "opendcp.h"

void dcp_usage() {
    FILE *fp;
    fp = stdout;

    fprintf(fp,"\n%s version %s %s\n\n",OPEN_DCP_NAME,OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
    fprintf(fp,"Verifies the digital signature of an XML file\n\n");
    fprintf(fp,"Usage:\n");
    fprintf(fp,"       opendcp_xml_verify <xml file>\n\n");

    fclose(fp);
    exit(0);
}

int main (int argc, char **argv) {
    FILE *fp;
    int result;
    char *filename;

    if ( argc <= 1 ) {
        dcp_usage();
    }

    filename = argv[1];

    fp = fopen(filename,"rb");

    if (fp) {
        fclose(fp);
    } else {
        dcp_log(LOG_ERROR,"could not open file: %s",filename);
        exit(DCP_FATAL);
    } 

    result = xml_verify(filename);

    if (result == DCP_SUCCESS) {
        dcp_log(LOG_INFO,"%s Signature is VALID",filename);
        exit(DCP_SUCCESS);
    } else {
        dcp_log(LOG_ERROR,"%s Signature is NOT VALID",filename);
        exit(DCP_FATAL);
    }
}
