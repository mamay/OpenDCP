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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "../opendcp.h"
#include "opendcp_image.h"

#define MAGIC_NUMBER 0x53445058
#define DEFAULT_GAMMA 1.0
#define DEFAULT_BLACK_POINT 95
#define DEFAULT_WHITE_POINT 685 

typedef enum {
    DPX_RGB =  50,
    DPX_RGBA = 51,
    DPX_ABGR = 52,
    DPX_YUV422 = 100,
    DPX_YUV444 = 102,
} dpx_color_enum;

typedef struct {
    uint32_t magic_num;        /* magic number 0x53445058 (SDPX) or 0x58504453 (XPDS) */
    uint32_t offset;           /* offset to image data in bytes */
    char     version[8];       /* which header format version is being used (v1.0)*/
    uint32_t file_size;        /* file size in bytes */
    uint32_t ditto_key;        /* read time short cut - 0 = same, 1 = new */
    uint32_t gen_hdr_size;     /* generic header length in bytes */
    uint32_t ind_hdr_size;     /* industry header length in bytes */
    uint32_t user_data_size;   /* user-defined data length in bytes */
    char     file_name[100];   /* iamge file name */
    char     create_time[24];  /* file creation date "yyyy:mm:dd:hh:mm:ss:LTZ" */
    char     creator[100];     /* file creator's name */
    char     project[200];     /* project name */
    char     copyright[200];   /* right to use or copyright info */
    uint32_t key;              /* encryption ( FFFFFFFF = unencrypted ) */
    char     reserved[104];    /* reserved field TBD (need to pad) */
} dpx_file_header_t;

typedef struct {
    uint32_t    data_sign;         /* data sign (0 = unsigned, 1 = signed ) */
                                   /* "Core set images are unsigned" */
    uint32_t    ref_low_data;      /* reference low data code value */
    float       ref_low_quantity;  /* reference low quantity represented */
    uint32_t    ref_high_data;     /* reference high data code value */
    float       ref_high_quantity; /* reference high quantity represented */
    uint8_t     descriptor;        /* descriptor for image element */
    uint8_t     transfer;          /* transfer characteristics for element */
    uint8_t     colorimetric;      /* colormetric specification for element */
    uint8_t     bit_size;          /* bit size for element */
    uint16_t    packing;           /* packing for element */
    uint16_t    encoding;          /* encoding for element */
    uint32_t    data_offset;       /* offset to data of element */
    uint32_t    eol_padding;       /* end of line padding used in element */
    uint32_t    eo_image_padding;  /* end of image padding used in element */
    char        description[32];   /* description of element */
} dpx_image_element_t;

typedef struct { 
    uint16_t    orientation;              /* image orientation */
    uint16_t    element_number;           /* number of image elements */
    uint32_t    pixels_per_line;          /* or x value */
    uint32_t    lines_per_image_ele;      /* or y value, per element */
    dpx_image_element_t image_element[8];
    char        reserved[52];             /* reserved for future use (padding) */
} dpx_image_header_t;

typedef struct { 
    uint32_t    x_offset;               /* X offset */
    uint32_t    y_offset;               /* Y offset */
    int32_t     x_center;               /* X center */
    int32_t     y_center;               /* Y center */
    uint32_t    x_orig_size;            /* X original size */
    uint32_t    y_orig_size;            /* Y original size */
    char        file_name[100];         /* source image file name */
    char        creation_time[24];      /* source image creation date and time */
    char        input_dev[32];          /* input device name */
    char        input_serial[32];       /* input device serial number */
    uint16_t    border[4];              /* border validity (XL, XR, YT, YB) */
    uint32_t    pixel_aspect[2];        /* pixel aspect ratio (H:V) */
    uint8_t     reserved[28];           /* reserved for future use (padding) */
} dpx_image_orientation_t;

