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
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include "opendcp.h"
#include "opendcp_certificates.h"

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
    if(strcmp(context->rating,"")) {
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
            fprintf(fp,"          <ScreenAspectRatio>%s</ScreenAspectRatio>\n",context->reel[x].MainPicture.aspect_ratio);
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
    if (context->xml_sign) {
        write_dsig_template(context, fp);
    }
    fprintf(fp,"</CompositionPlaylist>\n");
    fclose(fp);

    /* sign the XML file */
    if (context->xml_sign) {
        xml_sign(context, filename);
    }

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
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainPicture.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainPicture.size);
            fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
	    fprintf(fp,"    </Asset>\n");
        }
        /* Main Sound */
        if ( context->reel[x].MainSound.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSound.uuid);
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainSound.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainSound.size);
            fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
	    fprintf(fp,"    </Asset>\n");
        }
        /* Main Subtitle */
        if ( context->reel[x].MainSubtitle.essence_type ) {
            fprintf(fp,"    <Asset>\n");
            fprintf(fp,"      <Id>urn:uuid:%s</Id>\n",context->reel[x].MainSubtitle.uuid);
            if ( context->digest_flag ) {
                fprintf(fp,"      <Hash>%s</Hash>\n",context->reel[x].MainSubtitle.digest);
            }
            fprintf(fp,"      <Size>%s</Size>\n",context->reel[x].MainSubtitle.size);
            fprintf(fp,"      <Type>%s</Type>\n","application/mxf");
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
    fprintf(fp,"      <Type>%s</Type>\n","text/xml");
    fprintf(fp,"    </Asset>\n");
    fprintf(fp,"  </AssetList>\n");
    if (context->xml_sign) {
        write_dsig_template(context, fp);
    }
    fprintf(fp,"</PackingList>\n");

    fclose(fp);

    /* sign the XML file */
    if (context->xml_sign) {
        xml_sign(context, filename);
    }

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

char *dn_oneline(X509_NAME *xn) {
    BIO* bio;
    int n;
    char *result;
    unsigned long flags = XN_FLAG_RFC2253;

    if ((bio = BIO_new(BIO_s_mem())) == NULL) {
        return NULL;
    }

    X509_NAME_print_ex(bio, xn, 0, flags);
    n = BIO_pending(bio);
    result = (char *)malloc(sizeof(char *) * (n+1));
    n = BIO_read(bio, result, n);
    result[n] = '\0';
    BIO_free(bio);

    return result;
}

int write_dsig_template(context_t *context, FILE *fp) {
    BIO *bio[3];
    X509 *x[3];
    X509_NAME *issuer_xn[3];
    X509_NAME *subject_xn[3];
    char *cert[3];
    int i;

    if (context->xml_sign_certs) {
        /* read certificates from file */
        FILE *cp; 

        cp = fopen(context->signer_cert_file,"rb");
        if (cp) {
            x[0] = PEM_read_X509(cp,NULL,NULL,NULL);
            fclose(cp);
        }
        cp = fopen(context->ca_cert_file,"rb");
        if (cp) {
            x[1] = PEM_read_X509(cp,NULL,NULL,NULL);
            fclose(cp);
        }
        cp = fopen(context->root_cert_file,"rb");
        if (cp) {
            x[2] = PEM_read_X509(cp,NULL,NULL,NULL);
            fclose(cp);
        }
        cert[0] = strip_cert_file(context->signer_cert_file);
        cert[1] = strip_cert_file(context->ca_cert_file);
        cert[2] = strip_cert_file(context->root_cert_file);
    } else {
        /* read certificate from memory */
        bio[0] = BIO_new_mem_buf((void *)opendcp_signer_cert, -1);
        bio[1] = BIO_new_mem_buf((void *)opendcp_ca_cert, -1);
        bio[2] = BIO_new_mem_buf((void *)opendcp_root_cert, -1);

        /* save a copy with the BEGIN/END stripped */
        cert[0] = strip_cert(opendcp_signer_cert);
        cert[1] = strip_cert(opendcp_ca_cert);
        cert[2] = strip_cert(opendcp_root_cert);

        for (i=0;i<3;i++) {
            if (bio[i] == NULL) {
                dcp_log(LOG_ERROR,"Could allocate certificate from memory");
                return DCP_FATAL;
            }
            x[i] = PEM_read_bio_X509(bio[i], NULL, NULL, NULL);
            BIO_set_close(bio[i], BIO_NOCLOSE);

            if (x[i] == NULL) {
                dcp_log(LOG_ERROR,"Could not read certificate");
                return DCP_FATAL;
             }
        }
    }

    /* get issuer, subject */
    for (i=0;i<3;i++) {
        issuer_xn[i]  =  X509_get_issuer_name(x[i]); 
        subject_xn[i] =  X509_get_subject_name(x[i]); 
        if (issuer_xn[i] == NULL || subject_xn[i] == NULL) {
            dcp_log(LOG_ERROR,"Could not parse certificate data");
            return DCP_FATAL;
        }
    }

    /* signer */
    fprintf(fp,"  <Signer>\n"); 
    fprintf(fp,"    <dsig:X509Data>\n");
    fprintf(fp,"      <dsig:X509IssuerSerial>\n");
    fprintf(fp,"        <dsig:X509IssuerName>%s</dsig:X509IssuerName>\n",dn_oneline(issuer_xn[0]));
    fprintf(fp,"        <dsig:X509SerialNumber>%ld</dsig:X509SerialNumber>\n",ASN1_INTEGER_get(X509_get_serialNumber(x[0])));
    fprintf(fp,"      </dsig:X509IssuerSerial>\n");
    fprintf(fp,"      <dsig:X509SubjectName>%s</dsig:X509SubjectName>\n",dn_oneline(subject_xn[0]));
    fprintf(fp,"    </dsig:X509Data>\n");
    fprintf(fp,"  </Signer>\n"); 

    /* template */
    fprintf(fp,"  <dsig:Signature>\n");
    fprintf(fp,"    <dsig:SignedInfo>\n");
    fprintf(fp,"      <dsig:CanonicalizationMethod Algorithm=\"%s\"/>\n",DS_CMA);
    fprintf(fp,"      <dsig:SignatureMethod Algorithm=\"%s\"/>\n",DS_SMA[context->reel[0].MainPicture.xml_ns]);
    fprintf(fp,"      <dsig:Reference URI=\"\">\n");
    fprintf(fp,"        <dsig:Transforms>\n");
    fprintf(fp,"          <dsig:Transform Algorithm=\"%s\"/>\n",DS_TMA);
    fprintf(fp,"        </dsig:Transforms>\n");
    fprintf(fp,"        <dsig:DigestMethod Algorithm=\"%s\"/>\n",DS_DMA);
    fprintf(fp,"        <dsig:DigestValue/>\n");
    fprintf(fp,"      </dsig:Reference>\n");
    fprintf(fp,"    </dsig:SignedInfo>\n");
    fprintf(fp,"    <dsig:SignatureValue/>\n");
    fprintf(fp,"    <dsig:KeyInfo>\n");
    for (i=0;i<3;i++) {
        fprintf(fp,"      <dsig:X509Data>\n");
        fprintf(fp,"        <dsig:X509IssuerSerial>\n");
        fprintf(fp,"          <dsig:X509IssuerName>%s</dsig:X509IssuerName>\n",dn_oneline(issuer_xn[i]));
        fprintf(fp,"          <dsig:X509SerialNumber>%ld</dsig:X509SerialNumber>\n",ASN1_INTEGER_get(X509_get_serialNumber(x[i])));
        fprintf(fp,"        </dsig:X509IssuerSerial>\n");
        fprintf(fp,"        <dsig:X509Certificate>%s</dsig:X509Certificate>\n",cert[i]);
        fprintf(fp,"      </dsig:X509Data>\n");
    }
    fprintf(fp,"    </dsig:KeyInfo>\n");
    fprintf(fp,"  </dsig:Signature>\n");

    return DCP_SUCCESS;
}
