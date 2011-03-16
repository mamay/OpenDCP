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
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include "opendcp.h"

void cleanup(context_t *context, int exit_flag) {
    int result; 
    fprintf(stderr,"\nCleaning up temporary files... \n");
    if (exit_flag) {
        exit(1);
    }
}

void dcp_fatal(context_t *context, char *error) {
    fprintf(stderr, "[FATAL] %s\n",error);
    cleanup(context,0);
    exit(DCP_FATAL);
}

char *get_basename(const char *filename) {
    char *extension;
    char *base = 0;

    base = (char *)filename;
    extension = strrchr(filename,'.');
    base[(strlen(filename) - strlen(extension))] = '\0';

    return(base);
}

char *base64(const unsigned char *data, int length) {
    int len; 
    char *b_ptr;

    BIO *b64 = BIO_new(BIO_s_mem());
    BIO *cmd = BIO_new(BIO_f_base64());
    b64 = BIO_push(cmd, b64);

    BIO_write(b64, data, length);
    BIO_flush(b64);

    len = BIO_get_mem_data(b64, &b_ptr);
    b_ptr[len-1] = '\0';

    return b_ptr;
}

char *strip_cert(const char *data) {
    int i,len;
    int offset = 28;
    char *buffer;

    len = strlen(data) - 53;
    buffer = (char *)malloc(len);
    memset(buffer,0,(len));
    for (i=0;i<len-2;i++) {
        buffer[i] = data[i+offset];
    }

    return buffer;
}

char *strip_cert_file(char *filename) {
    int i=0;
    char text[5000];
    char *ptr; 
    FILE *fp=fopen(filename, "rb");

    while (!feof(fp)) {
        text[i++] = fgetc(fp);
    }
    text[i-1]='\0';
    ptr = strip_cert(text);
    return(ptr);
}

void get_timestamp(char *timestamp) { 
    time_t time_ptr;
    struct tm *time_struct;
    char buffer[30];

    time(&time_ptr);
    time_struct = localtime(&time_ptr);
    strftime(buffer,30,"%Y-%m-%dT%I:%M:%S+00:00",time_struct);
    sprintf(timestamp,"%.30s",buffer);
}  

int get_asset_type(asset_t asset) {
    switch (asset.essence_type) {
       case AET_MPEG2_VES:
       case AET_JPEG_2000:
       case AET_JPEG_2000_S:
           return ACT_PICTURE;
           break;
       case AET_PCM_24b_48k:
       case AET_PCM_24b_96k:
           return ACT_SOUND;
           break;
       case AET_TIMED_TEXT:
           return ACT_TIMED_TEXT;
           break;
       default:
           return ACT_UNKNOWN;
    }
}

int init_asset( asset_t *asset) {
    memset(asset,0,sizeof(asset_t));

    return DCP_SUCCESS;
}

int validate_reel(context_t *context, int reel) {
    int d = 0;
    int result = 0;
    int duration_mismatch = 0;

    dcp_log(LOG_INFO,"Validating Reel %d\n",reel+1);

    if (context->reel[reel].MainPicture.duration) {
        d = context->reel[reel].MainPicture.duration;
    } else {
        dcp_log(LOG_ERROR,"Main picture asset has no duration");
        return DCP_FATAL;
    }

    /* Validate Duration */
    if (context->reel[reel].MainSound.duration && context->reel[reel].MainSound.duration != d) {
        duration_mismatch = 1;
        if (context->reel[reel].MainSound.duration < d) {
            d = context->reel[reel].MainSound.duration;
        }
    }

    if (context->reel[reel].MainSubtitle.duration && context->reel[reel].MainSubtitle.duration != d) {
        duration_mismatch = 1;
        if (context->reel[reel].MainSubtitle.duration < d) {
            d = context->reel[reel].MainSubtitle.duration;
        }
    }

    if (duration_mismatch) {
            dcp_log(LOG_WARN,"Asset duration mismatch picture: %d / sound: %d, adjusting all durations to shortest asset", context->reel[reel].MainPicture.duration, context->reel[reel].MainSound.duration);
            context->reel[reel].MainPicture.duration = d;
            context->reel[reel].MainSound.duration = d;
            context->reel[reel].MainSubtitle.duration = d;
    }
          
    return result;
}

int add_reel(context_t *context, asset_list_t reel) {
    int result;
    int x;
    FILE *fp;
    char *filename;
    int bp;
    asset_t asset;
    struct stat st;

    dcp_log(LOG_INFO,"Adding Reel");

    /* parse argument and read asset information */
    for (x=0;x<reel.asset_count;x++) {
        filename=reel.asset_list[x].filename;
        init_asset(&asset);
      
        sprintf(asset.filename,"%s",filename);
        sprintf(asset.annotation,"%s",basename(filename));

        /* check if file exists */
        if ((fp = fopen(filename, "r")) == NULL) {
            dcp_log(LOG_ERROR,"add_reel: Could not open file: %s",filename);
            return DCP_FATAL;
        } else {
            fclose (fp);
        }

        /* get file size */
        stat(filename, &st);
        sprintf(asset.size,"%lld", st.st_size);

        /* read asset information */
        dcp_log(LOG_INFO,"Reading %s asset information",filename);

        result = read_asset_info(&asset);
        if (result == DCP_FATAL) {
            dcp_log(LOG_ERROR,"%s is not a proper essence file",filename);
            return DCP_FATAL;
        }

        /* Set duration, if specified */
        if (context->duration) {
            if  (context->duration<asset.duration) {
                asset.duration = context->duration;
            } else {
                dcp_log(LOG_WARN,"Desired duration %d cannot be greater than assset duration %d, ignoring value",context->duration,asset.duration);
            }
        }

        /* Set entry point, if specified */
        if (context->entry_point) {
            if (context->entry_point<asset.duration) {
                asset.entry_point = context->entry_point;
            } else {
                dcp_log(LOG_WARN,"Desired entry point %d cannot be greater than assset duration %d, ignoring value",context->entry_point,asset.duration);
            }
        }

        /* calculate digest */
        calculate_digest(filename,asset.digest);
   
        /* get asset type */
        result = get_asset_type(asset);

        if (result == ACT_PICTURE) {
            context->reel[context->reel_count].MainPicture = asset;
        } else if (result == ACT_SOUND) {
            context->reel[context->reel_count].MainSound = asset;
        } else if (result == ACT_TIMED_TEXT) {
            context->reel[context->reel_count].MainSubtitle = asset;
        } else {
            dcp_log(LOG_ERROR,"%s is not an unknown essence type",filename);
            return DCP_FATAL;
        }
    }

    context->reel_count++;

    return DCP_SUCCESS;
}