typedef struct { 
    char        film_mfg_id[2];    /* film manufacturer ID code (2 digits from film edge code) */
    char        film_type[2];      /* file type (2 digits from film edge code) */
    char        offset[2];         /* offset in perfs (2 digits from film edge code)*/
    char        prefix[6];         /* prefix (6 digits from film edge code) */
    char        count[4];          /* count (4 digits from film edge code)*/
    char        format[32];        /* format (i.e. academy) */
    uint32_t    frame_position;    /* frame position in sequence */
    uint32_t    sequence_len;      /* sequence length in frames */
    uint32_t    held_count;        /* held count (1 = default) */
    float       frame_rate;        /* frame rate of original in frames/sec */
    float       shutter_angle;     /* shutter angle of camera in degrees */
    char        frame_id[32];      /* frame identification (i.e. keyframe) */
    char        slate_info[100];   /* slate information */
    char        reserved[56];      /* reserved for future use (padding) */
} dpx_film_header_t;

typedef struct { 
    uint32_t    time_code;           /* SMPTE time code */
    uint32_t    userBits;            /* SMPTE user bits */
    char        interlace;           /* interlace ( 0 = noninterlaced, 1 = 2:1 interlace*/
    char        field_num;           /* field number */
    char        video_signal;        /* video signal standard (table 4)*/
    char        unused;              /* used for byte alignment only */
    float       hor_sample_rate;     /* horizontal sampling rate in Hz */
    float       ver_sample_rate;     /* vertical sampling rate in Hz */
    float       frame_rate;          /* temporal sampling rate or frame rate in Hz */
    float       time_offset;         /* time offset from sync to first pixel */
    float       gamma;               /* gamma value */
    float       black_level;         /* black level code value */
    float       black_gain;          /* black gain */
    float       break_point;         /* breakpoint */
    float       white_level;         /* reference white level code value */
    float       integration_times;   /* integration time(s) */
    char        reserved[76];        /* reserved for future use (padding) */
} dpx_tv_header_t;

