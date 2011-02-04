/*
    OpenDCP: Builds XML files for Digital Cinema Packages
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
    fprintf(fp,"       opendcp_xml --reel <mxf mxf mxf> [options]\n\n");
    fprintf(fp,"Ex:\n");
    fprintf(fp,"       opendcp_xml --reel picture.mxf sound.mxf\n");
    fprintf(fp,"       opendcp_xml --reel picture.mxf sound.mxf --reel picture.mxf sound.mxf\n");
    fprintf(fp,"       opendcp_xml --reel picture.mxf sound.mxf subtitle.mxf --reel picture.mxf sound.mxf\n");
    fprintf(fp,"       opendcp_xml --reel picture.mxf sound.mxf --digest --creator OpenDCP --kind trailer\n");
    fprintf(fp,"\n");
    fprintf(fp,"Required: At least 1 reel is required:\n");
    fprintf(fp,"       -r | --reel <mxf mxf mxf>      - Creates a reel of MXF elements. The first --reel is reel 1, second --reel is reel 2, etc.\n");
    fprintf(fp,"                                        The agrument is a space seperated list of the essence elemements.\n");
    fprintf(fp,"                                        Picture/Sound/Subtitle (order of the mxf files in the list doesn't matter)\n");
    fprintf(fp,"                                        *** a picture mxf is required per reel ***\n");
    fprintf(fp,"Options:\n");
    fprintf(fp,"       -h | --help                    - show help\n");
    fprintf(fp,"       -v | --version                 - show version\n");
    fprintf(fp,"       -d | --digest                  - Generates digest (used to validate DCP asset integrity)\n");
    fprintf(fp,"       -s | --sign                    - Writes XML digital signature\n");
    fprintf(fp,"       -1 | --root                    - root pem certificate used to sign XML files\n");
    fprintf(fp,"       -2 | --ca                      - ca (intermediate) pem certificate used to sign XML files\n");
    fprintf(fp,"       -3 | --signer                  - signer (leaf) pem certificate used to sign XML files\n");
    fprintf(fp,"       -p | --privatekey              - privae (signer) pem key used to sign XML files\n");
    fprintf(fp,"       -i | --issuer <issuer>         - Issuer details\n");
    fprintf(fp,"       -c | --creator <creator>       - Creator details\n");
    fprintf(fp,"       -a | --annotation <annotation> - Asset annotations\n");
    fprintf(fp,"       -t | --title <title>           - DCP content title\n");
    fprintf(fp,"       -b | --base <basename>         - Prepend CPL/PKL filenames with basename rather than UUID\n");
    fprintf(fp,"       -n | --duration <duration>     - Set asset durations in frames\n");
    fprintf(fp,"       -m | --rating <duration>       - Set DCP MPAA rating G PG PG-13 R NC-17 (default none)\n");
    fprintf(fp,"       -e | --entry <entry point>     - Set asset entry point (offset) frame\n");
    fprintf(fp,"       -k | --kind <kind>             - Content kind (test, feature, trailer, policy, teaser, etc)\n");
    fprintf(fp,"       -l | --log_level <level>       - Sets the log level 0:Quiet, 1:Error, 2:Warn (default),  3:Info, 4:Debug\n");
    fprintf(fp,"\n\n");

    fclose(fp);
    exit(0);
}

int main (int argc, char **argv) {
    int c,j;
    int x=0;
    char uuid_s[40];
    char buffer[80];
    context_t *context;
    asset_list_t reel_list[MAX_REELS];

    if ( argc <= 1 ) {
        dcp_usage();
    }

    context = malloc(sizeof(context_t));
    memset(context,0,sizeof (context_t));

    /* initialize context */
    context->log_level = LOG_WARN;
    context->reel_count = 0;
    sprintf(context->issuer,"%.80s %.80s",OPEN_DCP_NAME,OPEN_DCP_VERSION);
    sprintf(context->creator,"%.80s %.80s",OPEN_DCP_NAME, OPEN_DCP_VERSION);
    sprintf(context->annotation,"%.80s",DCP_ANNOTATION);
    sprintf(context->title,"%.80s",DCP_TITLE);
    sprintf(context->kind,"%.15s",DCP_KIND);
    get_timestamp(context->timestamp);

    /* parse options */
    while (1)
    {
        static struct option long_options[] =
        {
            {"annotation",     required_argument, 0, 'a'},
            {"base",           required_argument, 0, 'b'},
            {"creator",        required_argument, 0, 'c'},
            {"digest",         no_argument,       0, 'd'},
            {"duration",       required_argument, 0, 'n'},
            {"entry",          required_argument, 0, 'e'},
            {"help",           no_argument,       0, 'h'},
            {"issuer",         required_argument, 0, 'i'},
            {"kind",           required_argument, 0, 'k'},
            {"log_level",      required_argument, 0, 'l'},
            {"rating",         required_argument, 0, 'm'},
            {"reel",           required_argument, 0, 'r'},
            {"title",          required_argument, 0, 't'},
            {"root",           required_argument, 0, '1'},
            {"ca",             required_argument, 0, '2'},
            {"signer",         required_argument, 0, '3'},
            {"privatekey",     required_argument, 0, 'p'},
            {"sign",           no_argument,       0, 's'},
            {"version",        no_argument,       0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;
     
        c = getopt_long (argc, argv, "a:b:c:e:svdhi:k:r:l:m:n:t:p:1:2:3:",
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

            case 'a':
               sprintf(context->annotation,"%.80s",optarg);
            break;

            case 'b':
               sprintf(context->basename,"%.80s",optarg);
            break;

            case 'c':
               sprintf(context->creator,"%.80s",optarg);
            break;
     
            case 'd':
               context->digest_flag = 1;
            break;

            case 'e':
               context->entry_point = atoi(optarg);
            break;

            case 'h':
               dcp_usage();
            break;
     
            case 'i':
               sprintf(context->issuer,"%.80s",optarg);
            break;

            case 'k':
               sprintf(context->kind,"%.15s",optarg);
            break;

            case 'l':
               context->log_level = atoi(optarg);
            break;

            case 'm':
               if ( !strcmp(optarg,"G")
                    || !strcmp(optarg,"PG")
                    || !strcmp(optarg,"PG-13")
                    || !strcmp(optarg,"R")
                    || !strcmp(optarg,"NC-17") ) {
                   sprintf(context->rating,"%.5s",optarg);
               } else {
                   sprintf(buffer,"Invalid rating %s\n",optarg);
                   dcp_fatal(context,buffer);
               }
            break;

            case 'n':
               context->duration = atoi(optarg);
            break;
     
            case 'r':
               j = 0;
               optind--;
               while ( optind<argc && strncmp("-",argv[optind],1) != 0) {
				   sprintf(reel_list[x].asset_list[j++].filename,"%s",argv[optind++]);
               }
               reel_list[x++].asset_count = j--;
            break;

            case 's':
                context->xml_sign = 1;
            break;

            case 't':
               sprintf(context->title,"%.80s",optarg);
            break;

            case '1':
               context->root_cert_file = optarg;
               context->xml_sign_certs = 1;
            break;

            case '2':
               context->ca_cert_file = optarg;
               context->xml_sign_certs = 1;
            break;

            case '3':
               context->signer_cert_file = optarg;
               context->xml_sign_certs = 1;
            break;

            case 'p':
               context->private_key_file = optarg;
               context->xml_sign_certs = 1;
            break;

            case 'v':
               version();
            break;

            default:
               dcp_usage();
        }
    }

    /* set log level */
    dcp_set_log_level(context->log_level);

    if (context->log_level > 0) {
        printf("\nOpenDCP XML %s %s\n\n",OPEN_DCP_VERSION,OPEN_DCP_COPYRIGHT);
    }

    /* check cert files */
    if (context->xml_sign_certs) {
        FILE *tp;
        if (context->root_cert_file) {
            tp = fopen(context->root_cert_file,"rb");
            if (tp) {
                fclose(tp);
            } else {
                dcp_fatal(context,"Could not read root certificate");
            }
        } else {
            dcp_fatal(context,"XML digital signature certifcates enabled, but root certificate file not specified");
        }
        if (context->ca_cert_file) {
            tp = fopen(context->ca_cert_file,"rb");
            if (tp) {
                fclose(tp);
            } else {
                dcp_fatal(context,"Could not read ca certificate");
            }
        } else {
            dcp_fatal(context,"XML digital signature certifcates enabled, but ca certificate file not specified");
        }
        if (context->signer_cert_file) {
            tp = fopen(context->signer_cert_file,"rb");
            if (tp) {
                fclose(tp);
            } else {
                dcp_fatal(context,"Could not read signer certificate");
            }
        } else {
            dcp_fatal(context,"XML digital signature certifcates enabled, but signer certificate file not specified");
        }
        if (context->private_key_file) {
            tp = fopen(context->private_key_file,"rb");
            if (tp) {
                fclose(tp);
            } else {
                dcp_fatal(context,"Could not read private key file");
            }
        } else {
            dcp_fatal(context,"XML digital signature certifcates enabled, but private key file not specified");
        }
    }

    /* Generate UUIDs */
    uuid_random(uuid_s);
    sprintf(context->cpl.uuid,"%.36s",uuid_s);
    uuid_random(uuid_s);
    sprintf(context->pkl.uuid,"%.36s",uuid_s);

    /* Generate XML filenames */
    if ( !strcmp(context->basename,"") ) {
        sprintf(context->cpl.filename,"%.40s_cpl.xml",context->cpl.uuid);
        sprintf(context->pkl.filename,"%.40s_pkl.xml",context->pkl.uuid);
    } else {
        sprintf(context->cpl.filename,"%.40s_cpl.xml",context->basename);
        sprintf(context->pkl.filename,"%.40s_pkl.xml",context->basename);
    }

    /* Add and validate reels */
    for (c = 0;c<x;c++) {
        if (add_reel(context, reel_list[c]) != DCP_SUCCESS) {
            sprintf(buffer,"Could not add reel %d to DCP\n",c+1); 
            dcp_fatal(context,buffer);
        }
       if (validate_reel(context, c) != DCP_SUCCESS) {
            sprintf(buffer,"Could validate reel %d\n",c+1); 
            dcp_fatal(context,buffer);
       }
    }

    /* Write XML Files */
    if (write_cpl(context) != DCP_SUCCESS)
        dcp_fatal(context,"Writing composition playlist failed");
    if (write_pkl(context) != DCP_SUCCESS)
        dcp_fatal(context,"Writing packing list failed");
    if (write_volumeindex(context) != DCP_SUCCESS)
        dcp_fatal(context,"Writing volume index failed");
    if (write_assetmap(context) != DCP_SUCCESS)
        dcp_fatal(context,"Writing asset map failed");

    dcp_log(LOG_INFO,"DCP Complete");

    if (context->log_level > 0) {
        printf("\n");
    }

    if ( context != NULL) {
        free(context);
    }

    exit(0);
}
