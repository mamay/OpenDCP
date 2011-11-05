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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>
#include <opendcp.h>
#include "mxf-writer.h"

void MainWindow::mxfConnectSlots()
{
    // connect slots
    connect(ui->mxfStereoscopicCheckBox, SIGNAL(stateChanged(int)), this, SLOT(mxfSetStereoscopicState()));
    connect(ui->mxfSoundRadio2, SIGNAL(clicked()), this, SLOT(mxfSetSoundState()));
    connect(ui->mxfSoundRadio5, SIGNAL(clicked()), this, SLOT(mxfSetSoundState()));
    connect(ui->mxfSourceTypeComboBox,SIGNAL(currentIndexChanged(int)),this, SLOT(mxfSourceTypeUpdate()));
    connect(ui->mxfButton,SIGNAL(clicked()),this,SLOT(mxfStart()));
    connect(ui->subCreateButton,SIGNAL(clicked()),this,SLOT(mxfCreateSubtitle()));

    connect(mxfWriterThread, SIGNAL(frameDone()), dMxfConversion, SLOT(step()));
    connect(mxfWriterThread, SIGNAL(finished()), this, SLOT(mxfDone()));

    // Picture input lines
    signalMapper.setMapping(ui->pictureLeftButton, ui->pictureLeftEdit);
    signalMapper.setMapping(ui->pictureRightButton, ui->pictureRightEdit);

    // Sound input lines
    signalMapper.setMapping(ui->aLeftButton, ui->aLeftEdit);
    signalMapper.setMapping(ui->aRightButton, ui->aRightEdit);
    signalMapper.setMapping(ui->aCenterButton, ui->aCenterEdit);
    signalMapper.setMapping(ui->aSubButton, ui->aSubEdit);
    signalMapper.setMapping(ui->aLeftSButton, ui->aLeftSEdit);
    signalMapper.setMapping(ui->aRightSButton, ui->aRightSEdit);

    // Subtitle input lines
    signalMapper.setMapping(ui->subInButton, ui->subInEdit);

    // MXF output lines
    signalMapper.setMapping(ui->pMxfOutButton, ui->pMxfOutEdit);
    signalMapper.setMapping(ui->aMxfOutButton, ui->aMxfOutEdit);
    signalMapper.setMapping(ui->sMxfOutButton, ui->sMxfOutEdit);

    // Signal Connections
    connect(ui->pictureLeftButton, SIGNAL(clicked()),&signalMapper, SLOT(map()));
    connect(ui->pictureRightButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->aLeftButton, SIGNAL(clicked()),&signalMapper, SLOT(map()));
    connect(ui->aRightButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->aCenterButton, SIGNAL(clicked()),&signalMapper, SLOT(map()));
    connect(ui->aSubButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->aLeftSButton, SIGNAL(clicked()),&signalMapper, SLOT(map()));
    connect(ui->aRightSButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->pMxfOutButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->aMxfOutButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->subInButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->sMxfOutButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
}

void MainWindow::mxfSourceTypeUpdate() {
    if (ui->mxfSourceTypeComboBox->currentIndex() == 0) {
        ui->mxfStereoscopicCheckBox->setEnabled(1);
        ui->mxfTypeComboBox->setCurrentIndex(1);
        mxfSetStereoscopicState();
    } else {
        ui->mxfStereoscopicCheckBox->setEnabled(0);
        ui->mxfStereoscopicCheckBox->setChecked(0);
        ui->mxfTypeComboBox->setCurrentIndex(0);
        mxfSetStereoscopicState();
    }
}

void MainWindow::mxfSetStereoscopicState() {
    int value = ui->mxfStereoscopicCheckBox->checkState();

    if (value) {
        ui->pictureLeftLabel->setText(tr("Left:"));
        ui->pictureRightLabel->show();
        ui->pictureRightEdit->show();
        ui->pictureRightButton->show();
    } else {
        if (ui->mxfSourceTypeComboBox->currentIndex() == 0) {
            ui->pictureLeftLabel->setText(tr("Directory:"));
        } else {
            ui->pictureLeftLabel->setText(tr("M2V File:"));
        }
        ui->pictureRightLabel->hide();
        ui->pictureRightEdit->hide();
        ui->pictureRightButton->hide();
    }
}

void MainWindow::mxfSetSoundState() {
    if (ui->mxfSoundRadio2->isChecked()) {
        ui->aCenterLabel->setEnabled(0);
        ui->aCenterEdit->setEnabled(0);
        ui->aCenterButton->setEnabled(0);
        ui->aLeftSLabel->setEnabled(0);
        ui->aLeftSEdit->setEnabled(0);
        ui->aLeftSButton->setEnabled(0);
        ui->aRightSLabel->setEnabled(0);
        ui->aRightSEdit->setEnabled(0);
        ui->aRightSButton->setEnabled(0);
        ui->aSubLabel->setEnabled(0);
        ui->aSubEdit->setEnabled(0);
        ui->aSubButton->setEnabled(0);
    } else {
        ui->aCenterLabel->setEnabled(1);
        ui->aCenterEdit->setEnabled(1);
        ui->aCenterButton->setEnabled(1);
        ui->aLeftSLabel->setEnabled(1);
        ui->aLeftSEdit->setEnabled(1);
        ui->aLeftSButton->setEnabled(1);
        ui->aRightSLabel->setEnabled(1);
        ui->aRightSEdit->setEnabled(1);
        ui->aRightSButton->setEnabled(1);
        ui->aSubLabel->setEnabled(1);
        ui->aSubEdit->setEnabled(1);
        ui->aSubButton->setEnabled(1);
    }
}

void MainWindow::mxfDone() {
   dMxfConversion->finished(mxfWriterThread->success);
}

void MainWindow::mxfStart() {
    if (ui->aMxfOutEdit->text().isEmpty() && ui->pMxfOutEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Destination file needed"),tr("Please specify at least one destination MXF file."));
        return;
    }
    if (!ui->pMxfOutEdit->text().isEmpty()) {
        if (ui->mxfStereoscopicCheckBox->checkState()) {
            if ((ui->pictureLeftEdit->text().isEmpty() || ui->pictureRightEdit->text().isEmpty())) {
                QMessageBox::critical(this, tr("Source content needed"),tr("Please select left and right source image directories to generate a stereoscopic MXF file."));
                return;
            }
        } else {
            if ((ui->pictureLeftEdit->text().isEmpty())) {
                QMessageBox::critical(this, tr("Source content needed"),tr("Please select a source image directory to generate an MXF picture file."));
                return;
            }
        }
    }
    if (!ui->pictureLeftEdit->text().isEmpty() || !ui->pictureRightEdit->text().isEmpty()) {
        if (ui->pMxfOutEdit->text().isEmpty()) {
            QMessageBox::critical(this, tr("Destination file needed"),tr("Please select a destination picture MXF file."));
            return;
        }
    }
    if (!ui->aMxfOutEdit->text().isEmpty()) {
        if (ui->aLeftEdit->text().isEmpty() ||
            ui->aRightEdit->text().isEmpty()) {
            QMessageBox::critical(this, tr("Source content needed"),tr("Please specify left and right wav files to generate an MXF sound file."));
                return;
        }
        if (ui->mxfSoundRadio5->isChecked()) {
            if (ui->aCenterEdit->text().isEmpty() ||
                ui->aSubEdit->text().isEmpty() ||
                ui->aLeftSEdit->text().isEmpty() ||
                ui->aLeftSEdit->text().isEmpty()) {
                QMessageBox::critical(this, tr("Source content needed"),tr("Please specify at least 5.1 wav files to generate an MXF sound file."));
                    return;
            }
        }
    }

    // check frame rate
    if (ui->mxfStereoscopicCheckBox->checkState()) {
        if (ui->mxfFrameRateComboBox->currentText().toInt() > 30) {
            QMessageBox::critical(this, tr("Invalid frame rate"),tr("Stereoscopic MXF only supports 24, 25, or 30 fps."));
            return;
        }
    }

    // create picture mxf file
    if (!ui->pMxfOutEdit->text().isEmpty()) {
        mxfCreatePicture();
    }

    // create sound mxf file
    if (!ui->aMxfOutEdit->text().isEmpty()) {
        mxfCreateAudio();
    }
}

void MainWindow::mxfCreateSubtitle() {
    if (ui->subInEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Source subtitle needed"),tr("Please specify an input subtitle XML file."));
        return;
    }

    if (ui->sMxfOutEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Destination file needed"),tr("Please specify the destinaton subtitle MXF file."));
        return;
    }

    opendcp_t *mxfContext = create_opendcp();

    // process options
    mxfContext->log_level = 0;

    if (ui->mxfTypeComboBox->currentIndex() == 0) {
        mxfContext->ns = XML_NS_INTEROP;
    } else {
        mxfContext->ns = XML_NS_SMPTE;
    }

    filelist_t *fileList = (filelist_t*) malloc(sizeof(filelist_t));

    fileList->in = (char**) malloc(fileList->file_count*sizeof(char*));
    fileList->in[0] = new char [ui->subInEdit->text().toStdString().size()+1];
    strcpy(fileList->in[0], ui->subInEdit->text().toStdString().c_str());
    fileList->file_count = 1;

    char *outputFile = new char [ui->sMxfOutEdit->text().toStdString().size()+1];
    strcpy(outputFile, ui->sMxfOutEdit->text().toStdString().c_str());

    if (write_mxf(mxfContext,fileList,outputFile) != 0 )  {
        QMessageBox::critical(this, tr("MXF Creation Error"),
                             tr("Subtitle MXF creation failed."));
    }

    delete_opendcp(mxfContext);

    // free filelist
    for (int x=0;x<fileList->file_count;x++) {
        delete[] fileList->in[x];
    }

    if (fileList) {
        free(fileList);
    }

    delete[] outputFile;

    return;
}

void MainWindow::mxfCreateAudio() {
    opendcp_t     *mxfContext = create_opendcp();
    QFileInfoList inputList;
    QString       outputFile;

    // process options
    mxfContext->log_level = 0;

    if (ui->mxfTypeComboBox->currentIndex() == 0) {
        mxfContext->ns = XML_NS_INTEROP;
    } else {
        mxfContext->ns = XML_NS_SMPTE;
    }

    mxfContext->frame_rate = ui->mxfFrameRateComboBox->currentText().toInt();
    mxfContext->stereoscopic = 0;

    if (ui->mxfSoundRadio2->isChecked()) {
        inputList.append(QFileInfo(ui->aLeftEdit->text()));
        inputList.append(QFileInfo(ui->aRightEdit->text()));
    } else {
        inputList.append(QFileInfo(ui->aLeftEdit->text()));
        inputList.append(QFileInfo(ui->aRightEdit->text()));
        inputList.append(QFileInfo(ui->aCenterEdit->text()));
        inputList.append(QFileInfo(ui->aSubEdit->text()));
        inputList.append(QFileInfo(ui->aLeftSEdit->text()));
        inputList.append(QFileInfo(ui->aRightSEdit->text()));
    }

    outputFile = ui->aMxfOutEdit->text();
    mxfWriterThread->setMxfInputs(mxfContext, inputList, outputFile);

    dMxfConversion->init(inputList.size(), outputFile);
    mxfWriterThread->start();
    dMxfConversion->exec();

    if (!mxfWriterThread->success)  {
        QMessageBox::critical(this, tr("MXF Creation Error"),
                             tr("Sound MXF creation failed."));
    }

    delete_opendcp(mxfContext);

    return;
}

void MainWindow::mxfCreatePicture() {
    QDir          pLeftDir;
    QDir          pRightDir;
    QFileInfoList pLeftList;
    QFileInfoList pRightList;
    QFileInfoList inputList;
    QString       outputFile;
    QString       msg;

    opendcp_t *mxfContext = create_opendcp();

    // process options
    mxfContext->log_level = 0;

    if (ui->mxfTypeComboBox->currentIndex() == 0) {
        mxfContext->ns = XML_NS_INTEROP;
    } else {
        mxfContext->ns = XML_NS_SMPTE;
    }
    mxfContext->frame_rate = ui->mxfFrameRateComboBox->currentText().toInt();
    mxfContext->stereoscopic = 0;

    // set log level
    dcp_set_log_level(mxfContext->log_level);

    if (ui->mxfSourceTypeComboBox->currentIndex() == 0) {
        pLeftDir.cd(ui->pictureLeftEdit->text());
        pLeftDir.setNameFilters(QStringList() << "*.j2c");
        pLeftDir.setFilter(QDir::Files | QDir::NoSymLinks);
        pLeftDir.setSorting(QDir::Name);
        pLeftList = pLeftDir.entryInfoList();
    } else {
        pLeftList.append(ui->pictureLeftEdit->text());
    }

    if (ui->mxfStereoscopicCheckBox->checkState()) {
        mxfContext->stereoscopic = 1;
        pRightDir.cd(ui->pictureRightEdit->text());
        pRightDir.setNameFilters(QStringList() << "*.j2c");
        pRightDir.setFilter(QDir::Files | QDir::NoSymLinks);
        pRightDir.setSorting(QDir::Name);
        pRightList = pRightDir.entryInfoList();

        if (pLeftList.size() != pRightList.size()) {
            QMessageBox::critical(this, tr("File Count Mismatch"),
                                 tr("The left and right image directories have different file counts. They must be the same. Please fix and try again."));
            goto Done;
        }
    }

    if (checkFileSequence(pLeftDir.entryList()) != DCP_SUCCESS) {
        goto Done;
    }

    if (ui->mxfStereoscopicCheckBox->checkState() && checkFileSequence(pRightDir.entryList()) != DCP_SUCCESS) {
        goto Done;
    }

    for (int x=0;x<pLeftList.size();x++) {
        inputList.append(pLeftList.at(x));
        if (ui->mxfStereoscopicCheckBox->checkState()) {
            inputList.append(pRightList.at(x));
        }
    }

    outputFile = ui->pMxfOutEdit->text();

    if (inputList.size() < 1) {
        QMessageBox::critical(this, tr("MXF Creation Error"), tr("No input files found"));
        goto Done;
    } else {
        mxfWriterThread->setMxfInputs(mxfContext,inputList,outputFile);
        dMxfConversion->init(inputList.size(), outputFile);
        mxfWriterThread->start();
        dMxfConversion->exec();
        if (!mxfWriterThread->success)  {
            QMessageBox::critical(this, tr("MXF Creation Error"), tr("Picture MXF creation failed."));
       }
    }

Done:

    delete_opendcp(mxfContext);
    return;
}
