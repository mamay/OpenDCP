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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openjpeg.h>
#include <tiffio.h>
#include "opendcp.h"
#include "opendcp_image.h"
#include "opendcp_xyz.h"

#define CLIP(m,max)                                 \
  (m)<0?0:((m)>max?max:(m))

extern int rgb_to_xyz_calculate(odcp_image_t *image, int index);
extern int rgb_to_xyz_lut(odcp_image_t *image, int index);

/* create opendcp image structure */
odcp_image_t *odcp_image_create(int n_components, int w, int h) {
    int x;
    odcp_image_t *image = 00; 

    image = (odcp_image_t*) malloc(sizeof(odcp_image_t));

    if (image) {
        memset(image,0,sizeof(odcp_image_t));
        image->component = (odcp_image_component_t*) malloc(n_components*sizeof(odcp_image_component_t));

        if (!image->component) {
            dcp_log(LOG_ERROR,"Unable to allocate memory for image components");
            odcp_image_free(image);
            return 00;
        }

        memset(image->component,0,n_components * sizeof(odcp_image_component_t));
       
        for (x=0;x<n_components;x++) {
            image->component[x].component_number = x;
            image->component[x].data = (int *)malloc((w*h)*sizeof(int));
            if (!image->component[x].data) {
                dcp_log(LOG_ERROR,"Unable to allocate memory for image components");
                odcp_image_free(image);
                return NULL;
            }
        }
    }

    /* set default image parameters 12-bit RGB */
    image->bpp          = 12;
    image->precision    = 12;
    image->n_components = 3;
    image->signed_bit   = 0;
    image->dx           = 1;
    image->dy           = 1;
    image->w            = w;
    image->h            = h;
    image->x0           = 0;
    image->y0           = 0;
    image->x1 = !image->x0 ? (w - 1) * image->dx + 1 : image->x0 + (w - 1) * image->dx + 1;
    image->y1 = !image->y0 ? (h - 1) * image->dy + 1 : image->y0 + (h - 1) * image->dy + 1;

    return image;
}

/* free opendcp image structure */
void odcp_image_free(odcp_image_t *odcp_image) {
    int i;
    if (odcp_image) {
        if(odcp_image->component) {
            for(i = 0; i < odcp_image->n_components; i++) {
                odcp_image_component_t *component = &odcp_image->component[i];
                if(component->data) {
                    free(component->data);
                }
            }
            free(odcp_image->component);
        }
        free(odcp_image);
    }
}

int read_image(odcp_image_t **image, char *file) {
    char *extension;
    int  result;

    extension = strrchr(file,'.');
    extension++;

    if (strncasecmp(extension,"tif",3) == 0) {
        result = read_tif(image, file, 0);
    } else if (strncasecmp(extension,"dpx",3) == 0) {
        result = read_dpx(image, 0, file, 0);
    } else if (strncasecmp(extension,"bmp",3) == 0) {
        result = read_bmp(image, file, 0);
    } else {
        result = DCP_ERROR;
    }

    if (result != DCP_SUCCESS) {
        return DCP_FATAL;
    }

    return DCP_SUCCESS;
}

int odcp_image_readline(odcp_image_t *image, int y, unsigned char *data) {
    int x,i;
    int d = 0;

    for (x = 0; x<image->w; x+=2) {
        i = (x+y+0) + ((image->w-1)*y);
        data[d+0] = (image->component[0].data[i] >> 4);
        data[d+1] = ((image->component[0].data[i] & 0x0f) << 4 )|((image->component[1].data[i] >> 8)& 0x0f);
        data[d+2] = image->component[1].data[i];
        data[d+3] = (image->component[2].data[i] >> 4);
        data[d+4] = ((image->component[2].data[i] & 0x0f) << 4 )|((image->component[0].data[i+1] >> 8)& 0x0f);
        data[d+5] = image->component[0].data[i+1];
        data[d+6] = (image->component[1].data[i+1] >> 4);
        data[d+7] = ((image->component[1].data[i+1] & 0x0f)<< 4 )|((image->component[2].data[i+1] >> 8)& 0x0f);
        data[d+8] = image->component[2].data[i+1];
        d+=9;
    }

    return DCP_SUCCESS;
}

