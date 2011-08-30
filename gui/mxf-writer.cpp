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
}

void MxfWriter::reset()
{
  // emit default
}

void MxfWriter::run()
{
    Result_t result = writeMxf();
    if (result = RESULT_OK) {
        success = 1;
    } else {
        success = 0;
    }
    emit finished();
}

void MxfWriter::setMxfInputs(opendcp_t *opendcp, filelist_t *filelist, char *output_file)
{
    opendcpMxf = opendcp;
    filelistMxf = filelist;
    outputFileMxf = output_file;
}

Result_t MxfWriter::writeMxf()
{
    Result_t      result = RESULT_OK;
    EssenceType_t essence_type;

    result = ASDCP::RawEssenceType(filelistMxf->in[0], essence_type);

    if (ASDCP_FAILURE(result)) {
        printf("Could determine essence type of  %s\n",filelistMxf->in[0]);
        result = RESULT_FAIL;
    }

    switch (essence_type) {
        case ESS_JPEG_2000:
        case ESS_JPEG_2000_S:
            if ( opendcpMxf->stereoscopic )
                result = writeJ2kStereoscopicMxf(opendcpMxf,filelistMxf,outputFileMxf);
            else
                result = writeJ2kMxf(opendcpMxf,filelistMxf,outputFileMxf);
            break;
        case ESS_PCM_24b_48k:
        case ESS_PCM_24b_96k:
            result = writePcmMxf(opendcpMxf,filelistMxf,outputFileMxf);
            break;
        case ESS_MPEG2_VES:
            result = writeMpeg2Mxf(opendcpMxf,filelistMxf,outputFileMxf);
            break;
        case ESS_TIMED_TEXT:
            result = writeTTMxf(opendcpMxf,filelistMxf,outputFileMxf);
            break;
        case ESS_UNKNOWN:
            break;
    }

    if (ASDCP_FAILURE(result)) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

Result_t MxfWriter::writeJ2kMxf(opendcp_t *opendcp, filelist_t *filelist, char *output_file) {
    JP2K::MXFWriter         mxf_writer;
    JP2K::PictureDescriptor picture_desc;
    JP2K::CodestreamParser  j2k_parser;
    JP2K::FrameBuffer       frame_buffer(FRAME_BUFFER_SIZE);
    writer_info_t           writer_info;
    Result_t                result = RESULT_OK;
    ui32_t                  start_frame;
    ui32_t                  mxf_duration;

    // set the starting frame
    if (opendcp->mxf.start_frame && filelist->file_count >= (opendcp->mxf.start_frame-1)) {
        start_frame = opendcp->mxf.start_frame - 1; // adjust for zero base
    } else {
        start_frame = 0;
    }

    result = j2k_parser.OpenReadFrame(filelist->in[start_frame], frame_buffer);

    if (ASDCP_FAILURE(result)) {
        printf("Failed to open file %s\n",filelist->in[start_frame]);
        return result;
    }

    Rational edit_rate(opendcp->frame_rate,1);
    j2k_parser.FillPictureDescriptor(picture_desc);
    picture_desc.EditRate = edit_rate;

    fillWriterInfo(opendcp, &writer_info);

    result = mxf_writer.OpenWrite(output_file, writer_info.info, picture_desc);

    if (ASDCP_FAILURE(result)) {
        printf("failed to open output file %s\n",output_file);
        return result;
    }

    // set the duration of the output mxf
    if (opendcp->mxf.duration && filelist->file_count >= opendcp->mxf.duration) {
        mxf_duration = opendcp->mxf.duration;
    } else {
        mxf_duration = filelist->file_count;
    }

    ui32_t i = start_frame;

    // read each input frame and write to the output mxf until duration is reached
    while (ASDCP_SUCCESS(result) && mxf_duration--) {
        if (opendcp->slide == 0 || i == start_frame) {
            result = j2k_parser.OpenReadFrame(filelist->in[i], frame_buffer);

            if (opendcp->delete_intermediate) {
                unlink(filelist->in[i]);
            }

            if (ASDCP_FAILURE(result)) {
                printf("Failed to open file %s\n",filelist->in[i]);
                return result;
            }

            if (opendcp->encrypt_header_flag) {
                frame_buffer.PlaintextOffset(0);
            }
            qDebug() << "Duration: " << mxf_duration;
            i++;
            emit frameDone();
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

Result_t MxfWriter::writeJ2kStereoscopicMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{
    int hello;
}

Result_t MxfWriter::writePcmMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{

}

Result_t MxfWriter::writeMpeg2Mxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{

}

Result_t MxfWriter::writeTTMxf(opendcp_t *opendcp,filelist_t *filelist, char *output_file)
{

}

/*
bool MxfWriter::writeImage(Image &my_image, QString format, int quality, QString out)
{
    my_image.magick(format.toStdString());

    Image bgImg;
    bgImg.size(Magick::Geometry(my_image.columns(), my_image.rows()));

    QStringList excludedFormats;
    excludedFormats << "png" << "gif";

    if (!excludedFormats.contains(format, Qt::CaseInsensitive)) {
        bgImg.read("xc:#FFFFFF");
        bgImg.label("bgImg");
        bgImg.depth(my_image.depth());

        bgImg.composite(my_image, Magick::Geometry(bgImg.columns(),bgImg.rows()), Magick::DissolveCompositeOp );

        my_image = bgImg;
    }

    bool converted = false;

    if (quality != -1)
        my_image.quality(quality);

    try {
        my_image.write(out.toStdString());

        converted = true;
    }
    catch (Error& my_error) {
        converted = false;
    }

    return converted;
}
*/

void MxfWriter::stopProcess()
{
    //m_process_stopped = 1;
}
