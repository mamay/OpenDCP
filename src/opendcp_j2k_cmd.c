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
#else
#include <getopt.h>
#include <dirent.h>
#include <signal.h>
#endif
#ifdef OPENMP
#include <omp.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "opendcp.h"

#ifndef WIN32
#define strnicmp strncasecmp
#endif

opendcp_t *opendcp_ptr;

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
    fprintf(fp,"       -e | --encoder <0 | 1>         - jpeg2000 encoder 0:openjpeg 1:kakadu^ (default openjpeg)\n");
    fprintf(fp,"       -b | --bw                      - Max Mbps bandwitdh (default: 250)\n");
    fprintf(fp,"       -n | --no_overwrite            - do not overwrite existing jpeg2000 files\n");
    fprintf(fp,"       -l | --log_level <level>       - Sets the log level 0:Quiet, 1:Error, 2:Warn (default),  3:Info, 4:Debug\n");
    fprintf(fp,"       -h | --help                    - show help\n");
    fprintf(fp,"       -g | --lut                     - select color conversion LUT, 0:rec709,1:srgb\n");
    fprintf(fp,"       -s | --start                   - start frame\n");
    fprintf(fp,"       -d | --end                     - end frame\n");
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

int get_filelist(opendcp_t *opendcp,char *in_path,char *out_path,filelist_t *filelist) {
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
#ifdef OPENMP
    int nthreads = omp_get_num_threads(); 
#else
    int nthreads = 1;
#endif
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
    int openmp_flag = 0;
    opendcp_t *opendcp;
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

    opendcp = malloc(sizeof(opendcp_t));
    opendcp_ptr = opendcp;

    memset(opendcp,0,sizeof (opendcp_t));

    filelist = malloc(sizeof(filelist_t));
    memset(filelist,0,sizeof (filelist_t));

    /* set initial values */
    opendcp->xyz = 1;
    opendcp->log_level = LOG_WARN;
    opendcp->cinema_profile = DCP_CINEMA2K;
    opendcp->frame_rate = 24;
#ifdef OPENMP
    openmp_flag = 1;
    opendcp->threads = omp_get_num_procs();
#endif
 
    /* parse options */
    while (1)
    {
        static struct option long_options[] =
        {
            {"help",           required_argument, 0, 'h'},
            {"input",          required_argument, 0, 'i'},
            {"output",         required_argument, 0, 'o'},
            {"bw     ",        required_argument, 0, 'b'},
            {"rate",           required_argument, 0, 'r'},
            {"profile",        required_argument, 0, 'p'},
            {"log_level",      required_argument, 0, 'l'},
            {"threads",        required_argument, 0, 't'},
            {"encoder",        required_argument, 0, 'e'},
            {"start",          required_argument, 0, 's'},
            {"end",            required_argument, 0, 'e'},
            {"no_xyz",         no_argument,       0, 'x'},
            {"no_overwrite",   no_argument,       0, 'n'},
            {"3d",             no_argument,       0, '3'},
            {"version",        no_argument,       0, 'v'},
            {"tmp_dir",        required_argument, 0, 'm'},
            {"lut",            required_argument, 0, 'g'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;
     
        c = getopt_long (argc, argv, "b:d:e:i:o:r:s:p:l:t:m:g:3hvxn",
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
               opendcp->stereoscopic = 1;
            break;

            case 'd':
               opendcp->j2k.end_frame = atoi(optarg);
               if (opendcp->j2k.end_frame < 1) {
                   dcp_fatal(opendcp,"End frame  must be greater than 0");
               }
            break;

            case 's':
               opendcp->j2k.start_frame = atoi(optarg);
               if (opendcp->j2k.start_frame < 1) {
                   dcp_fatal(opendcp,"Start frame must be greater than 0");
               }
            break;

            case 'p':
               if (!strcmp(optarg,"cinema2k")) {
                   opendcp->cinema_profile = DCP_CINEMA2K;
               } else if (!strcmp(optarg,"cinema4k")) {
                   opendcp->cinema_profile = DCP_CINEMA4K;
               } else {
                   dcp_fatal(opendcp,"Invalid profile argument, must be cinema2k or cinema4k");
               }
            break;

            case 'i':
               in_path = optarg;
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

            case 'e':
               opendcp->encoder = atoi(optarg);
               if (opendcp->encoder == J2K_KAKADU) {
                   result = system("kdu_compress -u >/dev/null 2>&1");
                   if (result>>8 != 0) {
                       dcp_fatal(opendcp,"kdu_compress was not found. Either add to path or remove -e 1 flag");
                   }
               }
            break;

            case 'b':
               opendcp->bw = atoi(optarg);
               if (opendcp->bw < 50 || opendcp->bw > 250) {
                   dcp_fatal(opendcp,"Bandwidth must be between 50 and 250");
               } else {
                   opendcp->bw *= 1000000;
               }
            break;

            case 't':
               opendcp->threads = atoi(optarg);
            break;

            case 'x':
               opendcp->xyz = 0;
            break;

            case 'n':
               opendcp->no_overwrite = 1;
            break;
     
            case 'v':
               version();
            break;

            case 'm':
                tmp_path = optarg;
            break;
            case 'g':
                opendcp->lut = atoi(optarg);
            break;
        }
    }

    /* set log level */
    dcp_set_log_level(opendcp->log_level);

    if (opendcp->log_level > 0) {
        printf("\nOpenDCP J2K %s %s\n",OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
        if (opendcp->encoder == J2K_KAKADU) {
            printf("  Encoder: Kakadu\n");
        } else {
            printf("  Encoder: OpenJPEG\n");
        }
    }

    if (in_path == NULL) {
        dcp_fatal(opendcp,"Missing input file");
    }

    if (out_path == NULL) {
        dcp_fatal(opendcp,"Missing output file");
    }

    get_filelist(opendcp,in_path,out_path,filelist);

    if (opendcp->j2k.end_frame) {
        if (opendcp->j2k.end_frame > filelist->file_count) {
            dcp_fatal(opendcp,"End frame is greater than the actual frame count");
        }
    } else {
        opendcp->j2k.end_frame = filelist->file_count;
    }

    if (opendcp->j2k.start_frame) {
        if (opendcp->j2k.start_frame > opendcp->j2k.end_frame) {
            dcp_fatal(opendcp,"Start frame must be less than end frame");
        }
    } else {
        opendcp->j2k.start_frame = 1;
    }


    if (opendcp->log_level>0 && opendcp->log_level<3) { progress_bar(0,0); }

#ifdef OPENMP
    omp_set_num_threads(opendcp->threads);
#endif

    count = opendcp->j2k.start_frame;

    #pragma omp parallel for private(c)
    for (c=opendcp->j2k.start_frame-1;c<opendcp->j2k.end_frame;c++) {    
        #pragma omp flush(SIGINT_received)
        if (!SIGINT_received) {
            dcp_log(LOG_INFO,"JPEG2000 conversion %s started OPENMP: %d",filelist->in[c],openmp_flag);
            if(access( filelist->out[c], F_OK ) != 0 || opendcp->no_overwrite == 0) {
                result = convert_to_j2k(opendcp,filelist->in[c],filelist->out[c], tmp_path);
            } else {
                result = DCP_SUCCESS;
            }
            if (count) {
               if (opendcp->log_level>0 && opendcp->log_level<3) {progress_bar(count,opendcp->j2k.end_frame);}
            }

            if (result == DCP_FATAL) {
                dcp_log(LOG_ERROR,"JPEG200 conversion %s failed",filelist->in[c]);
                dcp_fatal(opendcp,"Exiting...");
            } else {
                dcp_log(LOG_INFO,"JPEG2000 conversion %s complete",filelist->in[c]);
            }
            count++;
        } else {
            cleanup(opendcp, 1);
        }
    }

    if (opendcp->log_level>0 && opendcp->log_level<3) {progress_bar(count-1,opendcp->j2k.end_frame);}

    if ( filelist != NULL) {
        free(filelist);
    }

    if (opendcp->log_level > 0) {
        printf("\n");
    }

    cleanup(opendcp,0);

    if ( opendcp != NULL) {
        free(opendcp);
    }

    exit(0);
}