int check_image_compliance(int profile, odcp_image_t *image, char *file) {
    int          w,h;
    odcp_image_t *odcp_image;

    if (image == NULL) {
        if (read_image(&odcp_image, file) == DCP_SUCCESS) {
            h = odcp_image->h;
            w = odcp_image->w;
            odcp_image_free(odcp_image);
        } else {
            odcp_image_free(odcp_image);
            return DCP_FATAL;
        }
    } else {
        h = image->h;
        w = image->w;
    }

    switch (profile) {
        case DCP_CINEMA2K:
            if (!((w == 2048) | (h == 1080))) {
                return DCP_ERROR;
            }
        break;
        case DCP_CINEMA4K:
            if (!((w == 4096) | (h == 2160))) {
                return DCP_ERROR;
            }
            break;
        default:
            break;
    }

    return DCP_SUCCESS;
}

rgb_pixel_float_t yuv444toRGB888(int y, int cb, int cr) {
    rgb_pixel_float_t p;

    p.r = CLIP(y+1.402*(cr-128),255);
    p.g = CLIP(y-0.344*(cb-128)-0.714*(cr-128),255);
    p.b = CLIP(y+1.772*(cb-128),255);

    return(p);
}

/* complex gamma function */
float complex_gamma(float p, float gamma) {
    float v;

    if ( p > 0.04045) {
        v = pow((p+0.055)/1.055,gamma);
    } else {
        v = p/12.92;
    }

    return v;
}

int rgb_to_xyz(odcp_image_t *image, int index, int method) {
    int result;

    if (method) {
        dcp_log(LOG_DEBUG,"rgb_to_xyz_calculate");
        result = rgb_to_xyz_calculate(image, index);
    } else {
        result = rgb_to_xyz_lut(image, index);
    }

    return result;
}

/* rgb to xyz color conversion 12-bit LUT */
int rgb_to_xyz_lut(odcp_image_t *image, int index) {
    int i;
    int size;
    rgb_pixel_float_t s;
    xyz_pixel_float_t d;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        /* in gamma lut */
        s.r = lut_in[index][image->component[0].data[i]];
        s.g = lut_in[index][image->component[1].data[i]];
        s.b = lut_in[index][image->component[2].data[i]];

        /* RGB to XYZ Matrix */
        d.x = ((s.r * color_matrix[index][0][0]) + (s.g * color_matrix[index][0][1]) + (s.b * color_matrix[index][0][2]));
        d.y = ((s.r * color_matrix[index][1][0]) + (s.g * color_matrix[index][1][1]) + (s.b * color_matrix[index][1][2]));
        d.z = ((s.r * color_matrix[index][2][0]) + (s.g * color_matrix[index][2][1]) + (s.b * color_matrix[index][2][2]));

        /* DCI Companding */
        d.x = d.x * DCI_COEFFICENT * (DCI_LUT_SIZE-1);
        d.y = d.y * DCI_COEFFICENT * (DCI_LUT_SIZE-1);
        d.z = d.z * DCI_COEFFICENT * (DCI_LUT_SIZE-1);

        /* out gamma lut */
        image->component[0].data[i] = lut_out[LO_DCI][(int)d.x];
        image->component[1].data[i] = lut_out[LO_DCI][(int)d.y];
        image->component[2].data[i] = lut_out[LO_DCI][(int)d.z];
    }

    return DCP_SUCCESS;
}

/* rgb to xyz color conversion hard calculations */
int rgb_to_xyz_calculate(odcp_image_t *image, int index) {
    int i;
    int size;
    rgb_pixel_float_t s;
    xyz_pixel_float_t d;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        s.r = complex_gamma(image->component[0].data[i]/(float)COLOR_DEPTH, GAMMA[index]);
        s.g = complex_gamma(image->component[1].data[i]/(float)COLOR_DEPTH, GAMMA[index]);
        s.b = complex_gamma(image->component[2].data[i]/(float)COLOR_DEPTH, GAMMA[index]);

        s.r = lut_in[index][image->component[0].data[i]];
        s.g = lut_in[index][image->component[1].data[i]];
        s.b = lut_in[index][image->component[2].data[i]];

        d.x = ((s.r * color_matrix[index][0][0]) + (s.g * color_matrix[index][0][1]) + (s.b * color_matrix[index][0][2]));
        d.y = ((s.r * color_matrix[index][1][0]) + (s.g * color_matrix[index][1][1]) + (s.b * color_matrix[index][1][2]));
        d.z = ((s.r * color_matrix[index][2][0]) + (s.g * color_matrix[index][2][1]) + (s.b * color_matrix[index][2][2]));

        image->component[0].data[i] = (pow((d.x*DCI_COEFFICENT),DCI_DEGAMMA) * COLOR_DEPTH);
        image->component[1].data[i] = (pow((d.y*DCI_COEFFICENT),DCI_DEGAMMA) * COLOR_DEPTH);
        image->component[2].data[i] = (pow((d.z*DCI_COEFFICENT),DCI_DEGAMMA) * COLOR_DEPTH);
    }

    return DCP_SUCCESS;
}

