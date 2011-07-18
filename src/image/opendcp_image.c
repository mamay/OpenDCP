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
#include "../opendcp.h"
#include "opendcp_image.h"
#include "opendcp_xyz.h"

/* create opendcp image structure */
odcp_image_t *odcp_image_create(int n_components, int image_size) {
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
            image->component[x].data = (int *)malloc((image_size)*sizeof(int));
            if (!image->component[x].data) {
                dcp_log(LOG_ERROR,"Unable to allocate memory for image components");
                odcp_image_free(image);
                return NULL;
            }
        }
    }

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

/* convert opendcp to openjpeg image format */
int odcp_to_opj(odcp_image_t *odcp, opj_image_t **opj_ptr) {
    OPJ_COLOR_SPACE color_space;
    opj_image_cmptparm_t cmptparm[3];
    opj_image_t *opj = NULL;
    int j,size;

    color_space = CLRSPC_SRGB;

    /* initialize image components */
    memset(&cmptparm[0], 0, odcp->n_components * sizeof(opj_image_cmptparm_t));
    for (j = 0;j <  odcp->n_components;j++) {
            cmptparm[j].w = odcp->w;
            cmptparm[j].h = odcp->h;
            cmptparm[j].prec = odcp->precision;
            cmptparm[j].bpp = odcp->bpp;
            cmptparm[j].sgnd = odcp->signed_bit;
            cmptparm[j].dx = odcp->dx;
            cmptparm[j].dy = odcp->dy;
    }

    /* create the image */
    opj = opj_image_create(odcp->n_components, &cmptparm[0], color_space);

    if(!opj) {
        dcp_log(LOG_ERROR,"Failed to create image");
        return DCP_FATAL;
    }

    /* set image offset and reference grid */
    opj->x0 = odcp->x0;
    opj->y0 = odcp->y0;
    opj->x1 = odcp->x1; 
    opj->y1 = odcp->y1; 

    size = odcp->w * odcp->h;

    memcpy(opj->comps[0].data,odcp->component[0].data,size*sizeof(int));
    memcpy(opj->comps[1].data,odcp->component[1].data,size*sizeof(int));
    memcpy(opj->comps[2].data,odcp->component[2].data,size*sizeof(int));

    /*
    for (j=0;j<size;j++) {
       opj->comps[0].data[j] = odcp->component[0].data[j];
       opj->comps[1].data[j] = odcp->component[1].data[j];
       opj->comps[2].data[j] = odcp->component[2].data[j];
    }
    */

    *opj_ptr = opj;
    return DCP_SUCCESS;
}

/* rgb to xyz color conversion 12-bit LUT */
int rgb_to_xyz(odcp_image_t *image, int index) {
    int i;
    int size;
    rgb_pixel_t s;
    xyz_pixel_t d;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        /* In gamma LUT */
        s.r = lut_in[index][image->component[0].data[i]];
        s.g = lut_in[index][image->component[1].data[i]];
        s.b = lut_in[index][image->component[2].data[i]];

        /* RGB to XYZ Matrix */
        d.x = ((s.r * color_matrix[index][0][0]) + (s.g * color_matrix[index][0][1]) + (s.b * color_matrix[index][0][2]));
        d.y = ((s.r * color_matrix[index][1][0]) + (s.g * color_matrix[index][1][1]) + (s.b * color_matrix[index][1][2]));
        d.z = ((s.r * color_matrix[index][2][0]) + (s.g * color_matrix[index][2][1]) + (s.b * color_matrix[index][2][2]));

        /* DCI Companding */
        d.x = ((d.x > 1) ? 1.0 : d.x) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);
        d.y = ((d.y > 1) ? 1.0 : d.y) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);
        d.z = ((d.z > 1) ? 1.0 : d.z) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);

        /* Out gamma LUT */
        image->component[0].data[i] = lut_out[LO_DCI][(int)d.x];
        image->component[1].data[i] = lut_out[LO_DCI][(int)d.y];
        image->component[2].data[i] = lut_out[LO_DCI][(int)d.z];
    }

    return DCP_SUCCESS;
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

/* rgb to xyz color conversion hard calculations */
int rgb_to_xyz_calculate(odcp_image_t *image, int index) {
    int i;
    int size;
    rgb_pixel_t s;
    xyz_pixel_t d;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        s.r = complex_gamma(image->component[0].data[i]/COLOR_DEPTH, GAMMA[index]);
        s.g = complex_gamma(image->component[1].data[i]/COLOR_DEPTH, GAMMA[index]);
        s.b = complex_gamma(image->component[2].data[i]/COLOR_DEPTH, GAMMA[index]);

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

    if (x< 2.0) {
        return(((2.0f-x) * (2.0f-x) * (2.0f-x)) * c);
    }
}

rgb_pixel_t get_pixel(odcp_image_t *image, int x, int y) {
    rgb_pixel_t p;
    int i;

    i = (image->w * y) + (x*3);

    p.r = image->component[0].data[i];
    p.b = image->component[1].data[i];
    p.g = image->component[2].data[i];
    
    return p;
} 

int resize(odcp_image_t **image,int w,int h,int method) {
    int image_size;
    int num_components = 3;
    odcp_image_t *ptr = *image;
    rgb_pixel_t p;

    image_size = w * h;

    printf("resize image\n");
    /* create the image */
    odcp_image_t *d_image = odcp_image_create(num_components,image_size);

    if (!d_image) {
        return -1;
    }
    
    if (method == NEAREST_PIXEL) {
        int x,y,i;
        float tx, ty, dx, dy;

        printf("original: %d x %d\n",ptr->w,ptr->h);
        tx = (float)ptr->w / w;
        ty = (float)ptr->h / h;
        printf("ratio: %f | %f\n",tx,ty);


        for(y=0; y<h; y++) {
            dy = y * ty; 
            for(x=0; x<w; x++) {
                dx = x * tx;
                //printf("getting pixel %d,%d\n",dx,dy);
                p = get_pixel(ptr, dx, dy);
                i = (w * y) + (x*3);
                d_image->component[0].data[i] = p.r;
                d_image->component[1].data[i] = p.g;
                d_image->component[2].data[i] = p.b;
             }
         }
    }

    printf("free image\n");
    odcp_image_free(*image);
    *image = d_image; 

    printf("resize complete\n");

    return 0;
}
