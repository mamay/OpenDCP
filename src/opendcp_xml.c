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
#include <time.h>
#include <inttypes.h>
#include <sys/stat.h>
#include "opendcp.h"

#ifdef WIN32
char *strsep (char **stringp, const char *delim) {
    register char *s;
    register const char *spanp;
    register int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}
#endif

char *get_aspect_ratio(char *dimension_string) {
    char *p, *ratio;
    int n, d;
    float a = 0.0;

    ratio = malloc(sizeof(char)*5);    
    p = malloc(strlen(dimension_string)+1);
    strcpy(p,dimension_string);
    n = atoi(strsep(&p," "));
    d = atoi(strsep(&p," "));
    
    if (d>0) {
        a = (n * 1.00) / (d * 1.00);
    }

    if ( a >= 1.77 && a <= 1.78) {
        a = 1.77;
    }

    sprintf(ratio,"%-3.2f",a);

    return(ratio);
}

int write_cpl(opendcp_t *opendcp, cpl_t *cpl) {
    FILE *fp;
    int a,r;
    struct stat st;
    char uuid_s[40];
    char filename[MAX_PATH_LENGTH];

    sprintf(filename,"%s",cpl->filename);

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing CPL file %.256s",filename);

    /* CPL XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<CompositionPlaylist xmlns=\"%s\" xmlns:dsig=\"%s\">\n",NS_CPL[opendcp->ns],DS_DSIG);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",cpl->uuid);
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",cpl->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",cpl->issuer);
    fprintf(fp,"  <Creator>%s</Creator>\n",cpl->creator);
    fprintf(fp,"  <ContentTitleText>%s</ContentTitleText>\n",cpl->title);
    fprintf(fp,"  <ContentKind>%s</ContentKind>\n",cpl->kind);

    if (opendcp->ns == XML_NS_SMPTE) {
        fprintf(fp,"  <ContentVersion>\n");
        fprintf(fp,"    <Id>urn:uri:%s_%s</Id>\n",cpl->uuid,cpl->timestamp);
        fprintf(fp,"    <LabelText>%s_%s</LabelText>\n",cpl->uuid,cpl->timestamp);
        fprintf(fp,"  </ContentVersion>\n");
    }

    if (strcmp(cpl->rating,"")) {
        fprintf(fp,"  <RatingList>\n");
        fprintf(fp,"    <Agency>%s</Agency>\n",RATING_AGENCY[1]);
        fprintf(fp,"    <Label>%s</Label>\n",cpl->rating);
        fprintf(fp,"  </RatingList>\n");
    } else {
        fprintf(fp,"  <RatingList/>\n");
    }

    /* Reel(s) Start */
    fprintf(fp,"  <ReelList>\n");
    for (r=0;r<cpl->reel_count;r++) {
        reel_t reel = cpl->reel[r];
        fprintf(fp,"    <Reel>\n");
        fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",cpl->reel[r].uuid);
        fprintf(fp,"      <AssetList>\n");

        /* Asset(s) Start */
        for (a=0;a<cpl->reel[r].asset_count;a++) {
            asset_t asset = cpl->reel[r].asset[a];

            if (asset.essence_class == ACT_PICTURE) {
                if (asset.stereoscopic) {
                    fprintf(fp,"        <msp-cpl:MainStereoscopicPicture xmlns:msp-cpl=\"%s\">\n",NS_CPL_3D[opendcp->ns]);
                } else {
                    fprintf(fp,"        <MainPicture>\n");
                }
            } 
            if (asset.essence_class == ACT_SOUND) {
                fprintf(fp,"        <MainSound>\n");
            }
            if (asset.essence_class == ACT_TIMED_TEXT) {
                fprintf(fp,"        <MainSubtitle\n");
            }
            fprintf(fp,"          <Id>urn:uuid:%s</Id>\n",asset.uuid);
            fprintf(fp,"          <AnnotationText>%s</AnnotationText>\n",asset.annotation);
            fprintf(fp,"          <EditRate>%s</EditRate>\n",asset.edit_rate);
            fprintf(fp,"          <IntrinsicDuration>%d</IntrinsicDuration>\n",asset.intrinsic_duration);
            fprintf(fp,"          <EntryPoint>%d</EntryPoint>\n",asset.entry_point);
            fprintf(fp,"          <Duration>%d</Duration>\n",asset.duration);
            if (asset.essence_class == ACT_PICTURE) {
                if (opendcp->ns == XML_NS_SMPTE) {
                    fprintf(fp,"          <ScreenAspectRatio>%s</ScreenAspectRatio>\n",asset.aspect_ratio);
                } else {
                    fprintf(fp,"          <ScreenAspectRatio>%s</ScreenAspectRatio>\n",get_aspect_ratio(asset.aspect_ratio));
                }
                fprintf(fp,"          <FrameRate>%s</FrameRate>\n",asset.frame_rate);
            }
            if ( opendcp->digest_flag ) {
                fprintf(fp,"          <Hash>%s</Hash>\n",asset.digest);
            }
            if (asset.essence_class == ACT_PICTURE) {
                if (asset.stereoscopic) {
                    fprintf(fp,"        </msp-cpl:MainStereoscopicPicture>\n");
                } else {
                    fprintf(fp,"        </MainPicture>\n");
                }
            }
            if (asset.essence_class == ACT_SOUND) {
                fprintf(fp,"        </MainSound>\n");
            }
            if (asset.essence_class == ACT_TIMED_TEXT) {
                fprintf(fp,"        </MainSubtitle\n");
            }
         }
        fprintf(fp,"      </AssetList>\n");
        fprintf(fp,"    </Reel>\n");
    }
    fprintf(fp,"  </ReelList>\n");

#ifdef XMLSEC
    if (opendcp->xml_sign) {
        write_dsig_template(opendcp, fp);
    }
#endif

    fprintf(fp,"</CompositionPlaylist>\n");
    fclose(fp);

#ifdef XMLSEC
    /* sign the XML file */
    if (opendcp->xml_sign) {
        xml_sign(opendcp, filename);
    }
#endif

    /* Store CPL file size */
    dcp_log(LOG_INFO,"Writing CPL file info");
    stat(filename, &st);
    sprintf(cpl->size,"%"PRIu64,st.st_size);
    calculate_digest(filename,cpl->digest);

    return DCP_SUCCESS;
}

