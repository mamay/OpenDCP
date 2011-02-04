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

#include <AS_DCP.h>
#include <KM_fileio.h>
#include <KM_prng.h>
#include <KM_memio.h>
#include <KM_util.h>
#include <PCMParserList.h>
#include <openssl/sha.h>

#include <iostream>
#include <assert.h>

#include "asdcp_intf.h"
#include "opendcp.h"

using namespace ASDCP;

const ui32_t FRAME_BUFFER_SIZE = 4 * Kumu::Megabyte;

/* generate a random UUID */
extern "C" void uuid_random(char *uuid) {
    char buffer[64];
    Kumu::UUID TmpID;
    Kumu::GenRandomValue(TmpID);
    sprintf(uuid,"%.36s", TmpID.EncodeHex(buffer, 64));
}

/* calcuate the SHA1 digest of a file */
extern "C" int calculate_digest(const char *filename, char *digest)
{
    using namespace Kumu;

    FileReader    reader;
    SHA_CTX       sha_context;
    ByteString    read_buffer(16384);
    ui32_t        read_length;
    const ui32_t  sha_length = 20;
    byte_t        byte_buffer[sha_length];
    char          sha_buffer[64];
    Result_t      result = RESULT_OK;

    result = reader.OpenRead(filename);
    SHA1_Init(&sha_context);

    while (ASDCP_SUCCESS(result)) {
        read_length = 0;
        result = reader.Read(read_buffer.Data(), read_buffer.Capacity(), &read_length);

        if (ASDCP_SUCCESS(result)) {
            SHA1_Update(&sha_context, read_buffer.Data(), read_length);
        }
    }

    if (result == RESULT_ENDOFFILE) {
        result = RESULT_OK;
    }

    if (ASDCP_SUCCESS(result)) {
        SHA1_Final(byte_buffer, &sha_context);
        sprintf(digest,"%.36s",base64encode(byte_buffer, sha_length, sha_buffer, 64));
    }

    if (ASDCP_SUCCESS(result)) {
        return DCP_SUCCESS;
    } else {
        return DCP_FATAL;
    }
}

/* get the essence type of an asset */
extern "C" int get_file_essence_type(char *filename) {
    EssenceType_t essence_type;
    Result_t result = RESULT_OK;

    result = ASDCP::RawEssenceType(filename, essence_type);

    if (ASDCP_FAILURE(result)) {
        return DCP_FATAL;
    }

    switch(essence_type) {
        case ESS_MPEG2_VES:
            return AET_MPEG2_VES;
            break;
        case ESS_JPEG_2000:
            return AET_JPEG_2000;
            break;
        case ESS_PCM_24b_48k:
        case ESS_PCM_24b_96k:
            return AET_JPEG_2000;
            break;
        case ESS_TIMED_TEXT:
            return AET_TIMED_TEXT;
            break;
        default:
            return AET_UNKNOWN;
            break;
    }
}