float b_spline(float x) {
    float c = (float)1/(float)6;

    if (x > 2.0 || x < -2.0) {
        return 0.0f;
    }

    if (x < -1.0) {
        return(((x+2.0f) * (x+2.0f) * (x+2.0f)) * c);
    }

    if (x < 0.0) {
        return(((x+4.0f) * (x) * (-6.0f-3.0f*x)) * c);
    }

    if (x < 1.0) {
        return(((x+4.0f) * (x) * (-6.0f+3.0f*x)) * c);
    }

    if (x < 2.0) {
        return(((2.0f-x) * (2.0f-x) * (2.0f-x)) * c);
    }

    return 0;
}

/* get the pixel index based on x,y */
static inline rgb_pixel_float_t get_pixel(odcp_image_t *image, int x, int y) {
    rgb_pixel_float_t p;
    int i;

    i = x + (image->w*y);
    //i = (x+y) + ((image->w-1)*y);

    p.r = image->component[0].data[i];
    p.g = image->component[1].data[i];
    p.b = image->component[2].data[i];
    
    return p;
} 

int letterbox(odcp_image_t **image, int w, int h) {
    int num_components = 3;
    int x,y;
    odcp_image_t *ptr = *image;
    rgb_pixel_float_t p;

    /* create the image */
    odcp_image_t *d_image = odcp_image_create(num_components, w, h);

    if (!d_image) {
        return -1;
    }

    for(y=0; y<h; y++) {
        for(x=0; x<w; x++) {
            p = get_pixel(ptr, x, y);
            d_image->component[0].data[0] = (int)p.r;
            d_image->component[1].data[0] = (int)p.g;
            d_image->component[2].data[0] = (int)p.b;
         }
    }


    odcp_image_free(*image);
    *image = d_image;

    return DCP_SUCCESS;
} 

/* resize image */
int resize(odcp_image_t **image, int profile, int method) {
    int num_components = 3;
    odcp_image_t *ptr = *image;
    rgb_pixel_float_t p;
    int w,h;
    float aspect;
  
    /* aspect ratio */
    aspect = (float)ptr->w/(float)ptr->h;

    /* depending on the aspect ratio, set height or weight priority */
    if (aspect <= 2.10) {
        w = (ptr->w*MAX_HEIGHT_2K/ptr->h);
        h = MAX_HEIGHT_2K;
    } else {
        w = MAX_WIDTH_2K;
        h = (ptr->h*MAX_WIDTH_2K/ptr->w);
    }

    /* if we overshot width, scale back */
    if (w > MAX_WIDTH_2K) {
        w = MAX_WIDTH_2K;
        h = w / aspect;
    }

    /* if we overshot height, scale back */
    if (h > MAX_HEIGHT_2K) {
        h = MAX_HEIGHT_2K;
        w = h * aspect;
    }

    /* adjust for 4K */
    if (profile == DCP_CINEMA4K) {
        w *= 2;
        h *= 2;
    }

    dcp_log(LOG_INFO,"Resizing from %dx%d to %dx%d (%f)",ptr->w, ptr->h,w,h,aspect);

    /* create the image */
    odcp_image_t *d_image = odcp_image_create(num_components, w, h);

    if (!d_image) {
        return -1;
    }

    /* simple resize - pixel double */
    if (method == NEAREST_PIXEL) {
        int x,y,i,dx,dy;
        float tx, ty;

        tx = (float)ptr->w / w;
        ty = (float)ptr->h / h;

        for(y=0; y<h; y++) {
            dy = y * ty; 
            for(x=0; x<w; x++) {
                dx = x * tx;
                p = get_pixel(ptr, dx, dy);
                i = x + (w*y);
                d_image->component[0].data[i] = (int)p.r;
                d_image->component[1].data[i] = (int)p.g;
                d_image->component[2].data[i] = (int)p.b;
             }
         }
    }

    odcp_image_free(*image);
    *image = d_image; 

    return DCP_SUCCESS;
}
