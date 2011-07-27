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
#include <tiffio.h>
#include "../opendcp.h"
#include "opendcp_image.h"

static inline int clip8(int value)
{
    if      (value < 0) return 0;
    else if (value > 255) return 255;
    else               return value;
}

int read_tif(odcp_image_t **image_ptr, const char *infile, int fd) {
    TIFF *tif;
    tdata_t buf;
    tstrip_t strip;
    tsize_t strip_size;
    int index,w,h,image_size;
    unsigned short int bps = 0;
    unsigned short int spp = 0;
    unsigned short int photo = 0;
    uint16 planar;
    odcp_image_t *image = 00;

    /* open tiff using filename or file descriptor */
    dcp_log(LOG_DEBUG,"Opening tiff file %s",infile);
    if (fd == 0) {
        tif = TIFFOpen(infile, "r");
    } else {
        tif = TIFFFdOpen(fd,infile,"r");
    }

    if (!tif) {
        dcp_log(LOG_ERROR,"Failed to open %s for reading", infile);
        return DCP_FATAL;
    }

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar);

    image_size = w * h;
   
    /* create the image */
    dcp_log(LOG_DEBUG,"Allocating odcp image");
    image = odcp_image_create(3,image_size);
    dcp_log(LOG_DEBUG,"Image allocated");
    
    if(!image) {
        TIFFClose(tif);
        dcp_log(LOG_ERROR,"Failed to create image %s", infile);
        return DCP_FATAL;
    }

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

    if (photo == 6) {
        int r,c;
        uint32* raster;

        dcp_log(LOG_ERROR,"YUV/YCbCr images not currently supported");
        return DCP_FATAL ; 
 
        dcp_log(LOG_DEBUG,"Photometric: 6, YUV");
        dcp_log(LOG_DEBUG,"TIFFmalloc");
        raster = (uint32*) _TIFFmalloc(image_size * 2 * sizeof (uint32));
        if (raster == NULL) {
            dcp_log(LOG_ERROR,"could not allocate memory for raster");
            return(DCP_FATAL);
        }

        dcp_log(LOG_DEBUG,"TIFFReadRGBAImage");
        if (!TIFFReadRGBAImage(tif, w*2, h, raster, 0)) {
            dcp_log(LOG_ERROR,"could not read tiff into memory");
            return(DCP_FATAL);
        }
        TIFFClose(tif);

        index = 0;
        unsigned char *img = (unsigned char *) raster;
        for (r=h;r>0;r--) {
            for (c=0;c<w;c+=3) {
                int y,u,v;
                int c,d,e;
                int r,g,b;
                y = img[r*w+c];
                u = img[(r/2)*(w/2)+(c/2)+image_size];
                v = img[(r/2)*(w/2)+(c/2)+image_size+(image_size/4)];
                c = y-16; 
                d = u-128;
                e = v-128;
                r = clip8(((298*c)+(409*e)+128)>>8); 
                g = clip8(((298*c)-(100*d)-(208*e)+128)>>8);        
                b = clip8(((298*c)+(516*d)+128)>>8);        
                image->component[0].data[index] = r << 4; // R 
                image->component[1].data[index] = g << 4; // G 
                image->component[2].data[index] = b << 4; // B 
                index++;
            }
        }
        _TIFFfree(raster);
    }

    if (photo == 2) {
        dcp_log(LOG_DEBUG,"Photometric: 2, RGB");
        buf = _TIFFmalloc(TIFFStripSize(tif));
        strip_size=0;
        strip_size=TIFFStripSize(tif);
        index = 0;

        /* Read the Image components*/
        for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
            unsigned char *dat8;
            int i, ssize;
            ssize = TIFFReadEncodedStrip(tif, strip, buf, strip_size);
            dat8 = (unsigned char*)buf;

            /*12 bits per pixel*/
            if (bps==12) {
                for (i=0; i<ssize; i+=(3*spp)) {
                    if((index < image_size)&(index+1 < image_size)) {
                        image->component[0].data[index]   = ( dat8[i+0]<<4 )        |(dat8[i+1]>>4); // R
                        image->component[1].data[index]   = ((dat8[i+1]& 0x0f)<< 8) | dat8[i+2];     // G
                        image->component[2].data[index]   = ( dat8[i+3]<<4)         |(dat8[i+4]>>4); // B
                        if (spp == 4) {
                            /* skip alpha channel */
                            image->component[0].data[index+1] = ( dat8[i+6]<<4)        |(dat8[i+7]>>4);  // R
                            image->component[1].data[index+1] = ((dat8[i+7]& 0x0f)<< 8) | dat8[i+8];     // G
                            image->component[2].data[index+1] = ( dat8[i+9]<<4)        |(dat8[i+10]>>4); // B
                        } else {
                            image->component[0].data[index+1] = ((dat8[i+4]& 0x0f)<< 8) | dat8[i+5];     // R
                            image->component[1].data[index+1] = ( dat8[i+6]<<4)        |(dat8[i+7]>>4);  // G
                            image->component[2].data[index+1] = ((dat8[i+7]& 0x0f)<< 8) | dat8[i+8];     // B
                        }
                        index+=2;
                    } else {
                        break;
                    }
                }
            /* 16 bits per pixel */
            } else if(bps==16) {
                for (i=0; i<ssize; i+=(2*spp)) {
                    if(index < image_size) {
                        image->component[0].data[index] = ( dat8[i+1] << 8 ) | dat8[i+0]; // R 
                        image->component[1].data[index] = ( dat8[i+3] << 8 ) | dat8[i+2]; // G 
                        image->component[2].data[index] = ( dat8[i+5] << 8 ) | dat8[i+4]; // B 
                        /* round to 12 bits dcinema */
                        image->component[0].data[index] = (image->component[0].data[index]) >> 4 ;
                        image->component[1].data[index] = (image->component[1].data[index]) >> 4 ;
                        image->component[2].data[index] = (image->component[2].data[index]) >> 4 ;
                        index++;
                    } else {
                        break;
                    }
                }
            /* 8 bits per pixel */
            } else if (bps==8) {
                for (i=0; i<ssize; i+=spp) {
                    if(index < image_size) {
                        /* rounded to 12 bits */
                        image->component[0].data[index] = dat8[i+0] << 4; // R 
                        image->component[1].data[index] = dat8[i+1] << 4; // G 
                        image->component[2].data[index] = dat8[i+2] << 4; // B 
                        index++;
                    } else {
                         break;
                    }
                }
            } else {
                 dcp_log(LOG_ERROR,"TIFF file load failed. Bits=%d, Only 8,12,16 bits implemented",bps);
                 return DCP_FATAL;
            }
        }
        _TIFFfree(buf);
        TIFFClose(tif);
    } else {
        dcp_log(LOG_ERROR,"TIFF file creation. Bad color format. Only RGB & Grayscale has been implemented");
        return DCP_FATAL;
    }

    dcp_log(LOG_DEBUG,"TIFF read complete");
    *image_ptr = image;
    return 0;
}
    
