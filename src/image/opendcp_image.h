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

typedef struct {
    int component_number;
    int *data;
} odcp_image_component_t;

typedef struct {
    int x0;
    int y0;
    int x1;
    int y1;
    int dx;
    int dy;
    int w;
    int h;
    int bpp;
    int precision;
    int signed_bit;
    odcp_image_component_t *component;
    int n_components;
} odcp_image_t;

int tif_to_image(opj_image_t **image, const char *infile, int fd);
int image_to_tif(opj_image_t *image, const char *outfile, int fd);
int read_tif(odcp_image_t **image_ptr, const char *infile, int fd);
int write_tif(odcp_image_t *image, const char *outfile, int fd);
odcp_image_t *odcp_image_create(int n_components, int image_size);
void odcp_image_free(odcp_image_t *image);
int rgb_to_xyz(odcp_image_t *image);