/* read asset file information */
extern "C" int read_asset_info(asset_t *asset)
{
    EssenceType_t essence_type;
    WriterInfo info;
    Result_t result = RESULT_OK;
    Kumu::UUID uuid;
    char buffer[80];
    char uuid_buffer[64];
    
    result = ASDCP::EssenceType(asset->filename, essence_type);

    if (ASDCP_FAILURE(result)) {
        return DCP_FATAL;
    }

    switch (essence_type) {
        case ESS_MPEG2_VES:
        {
            MPEG2::MXFReader reader;
            MPEG2::VideoDescriptor desc;
            result = reader.OpenRead(asset->filename);

            if (ASDCP_FAILURE(result)) {
                return DCP_FATAL;
            }

	    reader.FillVideoDescriptor(desc);
            reader.FillWriterInfo(info);

            asset->essence_type   = essence_type;
            asset->duration       = desc.ContainerDuration;
            asset->entry_point    = 0;
            asset->xml_ns         = info.LabelSetType;
            sprintf(asset->uuid,"%.36s", Kumu::bin2UUIDhex(info.AssetUUID,16,uuid_buffer, 64));
            sprintf(asset->aspect_ratio,"%d %d",desc.AspectRatio.Numerator,desc.AspectRatio.Denominator);
            sprintf(asset->edit_rate,"%d %d",desc.EditRate.Numerator,desc.EditRate.Denominator);
            sprintf(asset->sample_rate,"%d %d",desc.SampleRate.Numerator,desc.SampleRate.Denominator);
            sprintf(asset->frame_rate,"%d",desc.FrameRate);
        }
        break;
        case ESS_JPEG_2000_S:
        {        
            JP2K::MXFSReader reader;
            JP2K::PictureDescriptor desc;
            result = reader.OpenRead(asset->filename);

            if (ASDCP_FAILURE(result)) {
                return DCP_FATAL;
            }

            reader.FillPictureDescriptor(desc);
            reader.FillWriterInfo(info);

            asset->stereoscopic   = 1;
            asset->essence_type   = essence_type;
            asset->duration       = desc.ContainerDuration;
            asset->entry_point    = 0;
            asset->xml_ns         = info.LabelSetType;
            sprintf(asset->uuid,"%.36s", Kumu::bin2UUIDhex(info.AssetUUID,16,uuid_buffer, 64));
            sprintf(asset->aspect_ratio,"%d %d",desc.AspectRatio.Numerator,desc.AspectRatio.Denominator);
            sprintf(asset->edit_rate,"%d %d",desc.EditRate.Numerator,desc.EditRate.Denominator);
            sprintf(asset->sample_rate,"%d %d",desc.SampleRate.Numerator,desc.SampleRate.Denominator);
            sprintf(asset->frame_rate,"%d %d",desc.SampleRate.Numerator,desc.SampleRate.Denominator);
        }
        break;
        case ESS_JPEG_2000:
        {
            JP2K::MXFReader reader;
            JP2K::PictureDescriptor desc;
            result = reader.OpenRead(asset->filename);

            /* Try 3D MXF Interop */
            if (ASDCP_FAILURE(result)) {
                JP2K::MXFSReader reader;
                result = reader.OpenRead(asset->filename);
                asset->stereoscopic   = 1;
                if ( ASDCP_FAILURE(result) ) {
                    return DCP_FATAL;
                }
                reader.FillPictureDescriptor(desc);
                reader.FillWriterInfo(info);
            } else {
                reader.FillPictureDescriptor(desc);
                reader.FillWriterInfo(info);
            }

            asset->essence_type   = essence_type;
            asset->duration       = desc.ContainerDuration;
            asset->entry_point    = 0;
            asset->xml_ns         = info.LabelSetType;
            sprintf(asset->uuid,"%.36s", Kumu::bin2UUIDhex(info.AssetUUID,16,uuid_buffer, 64));
            sprintf(asset->aspect_ratio,"%d %d",desc.AspectRatio.Numerator,desc.AspectRatio.Denominator);
            sprintf(asset->edit_rate,"%d %d",desc.EditRate.Numerator,desc.EditRate.Denominator);
            sprintf(asset->sample_rate,"%d %d",desc.SampleRate.Numerator,desc.SampleRate.Denominator);
            sprintf(asset->frame_rate,"%d %d",desc.SampleRate.Numerator,desc.SampleRate.Denominator);
        }
        break;
        case ESS_PCM_24b_48k:
        case ESS_PCM_24b_96k:
        {
            PCM::MXFReader reader;
            PCM::AudioDescriptor desc;
            result = reader.OpenRead(asset->filename);

            if (ASDCP_FAILURE(result)) {
                return DCP_FATAL;
            }

            reader.FillAudioDescriptor(desc);
            reader.FillWriterInfo(info);

            asset->essence_type   = essence_type;
            asset->duration       = desc.ContainerDuration;
            asset->entry_point    = 0;
            asset->xml_ns         = info.LabelSetType;
            sprintf(asset->uuid,"%.36s", Kumu::bin2UUIDhex(info.AssetUUID,16,uuid_buffer, 64));
            sprintf(asset->edit_rate,"%d %d",desc.EditRate.Numerator,desc.EditRate.Denominator);
            sprintf(asset->sample_rate,"%d %d",desc.AudioSamplingRate.Numerator,desc.AudioSamplingRate.Denominator);
        }
        break;
        case ESS_TIMED_TEXT:
        { 
            TimedText::MXFReader reader;
            TimedText::TimedTextDescriptor desc;
            result = reader.OpenRead(asset->filename);

            if (ASDCP_FAILURE(result)) {
                return DCP_FATAL;
            }
  
            reader.FillTimedTextDescriptor(desc);
            reader.FillWriterInfo(info);
  
            asset->essence_type   = essence_type;
            asset->duration       = desc.ContainerDuration;
            asset->entry_point    = 0;
            asset->xml_ns         = info.LabelSetType;
            sprintf(asset->uuid,"%.36s", Kumu::bin2UUIDhex(info.AssetUUID,16,uuid_buffer, 64));
            sprintf(asset->edit_rate,"%d %d",desc.EditRate.Numerator,desc.EditRate.Denominator);
      }
      break;
      default:
            return DCP_FATAL;
    }
 
    return DCP_SUCCESS;
}

