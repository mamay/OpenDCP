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
#include <time.h>
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
    
    sprintf(ratio,"%-3.2f",a);

    return(ratio);
}

int write_cpl(context_t *context) {
    FILE *fp;
    int x;
    struct stat st;
    char uuid_s[40];
    char filename[128];

    sprintf(filename,"%.128s",context->cpl.filename);

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %.128s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing CPL file %.128s",filename);

    /* CPL XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<CompositionPlaylist xmlns=\"%s\" xmlns:dsig=\"%s\">\n",NS_CPL[context->reel[0].MainPicture.xml_ns],DS_DSIG);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",context->cpl.uuid);
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",context->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",context->issuer);
    fprintf(fp,"  <Creator>%s</Creator>\n",context->creator);
    fprintf(fp,"  <ContentTitleText>%s</ContentTitleText>\n",context->title);
    fprintf(fp,"  <ContentKind>%s</ContentKind>\n",context->kind);

    if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
        fprintf(fp,"  <ContentVersion>\n");
        fprintf(fp,"    <Id>urn:uri:%s_%s</Id>\n",context->cpl.uuid,context->timestamp);
        fprintf(fp,"    <LabelText>%s_%s</LabelText>\n",context->cpl.uuid,context->timestamp);
        fprintf(fp,"  </ContentVersion>\n");
    }

    if (strcmp(context->rating,"")) {
        fprintf(fp,"  <RatingList>\n");
        fprintf(fp,"    <Agency>%s</Agency>\n",RATING_AGENCY[1]);
        fprintf(fp,"    <Label>%s</Label>\n",context->rating);
        fprintf(fp,"  </RatingList>\n");
    } else {
        fprintf(fp,"  <RatingList/>\n");
    }

    /* Reel(s) Start */
    fprintf(fp,"  <ReelList>\n");
    for (x=0;x<context->reel_count;x++) {
        fprintf(fp,"    <Reel>\n");
        uuid_random(uuid_s);
        fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",uuid_s);
        fprintf(fp,"      <AssetList>\n");
        /* Main Picture */
        if ( context->reel[x].MainPicture.essence_type ) {
            if ( context->reel[x].MainPicture.stereoscopic ) {
                fprintf(fp,"        <msp-cpl:MainStereoscopicPicture xmlns:msp-cpl=\"%s\">\n",NS_CPL_3D[context->reel[0].MainPicture.xml_ns]);
            } else {
                fprintf(fp,"        <MainPicture>\n");
            }
            fprintf(fp,"          <Id>urn:uuid:%s</Id>\n",context->reel[x].MainPicture.uuid);
            fprintf(fp,"          <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainPicture.annotation);
            fprintf(fp,"          <EditRate>%s</EditRate>\n",context->reel[x].MainPicture.edit_rate);
            fprintf(fp,"          <IntrinsicDuration>%d</IntrinsicDuration>\n",context->reel[x].MainPicture.duration);
            fprintf(fp,"          <EntryPoint>%d</EntryPoint>\n",context->reel[x].MainPicture.entry_point);
            fprintf(fp,"          <Duration>%d</Duration>\n",context->reel[x].MainPicture.duration);
            if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
                fprintf(fp,"          <ScreenAspectRatio>%s</ScreenAspectRatio>\n",context->reel[x].MainPicture.aspect_ratio);
            } else {
                fprintf(fp,"          <ScreenAspectRatio>%s</ScreenAspectRatio>\n",get_aspect_ratio(context->reel[x].MainPicture.aspect_ratio));
            }
            fprintf(fp,"          <FrameRate>%s</FrameRate>\n",context->reel[x].MainPicture.frame_rate);
            if ( context->digest_flag ) {
                fprintf(fp,"          <Hash>%s</Hash>\n",context->reel[x].MainPicture.digest);
            }
            if ( context->reel[x].MainPicture.stereoscopic ) {
                fprintf(fp,"        </msp-cpl:MainStereoscopicPicture>\n");
            } else {
                fprintf(fp,"        </MainPicture>\n");
            }
         }
        /* Main Sound */
        if ( context->reel[x].MainSound.essence_type ) {
            fprintf(fp,"        <MainSound>\n");
            fprintf(fp,"          <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSound.uuid);
            fprintf(fp,"          <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainSound.annotation);
            fprintf(fp,"          <EditRate>%s</EditRate>\n",context->reel[x].MainSound.edit_rate);
            fprintf(fp,"          <IntrinsicDuration>%d</IntrinsicDuration>\n",context->reel[x].MainSound.duration);
            fprintf(fp,"          <EntryPoint>%d</EntryPoint>\n",context->reel[x].MainSound.entry_point);
            fprintf(fp,"          <Duration>%d</Duration>\n",context->reel[x].MainSound.duration);
            if ( context->digest_flag ) {
                fprintf(fp,"          <Hash>%s</Hash>\n",context->reel[x].MainSound.digest);
            }
            fprintf(fp,"        </MainSound>\n");
        }
        /* Subtitle */
        if ( context->reel[x].MainSubtitle.essence_type ) {
            fprintf(fp,"        <MainSubtitle>\n");
            fprintf(fp,"          <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSubtitle.uuid);
            fprintf(fp,"          <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainSubtitle.annotation);
            fprintf(fp,"          <EditRate>%s</EditRate>\n",context->reel[x].MainSubtitle.edit_rate);
            fprintf(fp,"          <IntrinsicDuration>%d</IntrinsicDuration>\n",context->reel[x].MainSubtitle.duration);
            fprintf(fp,"          <EntryPoint>%d</EntryPoint>\n",context->reel[x].MainSubtitle.entry_point);
            fprintf(fp,"          <Duration>%d</Duration>\n",context->reel[x].MainSubtitle.duration);
            if ( context->digest_flag ) {
                fprintf(fp,"          <Hash>%s</Hash>\n",context->reel[x].MainSubtitle.digest);
            }
            fprintf(fp,"        </MainSsubtitle>\n");
        }

        fprintf(fp,"      </AssetList>\n");
        fprintf(fp,"    </Reel>\n");
    }
    fprintf(fp,"  </ReelList>\n");

#ifdef XMLSEC
    if (context->xml_sign) {
        write_dsig_template(context, fp);
    }
#endif

    fprintf(fp,"</CompositionPlaylist>\n");
    fclose(fp);

#ifdef XMLSEC
    /* sign the XML file */
    if (context->xml_sign) {
        xml_sign(context, filename);
    }
#endif

    /* Store CPL file size */
    stat(filename, &st);
    sprintf(context->cpl.size,"%lld",st.st_size);
    calculate_digest(filename,context->cpl.digest);

    return DCP_SUCCESS;
}

int write_pkl(context_t *context) {
    FILE *fp;
    int x;
    struct stat st;
    char filename[128];

    sprintf(filename,"%.128s",context->pkl.filename);
    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %.128s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing PKL file %.128s",filename);

    /* PKL XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<PackingList xmlns=\"%s\" xmlns:dsig=\"%s\">\n",NS_PKL[context->reel[0].MainPicture.xml_ns],DS_DSIG);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",context->pkl.uuid);
    fprintf(fp,"  <AnnotationText>%s</AnnotationText>\n",context->annotation);
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",context->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",context->issuer);
    fprintf(fp,"  <Creator>%s</Creator>\n",context->creator);
    fprintf(fp,"  <AssetList>\n");

    /* Asset(s) Start */
    for (x=0;x<context->reel_count;x++) {
        /* Main Picture */
        if ( context->reel[x].MainPicture.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainPicture.uuid);
            fprintf(fp,"      <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainPicture.annotation);
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainPicture.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainPicture.size);
            if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
                fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
            } else {
                fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Picture");
            }
            fprintf(fp,"    </Asset>\n");
        }
        /* Main Sound */
        if ( context->reel[x].MainSound.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSound.uuid);
            fprintf(fp,"      <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainSound.annotation);
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainSound.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainSound.size);
            if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
                fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
            } else {
                fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Sound");
            }
            fprintf(fp,"    </Asset>\n");
        }
        /* Main Subtitle */
        if ( context->reel[x].MainSubtitle.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSubtitle.uuid);
            fprintf(fp,"      <AnnotationText>%s</AnnotationText>\n",context->reel[x].MainSubtitle.annotation);
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainSubtitle.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainSubtitle.size);
            if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
                fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
            } else {
                fprintf(fp,"      <Type>%s</Type>\n","application/x-smpte-mxf;asdcpKind=Subtitle");
            }
            fprintf(fp,"    </Asset>\n");
        }
    }

    /* CPL */
    fprintf(fp,"    <Asset>\n");
    fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->cpl.uuid);
    if ( context->digest_flag ) {
        fprintf(fp,"      <Hash>%s</Hash>\n",context->cpl.digest);
    }
    fprintf(fp,"      <Size>%s</Size>\n",context->cpl.size);
    if (context->reel[0].MainPicture.xml_ns == XML_NS_SMPTE) {
        fprintf(fp,"      <Type>%s</Type>\n","text/xml");
    } else {
        fprintf(fp,"      <Type>%s</Type>\n","text/xml;asdcpKind=CPL");
    }
    fprintf(fp,"    </Asset>\n");
    fprintf(fp,"  </AssetList>\n");

