/*
    OpenDCP: Builds Digital Cinema Packages
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

#ifdef WIN32
#include "win32/opendcp_win32_getopt.h"
#include "win32/opendcp_win32_dirent.h"
#include <omp-win32.h>
#else
#include <getopt.h>
#include <dirent.h>
#include <omp.h>
#include <signal.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "opendcp.h"

#ifndef WIN32
#define strnicmp strncasecmp
#endif

context_t *context_ptr;

#ifndef _WIN32
sig_atomic_t SIGINT_received = 0;

void sig_handler(int signum) {
    SIGINT_received = 1;
    #pragma omp flush(SIGINT_received)
}
#else
int SIGINT_received = 0;
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
    fprintf(fp,"       opendcp_j2k -i <file> -o <file> [options ...]\n\n");
    fprintf(fp,"Required:\n");
    fprintf(fp,"       -i | --input <file>            - input file or directory)\n");
    fprintf(fp,"       -o | --output <file>           - output file or directory\n");
    fprintf(fp,"\n");
    fprintf(fp,"Options:\n");
    fprintf(fp,"       -r | --rate <rate>             - frame rate (default 24)\n");
    fprintf(fp,"       -p | --profile <profile>       - profile cinema2k | cinema4k (default cinema2k)\n");
    fprintf(fp,"       -3 | --3d                      - Adjust frame rate for 3D\n");
    fprintf(fp,"       -t | --threads <threads>       - Set number of threads (default 4)\n");
    fprintf(fp,"       -x | --no_xyz                  - do not perform rgb->xyz color conversion\n");
    fprintf(fp,"       -e | --encoder <0 | 1>         - jpeg2000 encoder 0:openjpep 1:kakadu^ (default openjpeg)\n");
    fprintf(fp,"       -q | --quality                 - image quality level 0-100 (default 100)\n");
    fprintf(fp,"       -l | --log_level <level>       - Sets the log level 0:Quiet, 1:Error, 2:Warn (default),  3:Info, 4:Debug\n");
    fprintf(fp,"       -h | --help                    - show help\n");
    fprintf(fp,"       -v | --version                 - show version\n");
    fprintf(fp,"       -m | --tmp_dir                 - sets temporary directory (usually tmpfs one) to save there temporary tiffs for Kakadu");
    fprintf(fp,"\n\n");
    fprintf(fp,"^ Kakadu requires you to download and have the kdu_compress utility in your path.\n");
    fprintf(fp,"  You must agree to the Kakadu licensing terms and assume all respsonsibility of its use.\n");
    fprintf(fp,"\n\n");
    
    fclose(fp);
    exit(0);
}

static int file_filter(struct dirent *filename) {
    return check_extension(filename->d_name,"tif");
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

char *get_basename(const char *filename) {
    char *extension;
    char *base = 0;

    base = (char *)filename;
    extension = strrchr(filename,'.');
    base[(strlen(filename) - strlen(extension))] = '\0';

    return(base);
}

int get_filelist(context_t *context,char *in_path,char *out_path,filelist_t *filelist) {
    struct dirent **files;
    int x = 0;
    struct stat st_in;
    struct stat st_out;
    char *extension;

    if (stat(in_path, &st_in) != 0 ) {
        dcp_log(LOG_ERROR,"Could not open input file %s",in_path);
        return DCP_FATAL;
    }

    /* directory */
    if (S_ISDIR(st_in.st_mode)) {
        if (stat(out_path, &st_out) != 0 ) {
            dcp_log(LOG_ERROR,"Output directory %s does not exist",out_path);
            return DCP_FATAL;
        }

        if (st_out.st_mode & S_IFDIR) {
            filelist->file_count = scandir(in_path,&files,(void *)file_filter,alphasort);
            filelist->in = (char**) malloc(filelist->file_count*sizeof(char*));
            filelist->out = (char**) malloc(filelist->file_count*sizeof(char*));
            if (filelist->file_count) {
                for (x=0;x<filelist->file_count;x++) {
                        filelist->in[x] = (char *) malloc(MAX_FILENAME_LENGTH);
                        filelist->out[x] = (char *) malloc(MAX_FILENAME_LENGTH);
                        sprintf(filelist->in[x],"%s/%s",in_path,files[x]->d_name);
                        sprintf(filelist->out[x],"%s/%s.j2c",out_path,get_basename(files[x]->d_name));
                }
            }
        } else {
            dcp_log(LOG_ERROR,"If input is a directory, output must be as well");
            return DCP_FATAL;
        }
    } else {
        if (stat(out_path, &st_out) == 0 ) {
            if (st_out.st_mode & S_IFDIR) {
                dcp_log(LOG_ERROR,"If input is a file, output must be as well");
                return DCP_FATAL;
            }
        }
        extension = strrchr(in_path,'.');
        if (strnicmp(++extension,"tif",3) == 0) {
            filelist->file_count = 1;
            filelist->in = (char**) malloc(filelist->file_count*sizeof(char*));
            filelist->out = (char**) malloc(filelist->file_count*sizeof(char*));
            filelist->in[0] = (char *) malloc(MAX_FILENAME_LENGTH);
            filelist->out[0] = (char *) malloc(MAX_FILENAME_LENGTH);
            filelist->in[0] = in_path;
            filelist->out[0] = out_path;
        }
    }

   return DCP_SUCCESS;
}

void progress_bar(int val, int total) {
    int x;
    int step = 20;
    float c = (float)step/total * (float)val;
    int nthreads = omp_get_num_threads(); 
    printf("  JPEG2000 Conversion (%d thread",nthreads);
    if (nthreads > 1) {
        printf("s");
    }
    printf(") [");
    for (x=0;x<step;x++) {
        if (c>x) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] 100%% [%d/%d]\r",val,total);
    fflush(stdout);
}