/* write the asset to an mxf file */
extern "C" int write_mxf(context_t *context, filelist_t *filelist, char *output_file) {
    Result_t      result = RESULT_OK; 
    EssenceType_t essence_type;

    result = ASDCP::RawEssenceType(filelist->in[0], essence_type);

    if (ASDCP_FAILURE(result)) {
        printf("Could determine essence type of  %s\n",filelist->in[0]);
        return DCP_FATAL;
    }

    switch (essence_type) {
        case ESS_JPEG_2000:
            if ( context->stereoscopic )
                result = write_j2k_s_mxf(context,filelist,output_file);
            else
                result = write_j2k_mxf(context,filelist,output_file);
            break;
        case ESS_PCM_24b_48k:
        case ESS_PCM_24b_96k:
            result = write_pcm_mxf(context,filelist,output_file);
            break;
        case ESS_MPEG2_VES:
            result = write_mpeg2_mxf(context,filelist,output_file);
            break;
    }

    if (ASDCP_FAILURE(result)) {
        return DCP_FATAL;
    }

    return DCP_SUCCESS; 
}

/* write out j2k mxf file */
Result_t write_j2k_mxf(context_t *context, filelist_t *filelist, char *output_file) {
    AESEncContext*          aes_context = 0;
    HMACContext*            hmac_context = 0;
    JP2K::MXFWriter         mxf_writer;
    JP2K::PictureDescriptor picture_desc;
    JP2K::CodestreamParser  j2k_parser;
    JP2K::FrameBuffer       frame_buffer(FRAME_BUFFER_SIZE);
    Kumu::FortunaRNG        rng;
    WriterInfo              writer_info;
    byte_t                  iv_buf[CBC_BLOCK_SIZE];
    Result_t                result = RESULT_OK; 
    ui32_t                  mxf_duration;

    result = j2k_parser.OpenReadFrame(filelist->in[0], frame_buffer);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    Rational edit_rate(context->frame_rate,1);
    j2k_parser.FillPictureDescriptor(picture_desc);
    picture_desc.EditRate = edit_rate; 

    writer_info.ProductVersion = OPEN_DCP_VERSION; 
    writer_info.CompanyName = OPEN_DCP_NAME; 
    writer_info.ProductName = OPEN_DCP_NAME; 

    /* set the label type */
    if (context->ns == XML_NS_INTEROP) {
        writer_info.LabelSetType = LS_MXF_INTEROP;
    } else if ( context->ns == XML_NS_SMPTE ) {
        writer_info.LabelSetType = LS_MXF_SMPTE;
    } else {
        writer_info.LabelSetType = LS_MXF_UNKNOWN;
    }

    /* generate a random UUID for this essence */
    Kumu::GenRandomUUID(writer_info.AssetUUID);

    /* start encryption, if set */
    if (context->key_flag) {
        Kumu::GenRandomUUID(writer_info.ContextID);
        writer_info.EncryptedEssence = true;

        if (context->key_id) {
            memcpy(writer_info.CryptographicKeyID, context->key_id, UUIDlen);
        } else {
            rng.FillRandom(writer_info.CryptographicKeyID, UUIDlen);
        }

        aes_context = new AESEncContext;
        result = aes_context->InitKey(context->key_value);

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        result = aes_context->SetIVec(rng.FillRandom(iv_buf, CBC_BLOCK_SIZE));

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        if (context->write_hmac) {
            writer_info.UsesHMAC = true;
            hmac_context = new HMACContext;
            result = hmac_context->InitKey(context->key_value, writer_info.LabelSetType);

            if (ASDCP_FAILURE(result)) {
                return result;
            }
        }
    }

    result = mxf_writer.OpenWrite(output_file, writer_info, picture_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }
   
    /* set the duration of the output mxf */
    if (filelist->file_count < context->duration || !context->duration) {
        mxf_duration = filelist->file_count;
    } else {
        mxf_duration = context->duration;
    }

    ui32_t i = 0;
    /* read each input frame and write to the output mxf until duration is reached */
    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        if (context->slide == 0 || i == 0) {
            result = j2k_parser.OpenReadFrame(filelist->in[i], frame_buffer);

            if (context->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }

            if (context->encrypt_header_flag) {
                frame_buffer.PlaintextOffset(0);
            }
            i++;
        }
        result = mxf_writer.WriteFrame(frame_buffer, aes_context, hmac_context);
    }

    if (result == RESULT_ENDOFFILE) {
        result = RESULT_OK;
    }

    if (ASDCP_FAILURE(result)) {
        printf("not end of file\n");
        return result;
    }

    result = mxf_writer.Finalize();

    if (ASDCP_FAILURE(result)) {
        printf("failed to finalize\n");
        return result;
    }

    return result;
}

