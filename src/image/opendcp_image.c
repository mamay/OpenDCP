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
    rgb_pixel_t p_in;
    xyz_pixel_t p_out;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        /* In Gamma LUT */
        p_in.r = lut_in[index][image->component[0].data[i]];
        p_in.g = lut_in[index][image->component[1].data[i]];
        p_in.b = lut_in[index][image->component[2].data[i]];

        /* RGB to XYZ Matrix */
        p_out.x = ((p_in.r * color_matrix[index][0][0]) + (p_in.g * color_matrix[index][0][1]) + (p_in.b * color_matrix[index][0][2]));
        p_out.y = ((p_in.r * color_matrix[index][1][0]) + (p_in.g * color_matrix[index][1][1]) + (p_in.b * color_matrix[index][1][2]));
        p_out.z = ((p_in.r * color_matrix[index][2][0]) + (p_in.g * color_matrix[index][2][1]) + (p_in.b * color_matrix[index][2][2]));

        /* DCI Companding */
        p_out.x = ((p_out.x > 1) ? 1.0 : p_out.x) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);
        p_out.y = ((p_out.y > 1) ? 1.0 : p_out.y) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);
        p_out.z = ((p_out.z > 1) ? 1.0 : p_out.z) * (DCI_COEFFICENT) * (DCI_LUT_SIZE - 1);

        /* Out Gamma LUT */
        image->component[0].data[i] = lut_out[LO_DCI][(int)p_out.x];
        image->component[1].data[i] = lut_out[LO_DCI][(int)p_out.y];
        image->component[2].data[i] = lut_out[LO_DCI][(int)p_out.z];
    }

    return DCP_SUCCESS;
}

/* rgb to xyz color conversion hard calculations */
int rgb_to_xyz_calculate(odcp_image_t *image, int index) {
    int i;
    int size;
    float bpc;
    float in_gamma = 2.4;
    rgb_pixel_t p_in;

    size = image->w * image->h;

    for (i=0;i<size;i++) {
        p_in.r = image->component[0].data[i]/COLOR_DEPTH;
        p_in.g = image->component[1].data[i]/COLOR_DEPTH;
        p_in.b = image->component[2].data[i]/COLOR_DEPTH;

        if ( p_in.r > 0.04045) {
            p_in.r = pow((p_in.r+0.055)/1.055,in_gamma);
        } else {
            p_in.r = p_in.r/12.92;
        }

        if ( p_in.g > 0.04045) {
            p_in.g = pow((p_in.g+0.055)/1.055,in_gamma);
        } else {
            p_in.g = p_in.g/12.92;
        }

        if ( p_in.b > 0.04045) {
            p_in.b = pow((p_in.b+0.055)/1.055,in_gamma);
        } else {
            p_in.b = p_in.b/12.92;
        }
        image->component[0].data[i] = (pow(((p_in.r*0.4124)+(p_in.g*0.3576)+(p_in.b*0.1805))*DCI_COEFFICENT,DCI_DEGAMMA) * COLOR_DEPTH);
        image->component[1].data[i] = (pow(((p_in.r*0.2126)+(p_in.g*0.7152)+(p_in.b*0.0722))*DCI_COEFFICENT,DCI_DEGAMMA) * COLOR_DEPTH);
        image->component[2].data[i] = (pow(((p_in.r*0.0193)+(p_in.g*0.1192)+(p_in.b*0.9505))*DCI_COEFFICENT,DCI_DEGAMMA) * COLOR_DEPTH);
    }

    return DCP_SUCCESS;
}