int lut[1024] = {
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,    8,   12,   16,   20,   25, 
  29,   33,   37,   42,   46,   50,   54,   59,   63,   67,   72,   76,   80,   84,   89,   93,   97, 
 102,  106,  111,  115,  119,  124,  128,  132,  137,  141,  146,  150,  154,  159,  163,  168,  172, 
 177,  181,  186,  190,  195,  199,  204,  208,  213,  217,  222,  226,  231,  235,  240,  244,  249, 
 254,  258,  263,  267,  272,  277,  281,  286,  291,  295,  300,  305,  309,  314,  319,  323,  328, 
 333,  337,  342,  347,  352,  356,  361,  366,  371,  375,  380,  385,  390,  395,  399,  404,  409, 
 414,  419,  424,  428,  433,  438,  443,  448,  453,  458,  463,  468,  473,  477,  482,  487,  492, 
 497,  502,  507,  512,  517,  522,  527,  532,  537,  542,  547,  553,  558,  563,  568,  573,  578, 
 583,  588,  593,  598,  604,  609,  614,  619,  624,  629,  635,  640,  645,  650,  655,  661,  666, 
 671,  676,  682,  687,  692,  697,  703,  708,  713,  719,  724,  729,  735,  740,  745,  751,  756, 
 762,  767,  772,  778,  783,  789,  794,  800,  805,  811,  816,  822,  827,  833,  838,  844,  849, 
 855,  860,  866,  871,  877,  882,  888,  894,  899,  905,  911,  916,  922,  927,  933,  939,  944, 
 950,  956,  962,  967,  973,  979,  985,  990,  996, 1002, 1008, 1013, 1019, 1025, 1031, 1037, 1042, 
1048, 1054, 1060, 1066, 1072, 1078, 1084, 1090, 1095, 1101, 1107, 1113, 1119, 1125, 1131, 1137, 1143, 
1149, 1155, 1161, 1167, 1173, 1179, 1185, 1192, 1198, 1204, 1210, 1216, 1222, 1228, 1234, 1240, 1247, 
1253, 1259, 1265, 1271, 1278, 1284, 1290, 1296, 1303, 1309, 1315, 1321, 1328, 1334, 1340, 1347, 1353, 
1359, 1366, 1372, 1378, 1385, 1391, 1398, 1404, 1410, 1417, 1423, 1430, 1436, 1443, 1449, 1456, 1462, 
1469, 1475, 1482, 1488, 1495, 1501, 1508, 1515, 1521, 1528, 1534, 1541, 1548, 1554, 1561, 1568, 1574, 
1581, 1588, 1595, 1601, 1608, 1615, 1622, 1628, 1635, 1642, 1649, 1655, 1662, 1669, 1676, 1683, 1690, 
1697, 1704, 1710, 1717, 1724, 1731, 1738, 1745, 1752, 1759, 1766, 1773, 1780, 1787, 1794, 1801, 1808, 
1815, 1822, 1829, 1837, 1844, 1851, 1858, 1865, 1872, 1879, 1887, 1894, 1901, 1908, 1915, 1923, 1930, 
1937, 1944, 1952, 1959, 1966, 1974, 1981, 1988, 1996, 2003, 2010, 2018, 2025, 2033, 2040, 2048, 2055, 
2062, 2070, 2077, 2085, 2092, 2100, 2108, 2115, 2123, 2130, 2138, 2145, 2153, 2161, 2168, 2176, 2184, 
2191, 2199, 2207, 2214, 2222, 2230, 2237, 2245, 2253, 2261, 2269, 2276, 2284, 2292, 2300, 2308, 2316, 
2323, 2331, 2339, 2347, 2355, 2363, 2371, 2379, 2387, 2395, 2403, 2411, 2419, 2427, 2435, 2443, 2451, 
2459, 2467, 2476, 2484, 2492, 2500, 2508, 2516, 2525, 2533, 2541, 2549, 2557, 2566, 2574, 2582, 2591, 
2599, 2607, 2616, 2624, 2632, 2641, 2649, 2658, 2666, 2674, 2683, 2691, 2700, 2708, 2717, 2725, 2734, 
2742, 2751, 2760, 2768, 2777, 2785, 2794, 2803, 2811, 2820, 2829, 2837, 2846, 2855, 2863, 2872, 2881, 
2890, 2899, 2907, 2916, 2925, 2934, 2943, 2952, 2960, 2969, 2978, 2987, 2996, 3005, 3014, 3023, 3032, 
3041, 3050, 3059, 3068, 3077, 3087, 3096, 3105, 3114, 3123, 3132, 3141, 3151, 3160, 3169, 3178, 3187, 
3197, 3206, 3215, 3225, 3234, 3243, 3253, 3262, 3271, 3281, 3290, 3300, 3309, 3319, 3328, 3338, 3347, 
3357, 3366, 3376, 3385, 3395, 3404, 3414, 3424, 3433, 3443, 3453, 3462, 3472, 3482, 3492, 3501, 3511, 
3521, 3531, 3540, 3550, 3560, 3570, 3580, 3590, 3600, 3610, 3620, 3630, 3640, 3650, 3660, 3670, 3680, 
3690, 3700, 3710, 3720, 3730, 3740, 3750, 3760, 3771, 3781, 3791, 3801, 3812, 3822, 3832, 3842, 3853, 
3863, 3873, 3884, 3894, 3905, 3915, 3925, 3936, 3946, 3957, 3967, 3978, 3988, 3999, 4009, 4020, 4031, 
4041, 4052, 4062, 4073, 4084, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 
4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095 }; 

static inline uint8_t  r_8(uint8_t value, int endian) {
    if (endian) {
        return (value << 8) | (value >> 8);
    } else {
        return value;
    }
}

static inline uint16_t r_16(uint16_t value, int endian) {
    if (endian) {
        return (uint16_t)((value & 0xFFU) << 8 | (value & 0xFF00U) >> 8);
    } else {
        return value;
    }
}