/* write out 3D j2k mxf file */
Result_t write_j2k_s_mxf(context_t *context, filelist_t *filelist, char *output_file) {
    AESEncContext*          aes_context = 0;
    HMACContext*            hmac_context = 0;
    JP2K::MXFSWriter         mxf_writer;
    JP2K::PictureDescriptor picture_desc;
    JP2K::CodestreamParser  j2k_parser_left;
    JP2K::CodestreamParser  j2k_parser_right;
    JP2K::FrameBuffer       frame_buffer_left(FRAME_BUFFER_SIZE);
    JP2K::FrameBuffer       frame_buffer_right(FRAME_BUFFER_SIZE);
    Kumu::FortunaRNG        rng;
    WriterInfo              writer_info;
    byte_t                  iv_buf[CBC_BLOCK_SIZE];
    Result_t                result = RESULT_OK;
    ui32_t                  mxf_duration;

    result = j2k_parser_left.OpenReadFrame(filelist->in[0], frame_buffer_left);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    result = j2k_parser_right.OpenReadFrame(filelist->in[1], frame_buffer_right);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[1]);
        return result;
    }

    Rational edit_rate(context->frame_rate,1);
    j2k_parser_left.FillPictureDescriptor(picture_desc);
    picture_desc.EditRate = edit_rate;

    writer_info.ProductVersion = OPEN_DCP_VERSION;
    writer_info.CompanyName = OPEN_DCP_NAME;
    writer_info.ProductName = OPEN_DCP_NAME;

    /* set the label type */
    if (context->ns == XML_NS_INTEROP) {
        writer_info.LabelSetType = LS_MXF_INTEROP;
    } else if ( context->ns == XML_NS_SMPTE ) {
        writer_info.LabelSetType = LS_MXF_SMPTE;
    } else {
        writer_info.LabelSetType = LS_MXF_UNKNOWN;
    }

    /* generate a random UUID for this essence */
    Kumu::GenRandomUUID(writer_info.AssetUUID);

    /* start encryption, if set */
    if (context->key_flag) {
        Kumu::GenRandomUUID(writer_info.ContextID);
        writer_info.EncryptedEssence = true;

        if (context->key_id) {
            memcpy(writer_info.CryptographicKeyID, context->key_id, UUIDlen);
        } else {
            rng.FillRandom(writer_info.CryptographicKeyID, UUIDlen);
        }

        aes_context = new AESEncContext;
        result = aes_context->InitKey(context->key_value);

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        result = aes_context->SetIVec(rng.FillRandom(iv_buf, CBC_BLOCK_SIZE));
        if (ASDCP_FAILURE(result)) {
            return result;
        }

        if (context->write_hmac) {
            writer_info.UsesHMAC = true;
            hmac_context = new HMACContext;
            result = hmac_context->InitKey(context->key_value, writer_info.LabelSetType);

            if (ASDCP_FAILURE(result)) {
                return result;
            }
        }
    }

    result = mxf_writer.OpenWrite(output_file, writer_info, picture_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }
     
    /* set the duration of the output mxf, set to half the filecount since it is 3D */
    if ((filelist->file_count/2) < context->duration || !context->duration) {
        mxf_duration = filelist->file_count/2;
    } else {
        mxf_duration = context->duration;
    }

    ui32_t i = 0;
    /* read each input frame and write to the output mxf until duration is reached */
    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        if (context->slide == 0 || i == 0) {
            result = j2k_parser_left.OpenReadFrame(filelist->in[i], frame_buffer_left);

            if (context->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }
            i++;

            result = j2k_parser_right.OpenReadFrame(filelist->in[i], frame_buffer_right);

            if (context->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }
            i++;

            if (context->encrypt_header_flag) {
                frame_buffer_left.PlaintextOffset(0);
            }

            if (context->encrypt_header_flag) {
                frame_buffer_right.PlaintextOffset(0);
            }
        }
        result = mxf_writer.WriteFrame(frame_buffer_left, JP2K::SP_LEFT, aes_context, hmac_context);
        result = mxf_writer.WriteFrame(frame_buffer_right, JP2K::SP_RIGHT, aes_context, hmac_context);
    }

    if (result == RESULT_ENDOFFILE) {
        result = RESULT_OK;
    }

    if (ASDCP_FAILURE(result)) {
        printf("not end of file\n");
        return result;
    }

    result = mxf_writer.Finalize();

    if (ASDCP_FAILURE(result)) {
        printf("failed to finalize\n");
        return result;
    }

    return result;
}