int write_cpl_list(opendcp_t *opendcp) {
   int placeholder;
}

int write_pkl_list(opendcp_t *opendcp) {
   int placeholder;
}

int write_pkl(opendcp_t *opendcp, pkl_t *pkl) {
    FILE *fp;
    int a,c,r;
    struct stat st;
    char filename[MAX_PATH_LENGTH];

    sprintf(filename,"%s",pkl->filename);

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %.256s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing PKL file %.256s",filename);

    /* PKL XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<PackingList xmlns=\"%s\" xmlns:dsig=\"%s\">\n",NS_PKL[opendcp->ns],DS_DSIG);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",pkl->uuid);
    fprintf(fp,"  <AnnotationText>%s</AnnotationText>\n",pkl->annotation);
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",opendcp->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",opendcp->issuer);
    fprintf(fp,"  <Creator>%s</Creator>\n",opendcp->creator);
    fprintf(fp,"  <AssetList>\n");


    dcp_log(LOG_INFO,"CPLS: %d",pkl->cpl_count);

    /* Asset(s) Start */
    for (c=0;c<pkl->cpl_count;c++) {
        cpl_t cpl = pkl->cpl[c];
        dcp_log(LOG_INFO,"REELS: %d",cpl.reel_count);
        for (r=0;r<cpl.reel_count;r++) {
            reel_t reel = cpl.reel[r];

            for (a=0;a<reel.asset_count;a++) {
                asset_t asset = reel.asset[a];
                fprintf(fp,"    <Asset>\n");
                fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",asset.uuid);
                fprintf(fp,"      <AnnotationText>%s</AnnotationText>\n",asset.annotation);
                fprintf(fp,"      <Hash>%s</Hash>\n",asset.digest);
                fprintf(fp,"      <Size>%s</Size>\n",asset.size);
                if (opendcp->ns == XML_NS_SMPTE) {
                    fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
                } else {
                    if (asset.essence_class == ACT_PICTURE) {
                        fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Picture");
                    }
                    if (asset.essence_class == ACT_SOUND) {
                        fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Sound");
                    }
                    if (asset.essence_class == ACT_TIMED_TEXT) {
                        fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Subtitle");
                    }
                }
                fprintf(fp,"    </Asset>\n");
            }
        }

        /* CPL */
        fprintf(fp,"    <Asset>\n");
        fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",cpl.uuid);
        fprintf(fp,"      <Hash>%s</Hash>\n",cpl.digest);
        fprintf(fp,"      <Size>%s</Size>\n",cpl.size);
        if (opendcp->ns == XML_NS_SMPTE) {
            fprintf(fp,"      <Type>%s</Type>\n","text/xml");
        } else {
            fprintf(fp,"      <Type>%s</Type>\n","text/xml;asdcpKind=CPL");
        }
        fprintf(fp,"    </Asset>\n");
    }
    fprintf(fp,"  </AssetList>\n");

#ifdef XMLSEC
    if (opendcp->xml_sign) {
        write_dsig_template(opendcp, fp);
    }
#endif

    fprintf(fp,"</PackingList>\n");

    fclose(fp);

#ifdef XMLSEC
    /* sign the XML file */
    if (opendcp->xml_sign) {
        xml_sign(opendcp, filename);
    }
#endif

    /* Store PKL file size */
    stat(filename, &st);
    sprintf(pkl->size,"%"PRIu64,st.st_size);

    return DCP_SUCCESS;
}

