/*
    OpenDCP: Builds Digital Cinema Packages
    Copyright (c) 2010-2012 Terrence Meiczinger, All Rights Reserved

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
#include "opendcp.h"
#include "opendcp_image.h"

#define MAGIC_NUMBER 0x4D42

typedef enum {
    BMP_R         = 0,
    BMP_G         = 1,
    BMP_B         = 2
} bmp_color_enum;

typedef enum {
    BMP_TOP         = 0,
    BMP_BOTTOM      = 1,
} bmp_row_order_enum;

typedef enum {
    BMP_RGB       = 0,          /* no compression                 */
    BMP_RLE8      = 1,          /* 8-bit run-length-encoded       */
    BMP_RLE4      = 2,          /* 4-bit run-length-encoded       */
    BMP_BITFIELDS = 3,          /* RGB bitmap                     */
    BMP_JPEG      = 4,          /* JPEG (not supported)           */
    BMP_PNG       = 5           /* PNG (not supported)            */
} bmp_compression_enum;

typedef struct {
    uint16_t magic_num;         /* magic number 0x4d42            */
} bmp_magic_num_t;

typedef struct {
    uint32_t size;              /* file size in bytes             */
    uint32_t reserved;          /* rervered1, reserved2           */
    uint32_t offset;            /* offset to image data in bytes  */
} bmp_file_header_t;

typedef struct {
    uint32_t header_size;        /* header size in bytes          */
    int32_t  width;              /* width of image                */
    int32_t  height;             /* height of image               */
    uint16_t planes;             /* number of color components    */
    uint16_t bpp;                /* bits per pixel                */
    uint32_t compression;        /* compression method            */
    uint32_t image_size;         /* image size in bytes           */
    int32_t  x_ppm;              /* x pels                        */
    int32_t  y_ppm;              /* y pels                        */
    uint32_t colors_used;        /* number of colors              */
    uint32_t colors_important;   /* number of important colors    */
} bmp_image_header_t;

typedef struct {
    bmp_file_header_t        file;
    bmp_image_header_t       image;
    int                      row_order;
} bmp_image_t;

/* BMPs are sometimes bottom to top, so invert if needed */
static inline int invert_row(bmp_image_t bmp, int index) {
    int w = bmp.image.width;
    int h = bmp.image.height;

    if (bmp.row_order == BMP_BOTTOM) {
      index = (h - index / w) * w + ((index % w) - w);
    }

    return index;
}

void print_bmp_header(bmp_image_t *bmp) {
    dcp_log(LOG_DEBUG,"%-15.15s: file size:    %d","read_bmp",bmp->file.size);
    dcp_log(LOG_DEBUG,"%-15.15s: data offset:  %d","read_bmp",bmp->file.offset);

    dcp_log(LOG_DEBUG,"%-15.15s: header_size:  %d","read_bmp",bmp->image.header_size);
    dcp_log(LOG_DEBUG,"%-15.15s: width:        %d","read_bmp",bmp->image.width);
    dcp_log(LOG_DEBUG,"%-15.15s: height:       %d","read_bmp",bmp->image.height);
    dcp_log(LOG_DEBUG,"%-15.15s: planes:       %d","read_bmp",bmp->image.planes);
    dcp_log(LOG_DEBUG,"%-15.15s: bpp:          %d","read_bmp",bmp->image.bpp);
    dcp_log(LOG_DEBUG,"%-15.15s: compression:  %d","read_bmp",bmp->image.compression);
    dcp_log(LOG_DEBUG,"%-15.15s: size:         %d","read_bmp",bmp->image.image_size);
    dcp_log(LOG_DEBUG,"%-15.15s: x:            %d","read_bmp",bmp->image.x_ppm);
    dcp_log(LOG_DEBUG,"%-15.15s: y:            %d","read_bmp",bmp->image.y_ppm);
    dcp_log(LOG_DEBUG,"%-15.15s: colors_used:  %d","read_bmp",bmp->image.colors_used);
    dcp_log(LOG_DEBUG,"%-15.15s: colors_impor: %d","read_bmp",bmp->image.colors_important);

    dcp_log(LOG_DEBUG,"%-15.15s: row_order:    %d","read_bmp",bmp->row_order);
}

