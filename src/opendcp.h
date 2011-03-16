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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _OPEN_DCP_H_
#define _OPEN_DCP_H_

#define MAX_REELS    30 /* Soft limit */
#define MAX_PATH_LENGTH 4096 
#define MAX_FILENAME_LENGTH 256 
#define MAX_BASENAME_LENGTH 256 
#ifdef WIN32
#define stat _stati64 
#endif

#define MAX_DCP_JPEG_BITRATE 250000000  /* Maximum DCI compliant bit rate for JPEG2000 */
#define MAX_DCP_MPEG_BITRATE  80000000  /* Maximum DCI compliant bit rate for MPEG */

#ifdef OPENDCP_VERSION
static const char *OPEN_DCP_VERSION   = OPENDCP_VERSION; 
#else
static const char *OPEN_DCP_VERSION   = "unknown"; 
#endif

static const char *OPEN_DCP_NAME      = "OpenDCP"; 
static const char *OPEN_DCP_COPYRIGHT = "(c) 2010-2011 Terrence Meiczinger, All Rights Reserved."; 

/* XML Namespaces */
static const char *XML_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";

static const char *NS_CPL[]   = { "None",
                                  "http://www.digicine.com/PROTO-ASDCP-CPL-20040511#",
                                  "http://www.smpte-ra.org/schemas/429-7/2006/CPL"};

static const char *NS_PKL[]   = { "None",
                                  "http://www.digicine.com/PROTO-ASDCP-PKL-20040311#",
                                  "http://www.smpte-ra.org/schemas/429-8/2007/PKL"};

static const char *NS_AM[]    = { "None",
                                  "http://www.digicine.com/PROTO-ASDCP-AM-20040311#",
                                  "http://www.smpte-ra.org/schemas/429-9/2007/AM"};

static const char *NS_CPL_3D[] = { "None",
                                   "http://www.digicine.com/schemas/437-Y/2007/Main-Stereo-Picture-CPL",
                                   "http://www.smpte-ra.org/schemas/429-10/2008/Main-Stereo-Picture-CPL"};

/* Digital signature canonicalization method algorithm */
static const char *DS_DSIG     = "http://www.w3.org/2000/09/xmldsig#"; 

/* Digital signature canonicalization method algorithm */
static const char *DS_CMA      = "http://www.w3.org/TR/2001/REC-xml-c14n-20010315";

/* Digital signature digest method algorithm */
static const char *DS_DMA      =  "http://www.w3.org/2000/09/xmldsig#sha1";

/* Digital signature signature method algorithm */
static const char *DS_SMA[]    = { "None",
                                   "http://www.w3.org/2000/09/xmldsig#rsa-sha1",
                                   "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"};

/* Digital signature transport method algorithm */
static const char *DS_TMA      = "http://www.w3.org/2000/09/xmldsig#enveloped-signature";

static const char *RATING_AGENCY[] = { "None",
                                       "http://www.mpaa.org/2003-ratings",
                                       "http://rcq.qc.ca/2003-ratings"};

static const char *DCP_LOG[] = { "NONE",
                                 "ERROR",
                                 "WARN",
                                 "INFO",
                                 "DEBUG"};

/* defaults */
static const char *DCP_ANNOTATION = "OPENDCP-FILM";
static const char *DCP_TITLE      = "OPENDCP-FILM-TITLE";
static const char *DCP_KIND       = "feature";

enum DCP_ERROR {
    DCP_FATAL   = -1,
    DCP_SUCCESS =  0,
    DCP_WARN    =  1,
    DCP_QUIT    =  2
};

enum LOG_LEVEL {
    LOG_NONE = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};

enum ASSET_CLASS_TYPE {
    ACT_UNKNOWN,
    ACT_PICTURE,
    ACT_SOUND,
    ACT_TIMED_TEXT
};

enum CINEMA_PROFILE {
    DCP_CINEMA2K = 3,
    DCP_CINEMA4K = 4,
};