static inline uint32_t r_32(uint32_t value, int endian) {
  if (endian) {
      return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
             (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
  } else {
      return value;
  }
}

static double dpx_log(int in, int white, float gamma) {
    return (pow((pow(10.0, (in - white) * (0.002 / 1.7))),1.0/1.7));
}

int dpx_log_to_lin(int value) {
    double gain;
    double offset;
    double scale;

    gain = 4095.0 / (1 - dpx_log(DEFAULT_BLACK_POINT, DEFAULT_WHITE_POINT, DEFAULT_GAMMA));
    offset = gain - 4095;

    if (value <= DEFAULT_BLACK_POINT) {
        return 0;
    }

    if (value < DEFAULT_WHITE_POINT) {
        double f_i;
        f_i = dpx_log(value, DEFAULT_WHITE_POINT, DEFAULT_GAMMA);
        printf("%f\n",f_i);
        return ((int)((f_i*gain)-offset));
    }
    
    if (value <= 4095) {
        return 4095;
    }
}

void buildLut() {
    int x;

    for (x=0;x<1024;x++) {
       dpx_log_to_lin(x);
    }
}

int read_dpx(odcp_image_t **image_ptr, const char *infile, int fd) {
    dpx_file_header_t         *dpx_file;
    dpx_image_header_t        *dpx_image;
    dpx_image_orientation_t   *dpx_origin;
    dpx_film_header_t         *dpx_film;
    dpx_tv_header_t           *dpx_tv;
    FILE *dpx;
    odcp_image_t *image = 00;
    int w,h,image_size,i,j,endian, logarithmic;
    unsigned short int bps = 0;
    unsigned short int spp = 0;

    dpx_file   = malloc(sizeof(dpx_file_header_t)+1);
    dpx_image  = malloc(sizeof(dpx_image_header_t)+1);
    dpx_origin = malloc(sizeof(dpx_image_orientation_t)+1);
    dpx_film   = malloc(sizeof(dpx_film_header_t)+1);
    dpx_tv     = malloc(sizeof(dpx_tv_header_t)+1);

    /* open tiff using filename or file descriptor */
    dcp_log(LOG_DEBUG,"Opening dpx file %s",infile);
    if (fd == 0) {
        dpx = fopen(infile, "rb");
    } else {
        dpx = (FILE *)infile;
    }

    if (!dpx) {
        dcp_log(LOG_ERROR,"Failed to open %s for reading", infile);
        return DCP_FATAL;
    }

    fread(dpx_file,sizeof(dpx_file_header_t),1,dpx);
    fread(dpx_image,sizeof(dpx_image_header_t),1,dpx);
    fread(dpx_origin,sizeof(dpx_image_orientation_t),1,dpx);
    fread(dpx_film,sizeof(dpx_film_header_t),1,dpx);
    fread(dpx_tv,sizeof(dpx_tv_header_t),1,dpx);
    
    if (dpx_file->magic_num == MAGIC_NUMBER) {
        endian = 0;
    } else if (r_32(dpx_file->magic_num, 1) == MAGIC_NUMBER) {
        endian = 1;
    } else {
         dcp_log(LOG_ERROR,"%s is not a valid DPX file", infile);
         return DCP_FATAL;
    }

    bps = dpx_image->image_element[0].bit_size;

    if (bps < 8 || bps > 16) {
        dcp_log(LOG_ERROR, "%d-bit depth is not supported\n",bps);
        return DCP_FATAL;
    }

    switch (dpx_image->image_element[0].descriptor) {
        case DPX_RGB:      // RGB
            spp = 3;
            break;
        case DPX_RGBA:      // RGBA
            spp = 4;
            break;
        default:
            dcp_log(LOG_ERROR, "Unsupported image descriptor: %d\n", dpx_image->image_element[0].descriptor);
            return DCP_FATAL;
            break;
    }

    dcp_log(LOG_DEBUG,"dpx desc:\t%x",dpx_image->image_element[0].descriptor);
    dcp_log(LOG_DEBUG,"dpx color:\t%x",dpx_image->image_element[0].colorimetric);
    dcp_log(LOG_DEBUG,"dpx transfer:\t%x",dpx_image->image_element[0].transfer);

    dcp_log(LOG_DEBUG,"dpx film type:\t%s",dpx_film->film_type);
    dcp_log(LOG_DEBUG,"dpx tv gamma:\t%f",dpx_tv->gamma);

    double gain = 4095.0 / (1 - dpx_log(DEFAULT_BLACK_POINT, DEFAULT_WHITE_POINT, DEFAULT_GAMMA));
    double offset = gain - 4095;

    dcp_log(LOG_DEBUG,"dpx gain:\t%f",gain);
    dcp_log(LOG_DEBUG,"dpx offset:\t%f",offset);

    switch (dpx_image->image_element[0].transfer) {
        case 0:
            logarithmic = 1;
            break;
        case 1:
        case 2:
            logarithmic = 0;
            break;
        case 3:
            logarithmic = 1;
            break;
        default:
            dcp_log(LOG_ERROR, "Unsupported transfer characteristic: %d\n", dpx_image->image_element[0].transfer);
            //return DCP_FATAL;
            break;
    }
            logarithmic = 1;

    w = r_32(dpx_image->pixels_per_line, endian);
    h = r_32(dpx_image->lines_per_image_ele, endian);

    image_size = w * h;

    /* create the image */
    dcp_log(LOG_DEBUG,"Allocating odcp image");
    image = odcp_image_create(3,image_size);
    dcp_log(LOG_DEBUG,"Image allocated");

    /* RGB color */
    image->bpp          = 12;
    image->precision    = 12;
    image->n_components = 3;
    image->signed_bit   = 0;
    image->dx           = 1;
    image->dy           = 1;
    image->w            = w;
    image->h            = h;

    /* set image offset and reference grid */
    image->x0 = 0;
    image->y0 = 0;
    image->x1 = !image->x0 ? (w - 1) * image->dx + 1 : image->x0 + (w - 1) * image->dx + 1;
    image->y1 = !image->y0 ? (h - 1) * image->dy + 1 : image->y0 + (h - 1) * image->dy + 1;

    fseek(dpx, r_32(dpx_file->offset, endian), SEEK_SET);

    /* 8 bits per pixel */
    if (bps == 8) {
        uint8_t  data;
        for (i=0; i<image_size; i++) { 
            for (j=0; j<spp; j++) {
                data = fgetc(dpx);
                if (j < 3) { // Skip alpha channel 
                    image->component[j].data[i] = data << 4;
                } 
            }
        }
    /* 10 bits per pixel */
    } else if (bps == 10) {
        uint32_t data;
        uint32_t comp;
        for (i=0; i<image_size; i++) { 
            fread(&data,sizeof(data),1,dpx);
            for (j=0; j<spp; j++) {
                if (j==0) {
                    comp = r_32(data, endian) >> 16;
                    image->component[j].data[i] = ((comp & 0xFFF0) >> 4) | ((comp & 0x00CF) >> 6); 
                } else if (j==1) {
                    comp = r_32(data, endian) >> 6;
                    image->component[j].data[i] = ((comp & 0xFFF0) >> 4) | ((comp & 0x00CF) >> 6); 
                } else if (j==2) {
                    comp = r_32(data, endian) << 4;
                    image->component[j].data[i] = ((comp & 0xFFF0) >> 4) | ((comp & 0x00CF) >> 6); 
                }
                if (logarithmic) {
                    image->component[j].data[i] = lut[((image->component[j].data[i] >> 2))]; 
                }
            }
        }
    /* 12 bits per pixel */
    } else if (bps == 12) {
        uint8_t  data[2];
        for (i=0; i<image_size; i++) { 
            for (j=0; j<spp; j++) {
                data[0] = fgetc(dpx);
                data[1] = fgetc(dpx);
                if (j < 3) {
                    image->component[j].data[i] = (data[!endian]<<4) | (data[!endian]>>4);
                }
            }
        }
    /* 16 bits per pixel */
    } else if ( bps == 16) {
        uint8_t  data[2];
        for (i=0; i<image_size; i++) { 
            for (j=0; j<spp; j++) {
                data[0] = fgetc(dpx);
                data[1] = fgetc(dpx);
                if (j < 3) { // Skip alpha channel
                    image->component[j].data[i] = ( data[!endian] << 8 ) | data[!endian];
                    image->component[j].data[i] = (image->component[j].data[i]) >> 4; 
                }
            }
        }
    }

    fclose(dpx);
    free(dpx_file);
    free(dpx_image);
    free(dpx_origin);
    free(dpx_film);
    free(dpx_tv);

    dcp_log(LOG_DEBUG,"DPX read complete");
    *image_ptr = image;

    return 0;
}