#ifdef XMLSEC
    if (context->xml_sign) {
        write_dsig_template(context, fp);
    }
#endif

    fprintf(fp,"</PackingList>\n");

    fclose(fp);

#ifdef XMLSEC
    /* sign the XML file */
    if (context->xml_sign) {
        xml_sign(context, filename);
    }
#endif

    /* Store PKL file size */
    stat(filename, &st);
    sprintf(context->pkl.size,"%lld",st.st_size);

    return DCP_SUCCESS;
}

int write_assetmap(context_t *context) {
    FILE *fp;
    int x;
    char filename[MAX_FILENAME_LENGTH];
    char uuid_s[40];

    sprintf(filename,"ASSETMAP");

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
        dcp_log(LOG_ERROR,"Could not open file %.128s for writing",filename);
        return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing ASSETMAP file %.128s",filename);

    /* Generate Assetmap UUID */
    uuid_random(uuid_s);

    /* Assetmap XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<AssetMap xmlns=\"%s\">\n",NS_AM[context->reel[0].MainPicture.xml_ns]);
    fprintf(fp,"  <Id>urn:uuid:%s</Id>\n",uuid_s);
    fprintf(fp,"  <Creator>%s</Creator>\n",context->creator);
    fprintf(fp,"  <VolumeCount>1</VolumeCount>\n");
    fprintf(fp,"  <IssueDate>%s</IssueDate>\n",context->timestamp);
    fprintf(fp,"  <Issuer>%s</Issuer>\n",context->issuer);
    fprintf(fp,"  <AssetList>\n");
 
    /* PKL */
    fprintf(fp,"    <Asset>\n");
    fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->pkl.uuid);
    fprintf(fp,"      <PackingList>true</PackingList>\n");
    fprintf(fp,"      <ChunkList>\n");
    fprintf(fp,"        <Chunk>\n");
    fprintf(fp,"          <Path>%s</Path>\n",context->pkl.filename);
    fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
    fprintf(fp,"          <Offset>0</Offset>\n");
    fprintf(fp,"          <Length>%s</Length>\n",context->pkl.size);
    fprintf(fp,"        </Chunk>\n");
    fprintf(fp,"      </ChunkList>\n");
    fprintf(fp,"    </Asset>\n");

    /* CPL */
    fprintf(fp,"    <Asset>\n");
    fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->cpl.uuid);
    fprintf(fp,"      <ChunkList>\n");
    fprintf(fp,"        <Chunk>\n");
    fprintf(fp,"          <Path>%s</Path>\n",context->cpl.filename);
    fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
    fprintf(fp,"          <Offset>0</Offset>\n");
    fprintf(fp,"          <Length>%s</Length>\n",context->cpl.size);
    fprintf(fp,"        </Chunk>\n");
    fprintf(fp,"      </ChunkList>\n");
    fprintf(fp,"    </Asset>\n");

    /* Assets(s) Start */
    for (x=0;x<context->reel_count;x++) {
        /* Main Picture */
        if ( context->reel[x].MainPicture.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainPicture.uuid);
            fprintf(fp,"      <ChunkList>\n");
            fprintf(fp,"        <Chunk>\n");
            fprintf(fp,"          <Path>%s</Path>\n",context->reel[x].MainPicture.filename);
            fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
            fprintf(fp,"          <Offset>0</Offset>\n");
            fprintf(fp,"          <Length>%s</Length>\n",context->reel[x].MainPicture.size);
            fprintf(fp,"        </Chunk>\n");
            fprintf(fp,"      </ChunkList>\n");
            fprintf(fp,"    </Asset>\n");
        }
        /* Main Sound */
        if ( context->reel[x].MainSound.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSound.uuid);
            fprintf(fp,"      <ChunkList>\n");
            fprintf(fp,"        <Chunk>\n");
            fprintf(fp,"          <Path>%s</Path>\n",context->reel[x].MainSound.filename);
            fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
            fprintf(fp,"          <Offset>0</Offset>\n");
            fprintf(fp,"          <Length>%s</Length>\n",context->reel[x].MainSound.size);
            fprintf(fp,"        </Chunk>\n");
            fprintf(fp,"      </ChunkList>\n");
            fprintf(fp,"    </Asset>\n");
        }
        /* Main Subtitle */
        if ( context->reel[x].MainSubtitle.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSubtitle.uuid);
            fprintf(fp,"      <ChunkList>\n");
            fprintf(fp,"        <Chunk>\n");
            fprintf(fp,"          <Path>%s</Path>\n",context->reel[x].MainSubtitle.filename);
            fprintf(fp,"          <VolumeIndex>1</VolumeIndex>\n");
            fprintf(fp,"          <Offset>0</Offset>\n");
            fprintf(fp,"          <Length>%s</Length>\n",context->reel[x].MainSubtitle.size);
            fprintf(fp,"        </Chunk>\n");
            fprintf(fp,"      </ChunkList>\n");
            fprintf(fp,"    </Asset>\n");
        }
    }

    fprintf(fp,"  </AssetList>\n");
    fprintf(fp,"</AssetMap>\n");
   
    fclose(fp);

    return DCP_SUCCESS;
}

int write_volumeindex(context_t *context) {
    FILE *fp;
    char filename[MAX_FILENAME_LENGTH];

    sprintf(filename,"VOLINDEX");

    fp = fopen(filename, "w");

    if ( fp == NULL ) {
       dcp_log(LOG_ERROR,"Could not open file %.128s for writing",filename);
       return DCP_FATAL;
    }

    dcp_log(LOG_INFO,"Writing VOLINDEX file %.128s",filename);

    /* Volumeindex XML Start */
    fprintf(fp,"%s\n",XML_HEADER);
    fprintf(fp,"<VolumeIndex xmlns=\"%s\">\n",NS_AM[context->reel[0].MainPicture.xml_ns]);
    fprintf(fp,"  <Index>1</Index>\n");
    fprintf(fp,"</VolumeIndex>\n");

    fclose(fp);

    return DCP_SUCCESS;
}