/* write out pcm audio mxf file */
Result_t write_pcm_mxf(context_t *context, filelist_t *filelist, char *output_file) {
    AESEncContext*       aes_context = 0;
    HMACContext*         hmac_context = 0;
    PCMParserList        pcm_parser;
    PCM::MXFWriter       mxf_writer;
    PCM::FrameBuffer     frame_buffer;
    PCM::AudioDescriptor audio_desc;
    WriterInfo           writer_info;
    byte_t               iv_buf[CBC_BLOCK_SIZE];
    Kumu::FortunaRNG     rng;
    Result_t             result = RESULT_OK; 
    ui32_t               mxf_duration;

    Rational edit_rate(context->frame_rate,1);

    result = pcm_parser.OpenRead(filelist->file_count,(const char **)filelist->in, edit_rate);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    pcm_parser.FillAudioDescriptor(audio_desc);
    audio_desc.EditRate = edit_rate;
    frame_buffer.Capacity(PCM::CalcFrameBufferSize(audio_desc));

    writer_info.ProductVersion = OPEN_DCP_VERSION;
    writer_info.CompanyName = OPEN_DCP_NAME;
    writer_info.ProductName = OPEN_DCP_NAME;

    /* set the label type */
    if (context->ns == XML_NS_INTEROP) {
        writer_info.LabelSetType = LS_MXF_INTEROP;
    } else if ( context->ns == XML_NS_SMPTE ) {
        writer_info.LabelSetType = LS_MXF_SMPTE;
    } else {
        writer_info.LabelSetType = LS_MXF_UNKNOWN;
    }

    /* generate a random UUID for this essence */
    Kumu::GenRandomUUID(writer_info.AssetUUID);

    if (context->key_flag) {
        Kumu::GenRandomUUID(writer_info.ContextID);
        writer_info.EncryptedEssence = true;

        if (context->key_id) {
            memcpy(writer_info.CryptographicKeyID, context->key_id, UUIDlen);
        } else {
            rng.FillRandom(writer_info.CryptographicKeyID, UUIDlen);
        }

        aes_context = new AESEncContext;
        result = aes_context->InitKey(context->key_value);

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        result = aes_context->SetIVec(rng.FillRandom(iv_buf, CBC_BLOCK_SIZE));

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        if (context->write_hmac) {
            writer_info.UsesHMAC = true;
            hmac_context = new HMACContext;
            result = hmac_context->InitKey(context->key_value, writer_info.LabelSetType);

            if (ASDCP_FAILURE(result)) {
                return result;
            }
        }
    }

    result = mxf_writer.OpenWrite(output_file, writer_info, audio_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    result = pcm_parser.Reset();
  
    if (ASDCP_FAILURE(result)) {
        printf("parser reset failed\n");
        return result;
    }

    if (!context->duration) {
        mxf_duration = 0xffffffff;
    } else {
        mxf_duration = context->duration;
    }

    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        result = pcm_parser.ReadFrame(frame_buffer);

        if (ASDCP_FAILURE(result)) {
            continue;
        } else {
            if (frame_buffer.Size() != frame_buffer.Capacity()) {
                printf("WARNING: Last frame read was short, PCM input is possibly not frame aligned.\n");
                result = RESULT_ENDOFFILE;
                continue;
            }
            result = mxf_writer.WriteFrame(frame_buffer, aes_context, hmac_context);
        }
    }

    if (result == RESULT_ENDOFFILE) {
        result = RESULT_OK;
    }

    if (ASDCP_FAILURE(result)) {
        printf("not end of file\n");
        return result;
    }

    result = mxf_writer.Finalize();

    if (ASDCP_FAILURE(result)) {
        printf("failed to finalize\n");
        return result;
    }

    return result;
}

