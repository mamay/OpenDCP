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

#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>

#include <AS_DCP.h>
#include <KM_fileio.h>
#include <KM_prng.h>
#include <KM_memio.h>
#include <KM_util.h>
#include <PCMParserList.h>
#include <openssl/sha.h>

#include "opendcp.h"
#include "mxf-writer.h"
#include "dialogmxfconversion.h"

using namespace ASDCP;

MxfWriter::MxfWriter(QObject *parent)
    : QThread(parent)
{
    reset();

    //connect(this, SIGNAL(finished()), this, SLOT(mxfCompleted()));
}

MxfWriter::~MxfWriter()
{

}

void MxfWriter::reset()
{
    cancelled = 0;
    success = 0;
}

void MxfWriter::conversionCompleted()
{
    //emit nextConversion();
}

Result_t MxfWriter::fillWriterInfo(opendcp_t *opendcp, writer_info_t *writer_info) {
    Kumu::FortunaRNG        rng;
    byte_t                  iv_buf[CBC_BLOCK_SIZE];
    Result_t                result = RESULT_OK;

    writer_info->info.ProductVersion = OPEN_DCP_VERSION;
    writer_info->info.CompanyName = OPEN_DCP_NAME;
    writer_info->info.ProductName = OPEN_DCP_NAME;

    // set the label type
    if (opendcp->ns == XML_NS_INTEROP) {
        writer_info->info.LabelSetType = LS_MXF_INTEROP;
    } else if (opendcp->ns == XML_NS_SMPTE) {
        writer_info->info.LabelSetType = LS_MXF_SMPTE;
    } else {
        writer_info->info.LabelSetType = LS_MXF_UNKNOWN;
    }

    // generate a random UUID for this essence
    Kumu::GenRandomUUID(writer_info->info.AssetUUID);

    // start encryption, if set
    if (opendcp->key_flag) {
        Kumu::GenRandomUUID(writer_info->info.ContextID);
        writer_info->info.EncryptedEssence = true;

        if (opendcp->key_id) {
            memcpy(writer_info->info.CryptographicKeyID, opendcp->key_id, UUIDlen);
        } else {
            rng.FillRandom(writer_info->info.CryptographicKeyID, UUIDlen);
        }

        writer_info->aes_context = new AESEncContext;
        result = writer_info->aes_context->InitKey(opendcp->key_value);

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        result = writer_info->aes_context->SetIVec(rng.FillRandom(iv_buf, CBC_BLOCK_SIZE));

        if (ASDCP_FAILURE(result)) {
            return result;
        }

        if (opendcp->write_hmac) {
            writer_info->info.UsesHMAC = true;
            writer_info->hmac_context = new HMACContext;
            result = writer_info->hmac_context->InitKey(opendcp->key_value, writer_info->info.LabelSetType);

            if (ASDCP_FAILURE(result)) {
                return result;
            }
        }
    }
    return result;
}

void MxfWriter::run()
{
    Result_t result = writeMxf();

    if (result == RESULT_OK) {
        success = 1;
    } else {
        success = 0;
    }

    emit finished();
}

void MxfWriter::setMxfInputs(opendcp_t *opendcp, QFileInfoList fileList, QString outputFile)
{
    opendcpMxf    = opendcp;
    mxfFileList   = fileList;
    mxfOutputFile = outputFile;
}

