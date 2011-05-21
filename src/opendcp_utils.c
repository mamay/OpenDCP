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
#include <inttypes.h>
#include <time.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include "opendcp.h"

void cleanup(opendcp_t *opendcp, int exit_flag) {
    int result; 
    if (exit_flag) {
        exit(1);
    }
}

void dcp_fatal(opendcp_t *opendcp, char *error) {
    dcp_log(LOG_ERROR, "%s",error);
    cleanup(opendcp,0);
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

int find_seq_offset (char str1[], char str2[]) {
    int i = 0;
    while(1) {
        if(str1[i] != str2[i])
		return i;
	if(str1[i] == '\0' || str2[i] == '\0')
		return 0;
	i++;
    }
}

int find_ext_offset(char str[]) {
    int i = strlen(str);
    while(i) {
        if(str[i] == '.')
            return i;
        i--;
    }
  
    return 0;
}

int check_increment(char *str[], int index,int str_size) {
    int x;
    int array_size, offset, ext_offset, diff_len;

    offset     = find_seq_offset(str[str_size-1],str[0]);
    ext_offset = find_ext_offset(str[0]);
    diff_len   = ext_offset - offset;;

    char *seq = (char *)malloc(diff_len+1);
    strncpy(seq,str[index]+offset,diff_len);
    x = atoi(seq);

    if (x == index) {
        return 0;
    } else {
        return 1;
    }
}

int check_sequence(char str1[],char str2[]) {
    int x,y;
    int offset, ext_offset, diff_len;

    offset     = find_seq_offset(str2,str1);
    ext_offset = find_ext_offset(str1);
    diff_len   = ext_offset - offset;;

    char *seq = (char *)malloc(diff_len+1);

    strncpy(seq,str1+offset,diff_len);
    x = atoi(seq);

    strncpy(seq,str2+offset,diff_len);
    y = atoi(seq);

    if (seq) {
        free(seq);
    }

    if ((y - x) == 1) {
        return 0;
    } else {
        return 1;
    }
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

opendcp_t *create_opendcp() {
    opendcp_t *opendcp;

    /* allocation opendcp memory */
    opendcp = malloc(sizeof(opendcp_t));
    memset(opendcp,0,sizeof (opendcp_t));

    /* initialize opendcp */
    opendcp->log_level = LOG_WARN;
    sprintf(opendcp->issuer,"%.80s %.80s",OPEN_DCP_NAME,OPEN_DCP_VERSION);
    sprintf(opendcp->creator,"%.80s %.80s",OPEN_DCP_NAME, OPEN_DCP_VERSION);
    sprintf(opendcp->annotation,"%.128s",DCP_ANNOTATION);
    sprintf(opendcp->title,"%.80s",DCP_TITLE);
    sprintf(opendcp->kind,"%.15s",DCP_KIND);
    get_timestamp(opendcp->timestamp);
    sprintf(opendcp->assetmap.filename,"%.128s","ASSETMAP.xml");
    sprintf(opendcp->volindex.filename,"%.128s","VOLINDEX.xml");

    return opendcp;
}

int delete_opendcp(opendcp_t *opendcp) {
    if ( opendcp != NULL) {
        free(opendcp);
    }
    return DCP_SUCCESS;
}

int add_pkl(opendcp_t *opendcp) {
    char uuid_s[40];
    int i = opendcp->pkl_count++;

    strcpy(opendcp->pkl[i].issuer,     opendcp->issuer);
    strcpy(opendcp->pkl[i].creator,    opendcp->creator);
    strcpy(opendcp->pkl[i].annotation, opendcp->annotation);
    strcpy(opendcp->pkl[i].timestamp,  opendcp->timestamp);

    /* Generate UUIDs */
    uuid_random(uuid_s);
    sprintf(opendcp->pkl[i].uuid,"%.36s",uuid_s);

    /* Generate XML filename */
    if ( !strcmp(opendcp->basename,"") ) {
        sprintf(opendcp->pkl[i].filename,"%.40s_pkl.xml",opendcp->pkl[i].uuid);
    } else {
        sprintf(opendcp->pkl[i].filename,"%.40s_pkl.xml",opendcp->basename);
    }

    opendcp->pkl_count++;

    return DCP_SUCCESS;
}

int add_cpl(opendcp_t *opendcp, pkl_t *pkl) {
    char uuid_s[40];
    int i = pkl->cpl_count;

    strcpy(pkl->cpl[i].issuer,    opendcp->issuer);
    strcpy(pkl->cpl[i].creator,   opendcp->creator);
    strcpy(pkl->cpl[i].title,     opendcp->title);
    strcpy(pkl->cpl[i].kind,      opendcp->kind);
    strcpy(pkl->cpl[i].rating,    opendcp->rating);
    strcpy(pkl->cpl[i].timestamp, opendcp->timestamp);

    uuid_random(uuid_s);
    sprintf(pkl->cpl[i].uuid,"%.36s",uuid_s);

    /* Generate XML filename */
    if ( !strcmp(opendcp->basename,"") ) {
        sprintf(pkl->cpl[i].filename,"%.40s_cpl.xml",pkl->cpl[i].uuid);
    } else {
        sprintf(pkl->cpl[i].filename,"%.40s_cpl.xml",opendcp->basename);
    }

    pkl->cpl_count++;

    return DCP_SUCCESS;
}

int init_asset(asset_t *asset) {
    memset(asset,0,sizeof(asset_t));

    return DCP_SUCCESS;
}

int validate_reel(opendcp_t *opendcp, cpl_t *cpl, int reel) {
    int d = 0;
    int x,a;
    int result = 0;
    int picture = 0;
    int duration_mismatch = 0;

    dcp_log(LOG_INFO,"Validating Reel %d\n",reel+1);

    a = cpl->reel[reel].asset_count; 

    /* check if reel has a picture track */ 
    for (x=0;x<a;x++) {
        if (cpl->reel[reel].asset[x].essence_class == ACT_PICTURE) {
            picture++;
        }
    }

    if (picture < 1) {
        dcp_log(LOG_ERROR,"Reel %d has no picture track",reel);
        return DCP_FATAL;
    } else if (picture > 1) {
        dcp_log(LOG_ERROR,"Reel %d has multiple picture tracks",reel);
        return DCP_FATAL;
    }

    d = cpl->reel[reel].asset[0].duration;

    /* check durations */
    for (x=0;x<a;x++) {
        if (cpl->reel[reel].asset[x].duration) {
            if (cpl->reel[reel].asset[x].duration != d) {
                duration_mismatch = 1;
                if (cpl->reel[reel].asset[x].duration < d) {
                   d = cpl->reel[reel].asset[x].duration;
                }
            }
        } else {
            dcp_log(LOG_ERROR,"Asset %s has no duration",cpl->reel[reel].asset[x].filename);
           return DCP_FATAL;
        }
    }

    if (duration_mismatch) {
       dcp_log(LOG_WARN,"Asset duration mismatch, adjusting all durations to shortest asset duration of %d frames", d);
        for (x=0;x<a;x++) {
            cpl->reel[reel].asset[x].duration = d;
        }
    }
          
    return result;
}

int add_reel(opendcp_t *opendcp, cpl_t *cpl, asset_list_t reel) {
    int result;
    int x,r;
    FILE *fp;
    char *filename;
    int bp;
    asset_t asset;
    struct stat st;
    char uuid_s[40];

    dcp_log(LOG_INFO,"Adding Reel");

    r = cpl->reel_count; 

    /* add reel uuid */
    uuid_random(uuid_s);
    sprintf(cpl->reel[r].uuid,"%.36s",uuid_s);

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
        sprintf(asset.size,"%"PRIu64, st.st_size);

        /* read asset information */
        dcp_log(LOG_INFO,"Reading %s asset information",filename);

        result = read_asset_info(&asset);

        if (result == DCP_FATAL) {
            dcp_log(LOG_ERROR,"%s is not a proper essence file",filename);
            return DCP_FATAL;
        }

        if (x == 0) {
            opendcp->ns = asset.xml_ns;
        } else {
            if (opendcp->ns != asset.xml_ns) {
                dcp_log(LOG_ERROR,"Warning DCP specification mismatch in assets. Please make sure all assets are MXF Interop or SMPTE");
                return DCP_FATAL;
            }
        }

        /* Set duration, if specified */
        if (opendcp->duration) {
            if  (opendcp->duration<asset.duration) {
                asset.duration = opendcp->duration;
            } else {
                dcp_log(LOG_WARN,"Desired duration %d cannot be greater than assset duration %d, ignoring value",opendcp->duration,asset.duration);
            }
        }

        /* Set entry point, if specified */
        if (opendcp->entry_point) {
            if (opendcp->entry_point<asset.duration) {
                asset.entry_point = opendcp->entry_point;
            } else {
                dcp_log(LOG_WARN,"Desired entry point %d cannot be greater than assset duration %d, ignoring value",opendcp->entry_point,asset.duration);
            }
        }

        /* calculate digest */
        calculate_digest(filename,asset.digest);
   
        /* get asset type */
        result = get_asset_type(asset);

        /* add asset to cpl */
        cpl->reel[r].asset[x] = asset;
        cpl->reel[r].asset_count++;
    }

    cpl->reel_count++;

    return DCP_SUCCESS;
}