/* write out mpeg2 mxf file */
Result_t write_mpeg2_mxf(context_t *context, filelist_t *filelist, char *output_file) {
    AESEncContext*         aes_context = 0;
    HMACContext*           hmac_context = 0;
    MPEG2::FrameBuffer     frame_buffer(FRAME_BUFFER_SIZE);
    MPEG2::Parser          mpeg2_parser;
    MPEG2::MXFWriter       mxf_writer;
    MPEG2::VideoDescriptor video_desc;
    WriterInfo             writer_info;
    byte_t                 iv_buf[CBC_BLOCK_SIZE];
    Kumu::FortunaRNG       rng;
    Result_t               result = RESULT_OK; 
    ui32_t                 mxf_duration;

    result = mpeg2_parser.OpenRead(filelist->in[0]);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    mpeg2_parser.FillVideoDescriptor(video_desc);

    writer_info.ProductVersion = OPEN_DCP_VERSION;
    writer_info.CompanyName = OPEN_DCP_NAME;
    writer_info.ProductName = OPEN_DCP_NAME;

     /* set the label type */
    if (context->ns == XML_NS_INTEROP) {
        writer_info.LabelSetType = LS_MXF_INTEROP;
    } else if ( context->ns == XML_NS_SMPTE ) {
        writer_info.LabelSetType = LS_MXF_SMPTE;
    } else {
        writer_info.LabelSetType = LS_MXF_UNKNOWN;
    }

    /* generate a random UUID for this essence */
    Kumu::GenRandomUUID(writer_info.AssetUUID);

    /* start encryption, if set */
    if (context->key_flag) {
        Kumu::GenRandomUUID(writer_info.ContextID);
        writer_info.EncryptedEssence = true;

        if (context->key_id) {
            memcpy(writer_info.CryptographicKeyID, context->key_id, UUIDlen);
        } else {
            rng.FillRandom(writer_info.CryptographicKeyID, UUIDlen);
        }

        aes_context = new AESEncContext;
        result = aes_context->InitKey(context->key_value);

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        result = aes_context->SetIVec(rng.FillRandom(iv_buf, CBC_BLOCK_SIZE));

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        if (context->write_hmac) {
            writer_info.UsesHMAC = true;
            hmac_context = new HMACContext;
            result = hmac_context->InitKey(context->key_value, writer_info.LabelSetType);

            if (ASDCP_FAILURE(result)) {
                return result;
            }
        }
    }

    result = mxf_writer.OpenWrite(output_file, writer_info, video_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    result = mpeg2_parser.Reset();
  
    if (ASDCP_FAILURE(result)) {
        printf("parser reset failed\n");
        return result;
    }

    if (!context->duration) {
        mxf_duration = 0xffffffff;
    } else {
        mxf_duration = context->duration;
    }

    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        result = mpeg2_parser.ReadFrame(frame_buffer);

        if (ASDCP_FAILURE(result)) {
            continue;
        }

        if (context->encrypt_header_flag) {
            frame_buffer.PlaintextOffset(0);
        }

        result = mxf_writer.WriteFrame(frame_buffer, aes_context, hmac_context);
    }

    if (result == RESULT_ENDOFFILE) {
        result = RESULT_OK;
    }

    if (ASDCP_FAILURE(result)) {
        printf("not end of file\n");
        return result;
    }

    result = mxf_writer.Finalize();

    if (ASDCP_FAILURE(result)) {
        printf("failed to finalize\n");
        return result;
    }

    return result;
}
