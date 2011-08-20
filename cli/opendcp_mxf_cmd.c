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
#include "win32/opendcp_win32_dirent.h"
#else
#include <getopt.h>
#include <dirent.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "opendcp.h"

#ifndef WIN32
#define strnicmp strncasecmp
#endif

void version() {
    FILE *fp;

    fp = stdout;
    fprintf(fp,"\n%s version %s %s\n\n",OPEN_DCP_NAME,OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);

    exit(0);
}

void dcp_usage() {
    FILE *fp;
    fp = stdout;

    fprintf(fp,"\n%s version %s %s\n\n",OPEN_DCP_NAME,OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
    fprintf(fp,"Usage:\n");
    fprintf(fp,"       opendcp_mxf -i <file> -o <file> [options ...]\n\n");
    fprintf(fp,"Required:\n");
    fprintf(fp,"       -i | --input <file | dir>      - input file or directory.\n");
    fprintf(fp,"       -1 | --input_left <dir>        - left channel input images when creating a 3D essence\n");
    fprintf(fp,"       -2 | --input_right <dir>       - right channel input images when creating a 3D essence\n");
    fprintf(fp,"       -o | --output <file>           - output mxf file\n");
    fprintf(fp,"\n");
    fprintf(fp,"Options:\n");
    fprintf(fp,"       -n | --ns <interop | smpte>    - Generate SMPTE or MXF Interop labels (default smpte)\n");
    fprintf(fp,"       -r | --rate <rate>             - frame rate (default 24)\n");
    fprintf(fp,"       -s | --start <frame>           - start frame\n");
    fprintf(fp,"       -d | --end  <frame>            - end frame\n");
    fprintf(fp,"       -l | --log_level <level>       - Sets the log level 0:Quiet, 1:Error, 2:Warn (default),  3:Info, 4:Debug\n");
    fprintf(fp,"       -h | --help                    - show help\n");
    fprintf(fp,"       -v | --version                 - show version\n");
    fprintf(fp,"\n\n");
    
    fclose(fp);
    exit(0);
}

static int file_filter(struct dirent *filename) {
    char *extension;

    extension = strrchr(filename->d_name,'.');

    if ( extension == NULL ) {
        return 0;
    }

    extension++;

    /* return only known asset types */
    if (strnicmp(extension,"j2c",3) != 0 && 
        strnicmp(extension,"j2k",3) != 0 && 
        strnicmp(extension,"wav",3) != 0) {
        return 0;
    }

    return 1;
}

int check_extension(char *filename, char *pattern) {
    char *extension;

    extension = strrchr(filename,'.');
    
    if ( extension == NULL ) {
        return 0;
    }

    extension++;

   if (strnicmp(extension,pattern,3) !=0) {
       return 0;
   }

   return 1;
}

int get_filelist_3d(opendcp_t *opendcp,char *in_path_left,char *in_path_right,filelist_t *filelist) {
    filelist_t  *filelist_left;
    filelist_t  *filelist_right;
    int x,y;

    filelist_left = malloc(sizeof(filelist_t));
    filelist_right = malloc(sizeof(filelist_t));

    get_filelist(opendcp,in_path_left,filelist_left);
    get_filelist(opendcp,in_path_right,filelist_right);

    if (filelist_left->file_count != filelist_right->file_count) {
        dcp_log(LOG_ERROR,"Mismatching file count for 3D images left: %d right: %d\n",filelist_left->file_count,filelist_right->file_count);
        return DCP_FATAL; 
    }

    y = 0;
    filelist->file_count = filelist_left->file_count * 2;
    filelist->in = (char**) malloc(filelist->file_count*sizeof(char*));
    for (x=0;x<filelist_left->file_count;x++) {
        filelist->in[y] = (char *) malloc(MAX_FILENAME_LENGTH);
        filelist->in[y++] = filelist_left->in[x];
        filelist->in[y] = (char *) malloc(MAX_FILENAME_LENGTH);
        filelist->in[y++] = filelist_right->in[x];
    }

    if ( filelist_left != NULL) {
        free(filelist_left);
    }

    if ( filelist_right != NULL) {
        free(filelist_right);
    }

    return DCP_SUCCESS;
}

int get_filelist(opendcp_t *opendcp,char *in_path,filelist_t *filelist) {
    struct dirent **files;
    struct stat st_in;
    struct stat st_out;
    int x = 0;

    if (stat(in_path, &st_in) != 0 ) {
        dcp_log(LOG_ERROR,"Could not open input file %s",in_path);
        return DCP_FATAL;
    }

    if (S_ISDIR(st_in.st_mode)) {
        /* if input is directory, it is jpeg2000 or pcm */
        filelist->file_count = scandir(in_path,&files,(void *)file_filter,alphasort);
        filelist->in = (char**) malloc(filelist->file_count*sizeof(char*));
        if (filelist->file_count) {
            for (x=0;x<filelist->file_count;x++) {
                filelist->in[x] = (char *) malloc(MAX_FILENAME_LENGTH);
                sprintf(filelist->in[x],"%s/%s",in_path,files[x]->d_name);
            }
        }
    } else {
        /* mpeg2 or time_text */
        int essence_type = get_file_essence_type(in_path);
        if (essence_type == AET_UNKNOWN) {
            return DCP_FATAL;
        }
        filelist->file_count = 1;
        filelist->in = (char**) malloc(filelist->file_count*sizeof(char*));
        filelist->in[0] = (char *) malloc(MAX_FILENAME_LENGTH);
        filelist->in[0] = in_path;
    }

    return DCP_SUCCESS;
}

int main (int argc, char **argv) {
    int c;
    int count;
    int image_nb = 0;
    int result;
    opendcp_t *opendcp;
    char *in_path = NULL;
    char *in_path_left = NULL;
    char *in_path_right = NULL;
    char *out_path = NULL;
    filelist_t *filelist;
    char out_file[4096]; 

    if ( argc <= 1 ) {
        dcp_usage();
    }

    opendcp = malloc(sizeof(opendcp_t));
    memset(opendcp,0,sizeof (opendcp_t));

    filelist = malloc(sizeof(filelist_t));
    memset(filelist,0,sizeof (filelist_t));

    /* set initial values */
    opendcp->xyz = 1;
    opendcp->log_level = LOG_WARN;
    opendcp->ns = XML_NS_SMPTE;
    opendcp->frame_rate = 24;
    opendcp->threads = 4;

    /* parse options */
    while (1)
    {
        static struct option long_options[] =
        {
            {"help",           required_argument, 0, 'h'},
            {"input",          required_argument, 0, 'i'},
            {"left",           required_argument, 0, '1'},
            {"right",          required_argument, 0, '2'},
            {"ns",             required_argument, 0, 'n'},
            {"output",         required_argument, 0, 'o'},
            {"start",          required_argument, 0, 's'},
            {"end",            required_argument, 0, 'd'},
            {"rate",           required_argument, 0, 'r'},
            {"profile",        required_argument, 0, 'p'},
            {"log_level",      required_argument, 0, 'l'},
            {"version",        no_argument,       0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;
     
        c = getopt_long (argc, argv, "1:2:d:i:n:o:r:s:p:l:3hv",
                         long_options, &option_index);
     
        /* Detect the end of the options. */
        if (c == -1)
            break;
     
        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                   break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                   printf (" with arg %s", optarg);
                 printf ("\n");
            break;

            case '3':
               opendcp->stereoscopic = 1;
            break;

            case 'd':
               opendcp->mxf.end_frame = atoi(optarg);
               if (opendcp->mxf.end_frame < 1) {
                   dcp_fatal(opendcp,"End frame  must be greater than 0");
               }
            break;

            case 's':
               opendcp->mxf.start_frame = atoi(optarg);
               if (opendcp->mxf.start_frame < 1) {
                   dcp_fatal(opendcp,"Start frame must be greater than 0");
               }
            break;

            case 'n':
               if (!strcmp(optarg,"smpte")) {
                   opendcp->ns = XML_NS_SMPTE;
               } else if (!strcmp(optarg,"interop")) {
                   opendcp->ns = XML_NS_INTEROP;
               } else {
                   dcp_fatal(opendcp,"Invalid profile argument, must be smpte or interop");
               }
            break;

            case 'i':
               in_path = optarg;
            break;

            case '1':
               in_path_left = optarg;
               opendcp->stereoscopic = 1;
            break;

            case '2':
               in_path_right = optarg;
               opendcp->stereoscopic = 1;
            break;

            case 'l':
               opendcp->log_level = atoi(optarg);
            break;

            case 'o':
               out_path = optarg;
            break;

            case 'h':
               dcp_usage();
            break;

            case 'r':
               opendcp->frame_rate = atoi(optarg);
               if (opendcp->frame_rate > 60 || opendcp->frame_rate < 1 ) {
                   dcp_fatal(opendcp,"Invalid frame rate. Must be between 1 and 60.");
               }
            break;

            case 'v':
               version();
            break;
        }
    }

    /* set log level */
    dcp_set_log_level(opendcp->log_level);

    if (opendcp->log_level > 0) {
        printf("\nOpenDCP MXF %s %s\n",OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
    }

    if (opendcp->stereoscopic) {
        if (in_path_left == NULL) {
            dcp_fatal(opendcp,"3D input detected, but missing left image input path");
        } else if (in_path_right == NULL) {
            dcp_fatal(opendcp,"3D input detected, but missing right image input path");
        }
    } else {
        if (in_path == NULL) {
            dcp_fatal(opendcp,"Missing input file");
        }
    }

    if (out_path == NULL) {
        dcp_fatal(opendcp,"Missing output file");
    }

    if (opendcp->stereoscopic) {
        get_filelist_3d(opendcp,in_path_left,in_path_right,filelist);
    } else {
        get_filelist(opendcp,in_path,filelist);
    }
  
    if (filelist->file_count < 1) {
        dcp_fatal(opendcp,"No input files located");
    }
  
    if (opendcp->mxf.end_frame) {
        if (opendcp->mxf.end_frame > filelist->file_count) {
            dcp_fatal(opendcp,"End frame is greater than the actual frame count");
        }
    } else {
        opendcp->mxf.end_frame = filelist->file_count;
    }

    if (opendcp->mxf.start_frame) {
        if (opendcp->mxf.start_frame > opendcp->mxf.end_frame) {
            dcp_fatal(opendcp,"Start frame must be less than end frame");
        }
    } else {
        opendcp->mxf.start_frame = 1;
    }

    opendcp->mxf.duration = opendcp->mxf.end_frame - (opendcp->mxf.start_frame-1);  

    if (opendcp->mxf.duration < 1) {
        dcp_fatal(opendcp,"Duration must be at least 1 frame");
    }

    if (write_mxf(opendcp,filelist,out_path) != 0 )  {
        printf("Error!\n");
    }

    if ( filelist != NULL) {
        free(filelist);
    }

    dcp_log(LOG_INFO,"MXF creation complete");

    if (opendcp->log_level > 0) {
        printf("\n");
    }

    if ( opendcp != NULL) {
        free(opendcp);
    }

    exit(0);
}