enum ASSET_ESSENCE_TYPE {
    AET_UNKNOWN,
    AET_MPEG2_VES,
    AET_JPEG_2000,
    AET_PCM_24b_48k,
    AET_PCM_24b_96k,
    AET_TIMED_TEXT,
    AET_JPEG_2000_S
};

enum XML_NAMESPACE {
    XML_NS_UNKNOWN,
    XML_NS_INTEROP,
    XML_NS_SMPTE
};

enum J2K_ENCODER {
    J2K_OPENJPEG,
    J2K_KAKADU
};

typedef struct filelist_t {
    char **in;
    char **out;
    int file_count;
} filelist_t;

typedef struct {
    char           uuid[40];
    char           size[18];
    char           digest[40];
    char           filename[MAX_FILENAME_LENGTH];
} cpl_t;

typedef struct {
    char           uuid[40];
    char           size[18];
    char           filename[MAX_FILENAME_LENGTH];
} pkl_t;

typedef struct {
    char           filename[MAX_FILENAME_LENGTH];
} assetmap_t;

typedef struct {
    char           filename[MAX_FILENAME_LENGTH];
} volindex_t;


typedef struct {
    char           uuid[40];
    int            essence_type;
    int            duration;
    int            intrinsic_duration;
    int            entry_point;
    int            xml_ns;
    int            stereoscopic;
    char           size[18];
    char           name[128];
    char           annotation[128];
    char           edit_rate[20];
    char           frame_rate[20];
    char           sample_rate[20];
    char           aspect_ratio[20];
    char           digest[40];
    char           filename[MAX_FILENAME_LENGTH];
} asset_t;

typedef struct {
    char    annotation[128];
    asset_t MainPicture;
    asset_t MainSound;
    asset_t MainSubtitle;
} reel_t;

typedef struct {
    asset_t asset_list[3];
    int     asset_count;
} asset_list_t;

typedef unsigned char byte_t; 

typedef struct {
    int  cinema_profile;
    int  encoder;
    int  frame_rate;
    int  stereoscopic;
    int  slide;
    int  log_level;
    int  entry_point;
    int  duration;
    int  digest_flag;
    int  ns;
    int  encrypt_header_flag;
    int  key_flag;
    int  delete_intermediate;
    byte_t key_id[16];
    byte_t key_value[16];
    int  write_hmac;
    int  xyz;
    int  quality;
    int  threads;
    int  xml_sign;
    int  xml_sign_certs;
    char *root_cert_file;
    char *ca_cert_file;
    char *signer_cert_file;
    char *private_key_file; 
    char basename[40];
    char dcp_path[MAX_BASENAME_LENGTH];
    char timestamp[30];
    char issuer[80];
    char creator[80];
    char annotation[128];
    char title[80];
    char kind[15];
    char rating[5];
    int  reel_count;
    cpl_t cpl;
    pkl_t pkl;
    assetmap_t assetmap;
    volindex_t volindex;
    reel_t reel[MAX_REELS]; 
    int gamma;
} context_t;

void dcp_log(int level, const char *fmt, ...);
void dcp_fatal(context_t *context, char *error);
void get_timestamp(char *timestamp);
int get_asset_type(asset_t asset);
int get_file_essence_class(char *filename);
char *get_basename(const char *filename);
int validate_reel(context_t *context, int reel);
int add_reel(context_t *context, asset_list_t reel);
void dcp_set_log_level(int log_level);

/* ASDCPLIB Routines */
int read_asset_info(asset_t *asset);
void uuid_random(char *uuid);
int calculate_digest(const char *filename, char *digest);

/* MXF Routines */
int write_mxf(context_t *context, filelist_t *filelist, char *output);

/* XML Routines */
int write_cpl(context_t *context);
int write_pkl(context_t *context);
int write_assetmap(context_t *context);
int write_volumeindex(context_t *context);
char *base64(const unsigned char *data, int length);
char *strip_cert(const char *data);
char *strip_cert_file(char *filename);

/* J2K Routines */
int convert_to_j2k(context_t *context, char *in_file, char *out_file, char *tmp_path);
#endif // _OPEN_DCP_H_
#ifdef __cplusplus
}
#endif