int write_tif(odcp_image_t *image, const char *outfile, int fd) {
    int image_size;
    int index,adjust = 0;
    int last_i=0;
    TIFF *tif;
    tdata_t buf;
    tstrip_t strip;
    tsize_t strip_size;

    if (image->n_components == 3) {
        /* open tiff using filename or file descriptor */
        if (fd == 0) {
            tif = TIFFOpen(outfile, "wb");
        } else {
            tif = TIFFFdOpen(fd, outfile, "wb");
        }

        if (tif == NULL) {
            dcp_log(LOG_ERROR, "Failed to open %s for writing", outfile);
            return DCP_FATAL;
        }

        image_size = image->w * image->h;

        /* Set tags */
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image->w);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image->h);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, image->precision);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

        /* Get a buffer for the data */
        strip_size=TIFFStripSize(tif);
        buf = _TIFFmalloc(strip_size);
        index=0;
        adjust = image->signed_bit ? 1 << (image->precision - 1) : 0;
        for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
            unsigned char *dat8;
            int i, ssize;
            ssize = TIFFStripSize(tif);
            dat8 = (unsigned char*)buf;
            /* 12 bits per pixel */
            for (i=0; i<ssize-8; i+=9) {    // 12 bits per pixel 
                int r = 0,g = 0,b = 0;
                int r1 = 0,g1 = 0,b1 = 0;
                if ((index < image_size)&(index+1 < image_size)) {
                    r  = image->component[0].data[index];
                    g  = image->component[1].data[index];
                    b  = image->component[2].data[index];
                    r1 = image->component[0].data[index+1];
                    g1 = image->component[1].data[index+1];
                    b1 = image->component[2].data[index+1];
                    if (image->signed_bit) {                                        
                        r  += adjust;
                        g  += adjust;
                        b  += adjust;
                        r1 += adjust;
                        g1 += adjust;
                        b1 += adjust;
                    }
                    dat8[i+0] = (r >> 4);
                    dat8[i+1] = ((r & 0x0f) << 4 )|((g >> 8)& 0x0f);
                    dat8[i+2] = g ;
                    dat8[i+3] = (b >> 4);
                    dat8[i+4] = ((b & 0x0f) << 4 )|((r1 >> 8)& 0x0f);
                    dat8[i+5] = r1;
                    dat8[i+6] = (g1 >> 4);
                    dat8[i+7] = ((g1 & 0x0f)<< 4 )|((b1 >> 8)& 0x0f);
                    dat8[i+8] = b1;
                    index+=2;
                    last_i = i+9;
                } else {
                    break;
                }
            }
            if (last_i < ssize) {
                for (i=last_i; i<ssize; i+=9) {
                    int r = 0,g = 0,b = 0;
                    int r1 = 0,g1 = 0,b1 = 0;
                    if ((index < image_size)&(index+1 < image_size)) {
                        r  = image->component[0].data[index];
                        g  = image->component[1].data[index];
                        b  = image->component[2].data[index];
                        r1 = image->component[0].data[index+1];
                        g1 = image->component[1].data[index+1];
                        b1 = image->component[2].data[index+1];
                        if (image->signed_bit) {                                
                            r  += adjust;
                            g  += adjust;
                            b  += adjust;
                            r1 += adjust;
                            g1 += adjust;
                            b1 += adjust;
                        }
                        dat8[i+0] = (r >> 4);
                        if (i+1 <ssize) dat8[i+1] = ((r & 0x0f) << 4 )|((g >> 8)& 0x0f); else break;
                        if (i+2 <ssize) dat8[i+2] = g; else break;
                        if (i+3 <ssize) dat8[i+3] = (b >> 4); else break;
                        if (i+4 <ssize) dat8[i+4] = ((b & 0x0f) << 4 )|((r1 >> 8)& 0x0f);else break;
                        if (i+5 <ssize) dat8[i+5] = r1; else break;
                        if (i+6 <ssize) dat8[i+6] = (g1 >> 4); else break;
                        if (i+7 <ssize) dat8[i+7] = ((g1 & 0x0f)<< 4 )|((b1 >> 8)& 0x0f); else break;
                        if (i+8 <ssize) dat8[i+8] = b1; else break;
                        index+=2;
                    } else {
                        break;
                    }
                }
            }
            TIFFWriteEncodedStrip(tif, strip, buf, strip_size);
        }
        _TIFFfree(buf);
        TIFFClose(tif);
    } else {
        dcp_log(LOG_ERROR,"Failed TIFF file %s creation. Bad color format. Only RGB & Grayscale has been implemented",outfile);
        return DCP_FATAL;
    }
    return 0;
}