Result_t MxfWriter::writeMxf()
{
    Result_t      result = RESULT_OK;
    EssenceType_t essence_type;

    result = ASDCP::RawEssenceType(mxfFileList.at(0).absoluteFilePath().toAscii().constData(), essence_type);

    if (ASDCP_FAILURE(result)) {
        printf("Could determine essence type of  %s\n",mxfFileList.at(0).absoluteFilePath().toAscii().constData());
        result = RESULT_FAIL;
    }

    switch (essence_type) {
        case ESS_JPEG_2000:
        case ESS_JPEG_2000_S:
            if ( opendcpMxf->stereoscopic ) {
                //result = writeJ2kStereoscopicMxf(opendcpMxf,mxfFileList,mxfOutputFile);
            } else {
                result = writeJ2kMxf(opendcpMxf,mxfFileList,mxfOutputFile);
            }
            break;
        case ESS_PCM_24b_48k:
        case ESS_PCM_24b_96k:
           // result = writePcmMxf(opendcpMxf,mxfFileList,mxfOutputFile);
            break;
        case ESS_MPEG2_VES:
          //  result = writeMpeg2Mxf(opendcpMxf,mxfFileList,mxfOutputFile);
            break;
        case ESS_TIMED_TEXT:
          //  result = writeTTMxf(opendcpMxf,mxfFileList,mxfOutputFile);
            break;
        case ESS_UNKNOWN:
            result = RESULT_FAIL;
            break;
    }

    if (ASDCP_FAILURE(result)) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

Result_t MxfWriter::writeJ2kMxf(opendcp_t *opendcp, QFileInfoList mxfFileList, QString mxfOuputFile) {
    JP2K::MXFWriter         mxf_writer;
    JP2K::PictureDescriptor picture_desc;
    JP2K::CodestreamParser  j2k_parser;
    JP2K::FrameBuffer       frame_buffer(FRAME_BUFFER_SIZE);
    writer_info_t           writer_info;
    Result_t                result = RESULT_OK;
    ui32_t                  start_frame;
    ui32_t                  mxf_duration;

    // set the starting frame
    if (opendcp->mxf.start_frame && mxfFileList.size() >= (opendcp->mxf.start_frame-1)) {
        start_frame = opendcp->mxf.start_frame - 1; // adjust for zero base
    } else {
        start_frame = 0;
    }

    result = j2k_parser.OpenReadFrame(mxfFileList.at(start_frame).absoluteFilePath().toAscii().constData(), frame_buffer);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",mxfFileList.at(start_frame).absoluteFilePath().toAscii().constData());
        return result;
    }

    Rational edit_rate(opendcp->frame_rate,1);
    j2k_parser.FillPictureDescriptor(picture_desc);
    picture_desc.EditRate = edit_rate;

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(mxfOutputFile.toAscii().constData(), writer_info.info, picture_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",mxfOutputFile.toAscii().constData());
        return result;
    }

    // set the duration of the output mxf
    if (opendcp->mxf.duration && mxfFileList.size() >= opendcp->mxf.duration) {
        mxf_duration = opendcp->mxf.duration;
    } else {
        mxf_duration = mxfFileList.size();
    }

    ui32_t i = start_frame;

    // read each input frame and write to the output mxf until duration is reached
    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        if ((opendcp->slide == 0 || i == start_frame) && cancelled == 0) {
            result = j2k_parser.OpenReadFrame(mxfFileList.at(i).absoluteFilePath().toAscii().constData(), frame_buffer);

            if (opendcp->delete_intermediate) {
                unlink(mxfFileList.at(i).absoluteFilePath().toAscii().constData());
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",mxfFileList.at(i).absoluteFilePath().toAscii().constData());
                return result;
            }

            if (opendcp->encrypt_header_flag) {
                frame_buffer.PlaintextOffset(0);
            }
            i++;
        }
        result = mxf_writer.WriteFrame(frame_buffer, writer_info.aes_context, writer_info.hmac_context);
        emit frameDone();
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

Result_t MxfWriter::writeJ2kStereoscopicMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{
    JP2K::MXFSWriter         mxf_writer;
    JP2K::PictureDescriptor picture_desc;
    JP2K::CodestreamParser  j2k_parser_left;
    JP2K::CodestreamParser  j2k_parser_right;
    JP2K::FrameBuffer       frame_buffer_left(FRAME_BUFFER_SIZE);
    JP2K::FrameBuffer       frame_buffer_right(FRAME_BUFFER_SIZE);
    writer_info_t           writer_info;
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

    Rational edit_rate(opendcp->frame_rate,1);
    j2k_parser_left.FillPictureDescriptor(picture_desc);
    picture_desc.EditRate = edit_rate;

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(output_file, writer_info.info, picture_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    /* set the duration of the output mxf, set to half the filecount since it is 3D */
    if ((filelist->file_count/2) < opendcp->duration || !opendcp->duration) {
        mxf_duration = filelist->file_count/2;
    } else {
        mxf_duration = opendcp->duration;
    }

    ui32_t i = 0;
    /* read each input frame and write to the output mxf until duration is reached */
    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        if ((opendcp->slide == 0 || i == 0) && cancelled == 0) {
            result = j2k_parser_left.OpenReadFrame(filelist->in[i], frame_buffer_left);

            if (opendcp->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }
            i++;

            result = j2k_parser_right.OpenReadFrame(filelist->in[i], frame_buffer_right);

            if (opendcp->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }
            i++;

            if (opendcp->encrypt_header_flag) {
                frame_buffer_left.PlaintextOffset(0);
            }

            if (opendcp->encrypt_header_flag) {
                frame_buffer_right.PlaintextOffset(0);
            }
        }
        result = mxf_writer.WriteFrame(frame_buffer_left, JP2K::SP_LEFT, writer_info.aes_context, writer_info.hmac_context);
        result = mxf_writer.WriteFrame(frame_buffer_right, JP2K::SP_RIGHT, writer_info.aes_context, writer_info.hmac_context);
        emit frameDone();
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

Result_t MxfWriter::writePcmMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{
    PCMParserList        pcm_parser;
    PCM::MXFWriter       mxf_writer;
    PCM::FrameBuffer     frame_buffer;
    PCM::AudioDescriptor audio_desc;
    writer_info_t        writer_info;
    Result_t             result = RESULT_OK;
    ui32_t               mxf_duration;

    Rational edit_rate(opendcp->frame_rate,1);

    result = pcm_parser.OpenRead(filelist->file_count,(const char **)filelist->in, edit_rate);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    pcm_parser.FillAudioDescriptor(audio_desc);
    audio_desc.EditRate = edit_rate;
    frame_buffer.Capacity(PCM::CalcFrameBufferSize(audio_desc));

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(output_file, writer_info.info, audio_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    result = pcm_parser.Reset();

    if (ASDCP_FAILURE(result)) {
        printf("parser reset failed\n");
        return result;
    }

    if (!opendcp->duration) {
        mxf_duration = 0xffffffff;
    } else {
        mxf_duration = opendcp->duration;
    }

    while (ASDCP_SUCCESS(result) && mxf_duration-- && cancelled == 0) {
        result = pcm_parser.ReadFrame(frame_buffer);

        if (ASDCP_FAILURE(result)) {
            continue;
        } else {
            if (frame_buffer.Size() != frame_buffer.Capacity()) {
                printf("WARNING: Last frame read was short, PCM input is possibly not frame aligned.\n");
                result = RESULT_ENDOFFILE;
                continue;
            }
            result = mxf_writer.WriteFrame(frame_buffer, writer_info.aes_context, writer_info.hmac_context);
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

Result_t MxfWriter::writeMpeg2Mxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{
    MPEG2::FrameBuffer     frame_buffer(FRAME_BUFFER_SIZE);
    MPEG2::Parser          mpeg2_parser;
    MPEG2::MXFWriter       mxf_writer;
    MPEG2::VideoDescriptor video_desc;
    writer_info_t          writer_info;
    Result_t               result = RESULT_OK;
    ui32_t                 mxf_duration;

    result = mpeg2_parser.OpenRead(filelist->in[0]);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    mpeg2_parser.FillVideoDescriptor(video_desc);

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(output_file, writer_info.info, video_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    result = mpeg2_parser.Reset();

    if (ASDCP_FAILURE(result)) {
        printf("parser reset failed\n");
        return result;
    }

    if (!opendcp->duration) {
        mxf_duration = 0xffffffff;
    } else {
        mxf_duration = opendcp->duration;
    }

    while (ASDCP_SUCCESS(result) && mxf_duration-- && cancelled == 0) {
        result = mpeg2_parser.ReadFrame(frame_buffer);

        if (ASDCP_FAILURE(result)) {
            continue;
        }

        if (opendcp->encrypt_header_flag) {
            frame_buffer.PlaintextOffset(0);
        }

        result = mxf_writer.WriteFrame(frame_buffer, writer_info.aes_context, writer_info.hmac_context);
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

Result_t MxfWriter::writeTTMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{
    TimedText::DCSubtitleParser    tt_parser;
    TimedText::MXFWriter           mxf_writer;
    TimedText::FrameBuffer         frame_buffer(FRAME_BUFFER_SIZE);
    TimedText::TimedTextDescriptor tt_desc;
    TimedText::ResourceList_t::const_iterator resource_iterator;
    writer_info_t                  writer_info;
    std::string                    xml_doc;
    Result_t                       result = RESULT_OK;

    result = tt_parser.OpenRead(filelist->in[0]);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[0]);
        return result;
    }

    tt_parser.FillTimedTextDescriptor(tt_desc);

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(output_file, writer_info.info, tt_desc);
    result = tt_parser.ReadTimedTextResource(xml_doc);

    if (ASDCP_FAILURE(result)) {
        printf("Could not read Time Text Resource\n");
        return result;
    }

    result = mxf_writer.WriteTimedTextResource(xml_doc, writer_info.aes_context, writer_info.hmac_context);

    if (ASDCP_FAILURE(result)) {
        printf("Could not write Time Text Resource\n");
        return result;
    }

    resource_iterator = tt_desc.ResourceList.begin();

    while (ASDCP_SUCCESS(result) && resource_iterator != tt_desc.ResourceList.end() && cancelled == 0) {
        result = tt_parser.ReadAncillaryResource((*resource_iterator++).ResourceID, frame_buffer);

        if (ASDCP_FAILURE(result)) {
          printf("Could not read Time Text Resource\n");
          return result;
        }

        result = mxf_writer.WriteAncillaryResource(frame_buffer, writer_info.aes_context, writer_info.hmac_context);
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

void MxfWriter::cancel()
{
    //m_process_stopped = 1;
}
