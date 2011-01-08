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
    xmlDocDump(fp, doc);

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