int main (int argc, char **argv) {
    int c, result, count = 0, abort = 0;
    context_t *context;
    char *in_path = NULL;
    char *out_path = NULL;
    char *tmp_path = NULL;
    char dir_str[] = "tmpXXXXXX";
    filelist_t *filelist;

#ifndef _WIN32
    struct sigaction sig_action;
    sig_action.sa_handler = sig_handler;
    sig_action.sa_flags = 0;
    sigemptyset(&sig_action.sa_mask);

    sigaction(SIGINT,  &sig_action, NULL);
#endif

    if ( argc <= 1 ) {
        dcp_usage();
    }

    context = malloc(sizeof(context_t));
    context_ptr = context;

    memset(context,0,sizeof (context_t));

    filelist = malloc(sizeof(filelist_t));
    memset(filelist,0,sizeof (filelist_t));

    /* set initial values */
    context->xyz = 1;
    context->log_level = LOG_WARN;
    context->cinema_profile = DCP_CINEMA2K;
    context->frame_rate = 24;
    context->threads = 4;
 
    /* parse options */
    while (1)
    {
        static struct option long_options[] =
        {
            {"help",           required_argument, 0, 'h'},
            {"input",          required_argument, 0, 'i'},
            {"output",         required_argument, 0, 'o'},
            {"quality",        required_argument, 0, 'q'},
            {"rate",           required_argument, 0, 'r'},
            {"profile",        required_argument, 0, 'p'},
            {"log_level",      required_argument, 0, 'l'},
            {"threads",        required_argument, 0, 't'},
            {"encoder",        required_argument, 0, 'e'},
            {"no_xyz",         no_argument,       0, 'x'},
            {"3d",             no_argument,       0, '3'},
            {"version",        no_argument,       0, 'v'},
            {"tmp_dir",        required_argument, 0, 'm'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;
     
        c = getopt_long (argc, argv, "e:i:o:r:p:q:l:t:m:3hvx",
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
            break;

            case '3':
               context->stereoscopic = 1;
            break;

            case 'p':
               if (!strcmp(optarg,"cinema2k")) {
                   context->cinema_profile = DCP_CINEMA2K;
               } else if (!strcmp(optarg,"cinema4k")) {
                   context->cinema_profile = DCP_CINEMA4K;
               } else {
                   dcp_fatal(context,"Invalid profile argument, must be cinema2k or cinema4k");
               }
            break;

            case 'i':
               in_path = optarg;
            break;

            case 'l':
               context->log_level = atoi(optarg);
            break;

            case 'o':
               out_path = optarg;
            break;

            case 'h':
               dcp_usage();
            break;

            case 'r':
               context->frame_rate = atoi(optarg);
               if (context->frame_rate > 60 || context->frame_rate < 1 ) {
                   dcp_fatal(context,"Invalid frame rate. Must be between 1 and 60.");
               }
            break;

            case 'e':
               context->encoder = atoi(optarg);
               if (context->encoder == J2K_KAKADU) {
                   result = system("kdu_compress -u >& /dev/null");
                   if (result>>8 != 0) {
                       dcp_fatal(context,"kdu_compress was not found. Either add to path or remove -e 1 flag");
                   }
               }
            break;

            case 'q':
               context->quality = atoi(optarg);
               if (context->quality < 1 || context->quality > 100) {
                   dcp_fatal(context,"Quality modifier must be between 1 and 100");
               }
            break;

            case 't':
               context->threads = atoi(optarg);
            break;

            case 'x':
               context->xyz = 0;
            break;
     
            case 'v':
               version();
            break;

            case 'm':
                tmp_path = optarg;
            break;
        }
    }

    /* set log level */
    dcp_set_log_level(context->log_level);

    if (context->log_level > 0) {
        printf("\nOpenDCP J2K %s %s\n",OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
        if (context->encoder == J2K_KAKADU) {
            printf("  Encoder: Kakadu\n");
        } else {
            printf("  Encoder: OpenJPEG\n");
        }
    }

    if (in_path == NULL) {
        dcp_fatal(context,"Missing input file");
    }

    if (out_path == NULL) {
        dcp_fatal(context,"Missing output file");
    }

    get_filelist(context,in_path,out_path,filelist);

    /* adjust frame rate for 3D */
    if (context->stereoscopic) {
        context->frame_rate *=2;
    }

    if (context->log_level>0 && context->log_level<3) { progress_bar(0,0); }
    omp_set_num_threads(context->threads);

    #pragma omp parallel for
    for (c=0;c<filelist->file_count;c++) {    
        #pragma omp flush(SIGINT_received)
        if (!SIGINT_received) {
            dcp_log(LOG_INFO,"JPEG2000 conversion %s started",filelist->in[c]);
            result = convert_to_j2k(context,filelist->in[c],filelist->out[c], tmp_path);
            if (count) {
               if (context->log_level>0 && context->log_level<3) {progress_bar(count,filelist->file_count);}
            }

            if (result == DCP_FATAL) {
                dcp_log(LOG_ERROR,"JPEG200 conversion %s failed",filelist->in[c]);
                dcp_fatal(context,"Exiting...");
            } else {
                dcp_log(LOG_INFO,"JPEG2000 conversion %s complete",filelist->in[c]);
            }
            count++;
        } else {
            cleanup(context, 1);
        }
    }

    if (context->log_level>0 && context->log_level<3) {progress_bar(count,filelist->file_count);}

    if ( filelist != NULL) {
        free(filelist);
    }

    if (context->log_level > 0) {
        printf("\n");
    }

    cleanup(context,0);

    if ( context != NULL) {
        free(context);
    }

    exit(0);
}
