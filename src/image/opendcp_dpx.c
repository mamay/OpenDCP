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
} dpx_file_info_t;

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
    uint16_t    orientation;          /* image orientation */
    uint16_t    element_number;       /* number of image elements */
    uint32_t    pixels_per_line;      /* or x value */
    uint32_t    lines_per_image_ele;  /* or y value, per element */
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

uint8_t  r_8(uint8_t value, int endian) {
    if (endian) {
        return (value << 8) | (value >> 8);
    } else {
        return value;
    }
}

uint16_t r_16(uint16_t value, int endian) {
    if (endian) {
        return (uint16_t)((value & 0xFFU) << 8 | (value & 0xFF00U) >> 8);
    } else {
        return value;
    }
}

uint32_t r_32(uint32_t value, int endian) {
  if (endian) {
      return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
             (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
  } else {
      return value;
  }
}

static inline unsigned make_16bit(unsigned value) {
    value &= 0xFFC0;
    return value + (value >> 10);
}

int read_dpx(odcp_image_t **image_ptr, const char *infile, int fd) {
    dpx_file_info_t    *dpx_info;
    dpx_image_header_t *dpx_header;
    FILE *dpx;
    odcp_image_t *image = 00;
    int index,w,h,image_size,i,j,endian;
    unsigned short int bps = 0;
    unsigned short int spp = 0;

    dpx_info = malloc(sizeof(dpx_file_info_t)+1);
    dpx_header = malloc(sizeof(dpx_image_header_t)+1);

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

    //printf("file_header size: %d\n",sizeof(dpx_file_info_t));
    //printf("file_image header size: %d\n",sizeof(dpx_image_header_t));
    //printf("orientation header size: %d\n",sizeof(dpx_image_orientation_t));

    fread(dpx_info,sizeof(dpx_file_info_t),1,dpx);
    fread(dpx_header,sizeof(dpx_image_header_t),1,dpx);
    
    if (dpx_info->magic_num == MAGIC_NUMBER) {
        endian = 0;
        //printf("magic_num: %x (big endian)\n",dpx_info->magic_num);
    } else if (r_32(dpx_info->magic_num, 1) == MAGIC_NUMBER) {
        endian = 1;
        //printf("magic_num: %x (little endian)\n",dpx_info->magic_num);
    } else {
         dcp_log(LOG_ERROR,"%s is not a valid DPX file", infile);
         return DCP_FATAL;
    }

    bps = dpx_header->image_element[0].bit_size;

    switch (dpx_header->image_element[0].descriptor) {
        case 50:      // RGB
            spp = 3;
            break;
        case 51:      // RGBA
            spp = 4;
            break;
        default:
            dcp_log(LOG_ERROR, "Unsupported image descriptor: %d\n", dpx_header->image_element[0].descriptor);
            return DCP_FATAL;
    }

    w = r_32(dpx_header->pixels_per_line, endian);
    h = r_32(dpx_header->lines_per_image_ele, endian);

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

    fseek(dpx, r_32(dpx_info->offset, endian), SEEK_SET);

    printf("%dx%d (%d)\n",w,h,sizeof(uint32_t));
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
        for (i=0; i<image_size; i++) { 
            for (j=0; j<spp; j++) {
                fread(&data,sizeof(data),1,dpx);
                if (j==0) {
                    data = r_32(data, endian) >> 16;
                    image->component[j].data[i] = (uint16_t)((data & 0xFFF0) >> 4); 
                    //image->component[j].data[i] = (uint16_t)((data & 0xFFF0) >> 4) | ((data & 0x00CF) >> 6); 
                } else if (j==1) {
                    data = r_32(data, endian) >> 6;
                    image->component[j].data[i] = (uint16_t)((data & 0xFFF0) >> 4); 
                    //image->component[j].data[i] = ((data & 0xFFF0) >> 4) | ((data & 0x00CF) >> 6); 
                } else if (j==2) {
                    data = r_32(data, endian) << 4;
                    image->component[j].data[i] = (uint16_t)((data & 0xFFF0) >> 4); 
                    //image->component[j].data[i] = ((data & 0xFFF0) >> 4) | ((data & 0x00CF) >> 6); 
                }
             }
                 
        //for (i=0; i<image_size; i++) { 
            //fread(&data,4,1,dpx);
            //image->component[0].data[i] = (data[0] << 2 | (data[1] & 0xC0)) << 2;
            //image->component[1].data[i] = ((data[1] & 0x3F) << 4 | (data[2] & 0xF0) >> 6) << 2;
            //image->component[2].data[i] = ((data[2] & 0x0F) << 6 | (data[3] & 0xFC) << 2) << 2;
            //printf("%x|%x|%x|%x -> ",data[0],data[1],data[2],data[3]);
            //printf("%x|%x|%x|\n",image->component[0].data[i],image->component[1].data[i],image->component[2].data[i]);
        }
        printf("(%d,%d):\n",i,j); 
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
    free(dpx_info);
    free(dpx_header);

    dcp_log(LOG_DEBUG,"DPX read complete");
    *image_ptr = image;

    return 0;
}