int write_assetmap(opendcp_t *opendcp) {
    FILE *fp;
    int a,c,r;
    char filename[MAX_PATH_LENGTH];
    char uuid_s[40];

    if (opendcp->ns == XML_NS_INTEROP) {
        sprintf(filename,"%s","ASSETMAP");
    } else {
        sprintf(filename,"%s","ASSETMAP.xml");
    }

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %.256s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing ASSETMAP file %.256s",filename);

    /* Generate Assetmap UUID */
    uuid_random(uuid_s);

    /* Assetmap XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<AssetMap xmlns=\"%s\">\n",NS_AM[opendcp->ns]);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",uuid_s);
    fprintf(fp,"  <Creator>%s</Creator>\n",opendcp->creator);
    fprintf(fp,"  <VolumeCount>1</VolumeCount>\n");
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",opendcp->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",opendcp->issuer);
    fprintf(fp,"  <AssetList>\n");
 
    /* PKL */
    fprintf(fp,"    <Asset>\n");
    fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",opendcp->pkl[0].uuid);
    fprintf(fp,"      <PackingList>true</PackingList>\n");
    fprintf(fp,"      <ChunkList>\n");
    fprintf(fp,"        <Chunk>\n");
    fprintf(fp,"          <Path>%s</Path>\n",basename(opendcp->pkl[0].filename));
    fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
    fprintf(fp,"          <Offset>0</Offset>\n");
    fprintf(fp,"          <Length>%s</Length>\n",opendcp->pkl[0].size);
    fprintf(fp,"        </Chunk>\n");
    fprintf(fp,"      </ChunkList>\n");
    fprintf(fp,"    </Asset>\n");

    /* CPL */
    for (c=0;c<opendcp->pkl[0].cpl_count;c++) {
        cpl_t cpl = opendcp->pkl[0].cpl[c];
        fprintf(fp,"    <Asset>\n");
        fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",cpl.uuid);
        fprintf(fp,"      <ChunkList>\n");
        fprintf(fp,"        <Chunk>\n");
        fprintf(fp,"          <Path>%s</Path>\n",basename(cpl.filename));
        fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
        fprintf(fp,"          <Offset>0</Offset>\n");
        fprintf(fp,"          <Length>%s</Length>\n",cpl.size);
        fprintf(fp,"        </Chunk>\n");
        fprintf(fp,"      </ChunkList>\n");
        fprintf(fp,"    </Asset>\n");

        /* Assets(s) Start */
        for (r=0;r<cpl.reel_count;r++) {
            reel_t reel = cpl.reel[r];
            for (a=0;a<reel.asset_count;a++) {
                asset_t asset = reel.asset[a];
                fprintf(fp,"    <Asset>\n");
                fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",asset.uuid);
                fprintf(fp,"      <ChunkList>\n");
                fprintf(fp,"        <Chunk>\n");
                fprintf(fp,"          <Path>%s</Path>\n",basename(asset.filename));
                fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
                fprintf(fp,"          <Offset>0</Offset>\n");
                fprintf(fp,"          <Length>%s</Length>\n",asset.size);
                fprintf(fp,"        </Chunk>\n");
                fprintf(fp,"      </ChunkList>\n");
                fprintf(fp,"    </Asset>\n");
            }
        }
    }

    fprintf(fp,"  </AssetList>\n");
    fprintf(fp,"</AssetMap>\n");
   
    fclose(fp);

    return DCP_SUCCESS;
}

int write_volumeindex(opendcp_t *opendcp) {
    FILE *fp;
    char filename[MAX_PATH_LENGTH];

    if (opendcp->ns == XML_NS_INTEROP) {
        sprintf(filename,"%s","VOLINDEX");
    } else {
        sprintf(filename,"%s","VOLINDEX.xml");
    }

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
       dcp_log(LOG_ERROR,"Could not open file %.256s for writing",filename);
       return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing VOLINDEX file %.256s",filename);

    /* Volumeindex XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<VolumeIndex xmlns=\"%s\">\n",NS_AM[opendcp->ns]);
    fprintf(fp,"  <Index>1</Index>\n");
    fprintf(fp,"</VolumeIndex>\n");

    fclose(fp);

    return DCP_SUCCESS;
}
