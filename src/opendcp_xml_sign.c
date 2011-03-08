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

#define XMLSEC_CRYPTO_OPENSSL

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifndef XMLSEC_NO_XSLT
#include <libxslt/xslt.h>
#include <libxslt/extensions.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#endif

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/templates.h>
#include <xmlsec/crypto.h>

#include "opendcp.h"
#include "opendcp_certificates.h"

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

int xmlsec_init() {
    /* init libxml lib */
    xmlInitParser();
    xmlIndentTreeOutput = 1; 
    LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);

    /* init xmlsec lib */
    if(xmlSecInit() < 0) {
        dcp_log(DCP_FATAL,"Error: xmlsec initialization failed.");
        return(DCP_FATAL);
    }

    /* Check loaded library version */
    if(xmlSecCheckVersion() != 1) {
        dcp_log(DCP_FATAL, "Error: loaded xmlsec library version is not compatible.");
        return(DCP_FATAL);
    }

    /* Init crypto library */
    if(xmlSecCryptoAppInit(NULL) < 0) {
        dcp_log(DCP_FATAL, "Error: crypto initialization failed.");
        return(DCP_FATAL);
    }

    /* Init xmlsec-crypto library */
    if(xmlSecCryptoInit() < 0) {
        dcp_log(DCP_FATAL, "Error: xmlsec-crypto initialization failed.");
        return(DCP_FATAL);
    }

    return(DCP_SUCCESS);
}

xmlSecKeysMngrPtr load_certificates(context_t *context) {
    xmlSecKeysMngrPtr key_manager;
    xmlSecKeyPtr      key; 
        
    /* create and initialize keys manager */
    key_manager = xmlSecKeysMngrCreate();

    if (key_manager == NULL) {
        fprintf(stderr, "Error: failed to create keys manager.\n");
        return(NULL);
    }

    if (xmlSecCryptoAppDefaultKeysMngrInit(key_manager) < 0) {
        fprintf(stderr, "Error: failed to initialize keys manager.\n");
        xmlSecKeysMngrDestroy(key_manager);
        return(NULL);
    }    

    /* read key file */
    if (context->private_key_file) {
        key = xmlSecCryptoAppKeyLoad(context->private_key_file, xmlSecKeyDataFormatPem, NULL, NULL, NULL);
    } else {
        key = xmlSecCryptoAppKeyLoadMemory(opendcp_private_key, strlen(opendcp_private_key),xmlSecKeyDataFormatPem, NULL, NULL, NULL);
    }
  
    if (xmlSecCryptoAppDefaultKeysMngrAdoptKey(key_manager, key) < 0) {
        fprintf(stderr, "Error: failed to initialize keys manager.\n");
        xmlSecKeysMngrDestroy(key_manager);
        return(NULL);
    }
 
    /* load root certificate */
    if (context->root_cert_file) {
        if (xmlSecCryptoAppKeysMngrCertLoad(key_manager, context->root_cert_file, xmlSecKeyDataFormatPem, xmlSecKeyDataTypeTrusted) < 0) {
            fprintf(stderr,"Error: failed to load pem certificate \"%s\"\n", context->root_cert_file);
            xmlSecKeysMngrDestroy(key_manager);
            return(NULL);
        }
    } else {
        if (xmlSecCryptoAppKeysMngrCertLoadMemory(key_manager, opendcp_root_cert, strlen(opendcp_root_cert), xmlSecKeyDataFormatPem, xmlSecKeyDataTypeTrusted) < 0) {
            fprintf(stderr,"Error: failed to load pem certificate from memory\n");
            xmlSecKeysMngrDestroy(key_manager);
            return(NULL);
        }
    }
    
    /* load ca (intermediate) certificate */
    if (context->ca_cert_file) {
        if (xmlSecCryptoAppKeysMngrCertLoad(key_manager, context->ca_cert_file, xmlSecKeyDataFormatPem, xmlSecKeyDataTypeTrusted) < 0) {
            fprintf(stderr,"Error: failed to load pem certificate \"%s\"\n", context->ca_cert_file);
            xmlSecKeysMngrDestroy(key_manager);
            return(NULL);
        }
    } else {
        if (xmlSecCryptoAppKeysMngrCertLoadMemory(key_manager, opendcp_ca_cert, strlen(opendcp_ca_cert), xmlSecKeyDataFormatPem, xmlSecKeyDataTypeTrusted) < 0) {
            fprintf(stderr,"Error: failed to load pem certificate from memory\n");
            xmlSecKeysMngrDestroy(key_manager);
            return(NULL);
        }
    }

    return(key_manager);
}

int xmlsec_close() {
    /* Shutdown xmlsec-crypto library */
    xmlSecCryptoShutdown();
    
    /* Shutdown crypto library */
    xmlSecCryptoAppShutdown();
    
    /* Shutdown xmlsec library */
    xmlSecShutdown();

    /* Shutdown libxslt/libxml */
    xmlCleanupParser();
    
    return(DCP_SUCCESS);
}

int xml_sign(context_t *context, char *filename) {
    xmlSecDSigCtxPtr dsig_ctx = NULL;
    xmlDocPtr        doc = NULL;
    xmlNodePtr       root_node;
    xmlNodePtr       sign_node;
    FILE *fp;
    int result = DCP_FATAL;
    xmlSecKeysMngrPtr key_manager;
    
    xmlsec_init();

    /* load doc file */
    doc = xmlParseFile(filename);

    if (doc == NULL) {
        dcp_log(DCP_FATAL, "Error: unable to parse file %s", filename);
        goto done;
    }

    /* find root node */
    root_node = xmlDocGetRootElement(doc);

    if (root_node == NULL){
        dcp_log(DCP_FATAL, "Error: unable to find root node");
        goto done;
    }

    /* find signature node */
   sign_node = xmlSecFindNode(root_node, xmlSecNodeSignature, xmlSecDSigNs);
    if(sign_node == NULL) {
        fprintf(stderr, "Error: start node not found");
        goto done;      
    }
  
    /* create keys manager */
    key_manager = load_certificates(context);
    if (key_manager == NULL) {
        fprintf(stderr,"Error: failed to create key manager\n");
        goto done;
    }

    /* create signature context */
    dsig_ctx = xmlSecDSigCtxCreate(key_manager);

    if(dsig_ctx == NULL) {
        fprintf(stderr,"Error: failed to create signature context\n");
        goto done;
    }

    /* sign the template */
    if(xmlSecDSigCtxSign(dsig_ctx, sign_node) < 0) {
        fprintf(stderr,"Error: signature failed\n");
        goto done;
    }

    /* open xml file */
    fp = fopen(filename,"wb"); 

    if (fp == NULL) {
        fprintf(stderr,"Error: could not open output file\n");
        goto done;
    }

    /* write the xml file */
    if (xmlDocDump(fp, doc) < 0) {
        fprintf(stderr,"Error: writing XML document failed\n");
        goto done;
    }
  
    /* close the file */
    fclose(fp);

    /* success */
    result = 0;

done:    
    /* destroy keys manager */
    xmlSecKeysMngrDestroy(key_manager);

    /* destroy signature context */
    if(dsig_ctx != NULL) {
        xmlSecDSigCtxDestroy(dsig_ctx);
    }
    
    /* destroy xml doc */
    if(doc != NULL) {
        xmlFreeDoc(doc); 
    }

    xmlsec_close();

    return(result);
}