int read_bmp(odcp_image_t **image_ptr, const char *infile, int fd) {
    bmp_magic_num_t magic;
    bmp_image_t     bmp;
    FILE            *bmp_fp;
    odcp_image_t    *image = 00;
    int             pixels = 0;
    int i,w,h;

    /* open bmp using filename or file descriptor */
    dcp_log(LOG_DEBUG,"%-15.15s: opening bmp file %s","read_bmp",infile);

    if (fd == 0) {
        bmp_fp = fopen(infile, "rb");
    } else {
        bmp_fp = (FILE *)infile;
    }

    if (!bmp_fp) {
        dcp_log(LOG_ERROR,"%-15.15s: opening bmp file %s","read_bmp",infile);
        return DCP_FATAL;
    }

    if (fread(&magic,sizeof(bmp_magic_num_t),1,bmp_fp) < 0) {
        dcp_log(LOG_ERROR,"%-15.15s: failed to read magic number","read_bmp");
        return DCP_FATAL;
    }
    
    if (fread(&bmp,sizeof(bmp_image_t),1,bmp_fp) < 0) {
        dcp_log(LOG_ERROR,"%-15.15s: failed to header","read_bmp");
        return DCP_FATAL;
    }

    if (magic.magic_num != MAGIC_NUMBER) {
         dcp_log(LOG_ERROR,"%s is not a valid BMP file", infile);
         return DCP_FATAL;
    }

    if (bmp.image.height < 0) {
        bmp.row_order = BMP_TOP;
    } else {
        bmp.row_order = BMP_BOTTOM;
    }

    w = bmp.image.width;
    h = abs(bmp.image.height);
    pixels = w * h;

    print_bmp_header(&bmp);
    
    switch (bmp.image.compression) {
        case BMP_RGB:
            break;
        case BMP_RLE8:
        case BMP_RLE4:
        case BMP_BITFIELDS:
        case BMP_JPEG:
        case BMP_PNG:
        default:
            dcp_log(LOG_ERROR, "Unsupported image compression: %d\n", bmp.image.compression);
            return DCP_FATAL;
            break;
    }

    /* apparently, some applications don't fill in the image size */
    if (bmp.image.image_size == 0) {
        bmp.image.image_size = bmp.file.size - sizeof(bmp_magic_num_t) - sizeof(bmp_image_t);
    }

    if (bmp.image.bpp < 24 || bmp.image.bpp > 32) {
        dcp_log(LOG_ERROR, "%d-bit depth is not supported.",bmp.image.bpp);
        return DCP_FATAL;
    }
  
    /* create the image */
    dcp_log(LOG_DEBUG,"%-15.15s: allocating odcp image","read_bmp");
    image = odcp_image_create(3,w,h);
    dcp_log(LOG_DEBUG,"%-15.15s: image allocated","read_bmp");

    fseek(bmp_fp, bmp.file.offset, SEEK_SET);

    /* RGB(A) */
    if (bmp.image.compression == BMP_RGB) {
        /* 16-bits per pixel */
        if (bmp.image.bpp == 16 ) {
            uint8_t data[2];
            for (i=0; i<pixels; i++) { 
                fread(&data,sizeof(data),1,bmp_fp);
                int p = invert_row(bmp, i);
                image->component[BMP_B].data[p] = data[0] << 2;
                image->component[BMP_G].data[p] = data[0] << 4;
                image->component[BMP_R].data[p] = data[1] << 2;
            }
        /* 24-bits per pixel */
        } else if (bmp.image.bpp == 24 ) {
            uint8_t data[3];
            for (i=0; i<pixels; i++) { 
                fread(&data,sizeof(data),1,bmp_fp);
                int p = invert_row(bmp, i);
                image->component[BMP_B].data[p] = data[0] << 4;
                image->component[BMP_G].data[p] = data[1] << 4;
                image->component[BMP_R].data[p] = data[2] << 4;
            }
        /* 32-bits per pixel */
        } else if (bmp.image.bpp == 32 ) {
            uint8_t data[4];
            for (i=0; i<pixels; i++) { 
                fread(&data,sizeof(data),1,bmp_fp);
                int p = invert_row(bmp, i);
                image->component[BMP_B].data[p] = data[0] << 4;
                image->component[BMP_G].data[p] = data[1] << 4;
                image->component[BMP_R].data[p] = data[2] << 4;
            }
        }
    }
    /* RGB(A) */

    fclose(bmp_fp);

    dcp_log(LOG_DEBUG,"%-15.15s: BMP read complete","read_bmp");
    *image_ptr = image;

    return DCP_SUCCESS;
}
