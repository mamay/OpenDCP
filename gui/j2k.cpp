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
#include <QPixmap>
#include <stdio.h>
#include <opendcp.h>

void MainWindow::j2kConnectSlots()
{
    // connect slots
    connect(ui->stereoscopicCheckBox, SIGNAL(stateChanged(int)), this, SLOT(j2kSetStereoscopicState()));
    connect(ui->bwSlider,SIGNAL(valueChanged(int)),this, SLOT(j2kBwSliderUpdate()));
    connect(ui->encodeButton,SIGNAL(clicked()),this,SLOT(j2kStart()));
    connect(ui->profileComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(j2kCinemaProfileUpdate()));

    // set signal mapper to handle file dialogs
    signalMapper.setMapping(ui->inImageLeftButton, ui->inImageLeftEdit);
    signalMapper.setMapping(ui->inImageRightButton, ui->inImageRightEdit);
    signalMapper.setMapping(ui->outJ2kLeftButton, ui->outJ2kLeftEdit);
    signalMapper.setMapping(ui->outJ2kRightButton, ui->outJ2kRightEdit);

    // connect j2k signals
    connect(ui->inImageLeftButton, SIGNAL(clicked()),&signalMapper, SLOT(map()));
    connect(ui->inImageRightButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->outJ2kLeftButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));
    connect(ui->outJ2kRightButton, SIGNAL(clicked()), &signalMapper, SLOT(map()));

    // update file
    connect(ui->inImageLeftEdit, SIGNAL(textChanged(QString)),this,SLOT(j2kCheckLeftInputFiles()));
    connect(ui->inImageRightEdit, SIGNAL(textChanged(QString)),this,SLOT(j2kCheckRightInputFiles()));
    connect(ui->endSpinBox, SIGNAL(valueChanged(int)),this,SLOT(j2kUpdateEndSpinBox()));
}

void MainWindow::j2kSetStereoscopicState() {
    int value = ui->stereoscopicCheckBox->checkState();

    if (value) {
        ui->inImageLeft->setText(tr("Left:"));
        ui->outJ2kLeft->setText(tr("Left:"));
        ui->inImageRight->show();
        ui->inImageRightEdit->show();
        ui->inImageRightButton->show();
        ui->outJ2kRight->show();
        ui->outJ2kRightEdit->show();
        ui->outJ2kRightButton->show();

        ui->bwSlider->setValue(250);
    } else {
        ui->inImageLeft->setText(tr("Directory:"));
        ui->outJ2kLeft->setText(tr("Directory:"));
        ui->inImageRight->hide();
        ui->inImageRightEdit->hide();
        ui->inImageRightButton->hide();
        ui->outJ2kRight->hide();
        ui->outJ2kRightEdit->hide();
        ui->outJ2kRightButton->hide();
        ui->bwSlider->setValue(125);
    }
}

void MainWindow::j2kCinemaProfileUpdate() {
    if (ui->profileComboBox->currentIndex() == 0) {
#ifdef Q_WS_WIN
        //ui->threadsSpinBox->setMaximum(6);
#endif
        ui->threadsSpinBox->setMaximum(QThreadPool::globalInstance()->maxThreadCount());
        ui->threadsSpinBox->setValue(QThread::idealThreadCount());
    } else {
#ifdef Q_WS_WIN
        //ui->threadsSpinBox->setMaximum(2);
#endif
        ui->threadsSpinBox->setMaximum(QThreadPool::globalInstance()->maxThreadCount());
        ui->threadsSpinBox->setValue(QThread::idealThreadCount());
    }
}

void MainWindow::j2kBwSliderUpdate() {
    int bw = ui->bwSlider->value();
    QString string;

    string.sprintf("%d mb/s",bw);
    ui->bwValueLabel->setText(string);
}

// globals for threads
opendcp_t *context;
int iterations = 0;
QFileInfoList inLeftList;
QFileInfoList inRightList;
QString outLeftDir;
QString outRightDir;

void j2kEncode(int &iteration)
{
    QFileInfo fileinfo;
    QString inputFile;
    QString outputFile;
    QString baseName;
    int status = 0;

    fileinfo = inLeftList.at(iteration);
    inputFile = fileinfo.absoluteFilePath();
    baseName = fileinfo.completeBaseName();

    outputFile.sprintf("%s/%s.%s",outLeftDir.toAscii().constData(),baseName.toAscii().constData(),"j2c");

    QFile f(outputFile);
    if (!f.exists() || context->no_overwrite == 0) {
        f.close();
        convert_to_j2k(context,inputFile.toAscii().data(),outputFile.toAscii().data(), NULL);
    }

    if (context->stereoscopic) {
        fileinfo = inRightList.at(iteration);
        inputFile = fileinfo.absoluteFilePath();
        baseName = fileinfo.completeBaseName();

        outputFile.sprintf("%s/%s.%s",outRightDir.toStdString().c_str(),baseName.toStdString().c_str(),"j2c");
        QFile f(outputFile);
        if (!f.exists() || context->no_overwrite == 0) {
            f.close();
            status = convert_to_j2k(context,inputFile.toAscii().data(),outputFile.toAscii().data(), NULL);
        }
    }
}

void MainWindow::preview() {
    QString filter = "*.tif;*.tiff;*.dpx";
    QDir inLeftDir;
    QFileInfo file;
    QImage image;

    inLeftDir.cd(ui->inImageLeftEdit->text());
    inLeftDir.setFilter(QDir::Files | QDir::NoSymLinks);
    inLeftDir.setNameFilters(filter.split(';'));
    inLeftDir.setSorting(QDir::Name);
    file = inLeftDir.entryList().at(0);
    ui->outJ2kLeftEdit->setText(file.absoluteFilePath());
    QPixmap pixmap(file.absoluteFilePath());
    ui->previewLabel->setPixmap(pixmap);
}

void MainWindow::showImage(QImage image) {
    ui->previewLabel->setPixmap(QPixmap::fromImage(image));
    ui->previewLabel->setEnabled(1);
}

void MainWindow::j2kConvert() {
    iterations = ui->endSpinBox->value() - ui->startSpinBox->value() + 1;
    int threadCount = 0;

    // set thread limit
    QThreadPool::globalInstance()->setMaxThreadCount(ui->threadsSpinBox->value());

    // Prepare the vector.
    QVector<int> vector;
    for (int i = ui->startSpinBox->value() - 1; i < ui->endSpinBox->value(); i++)
        vector.append(i);

    threadCount = QThreadPool::globalInstance()->maxThreadCount();
    dJ2kConversion->init(iterations, threadCount);

    // Create a QFutureWatcher and conncect signals and slots.
    QFutureWatcher<void> futureWatcher;
    QObject::connect(dJ2kConversion, SIGNAL(cancel()), &futureWatcher, SLOT(cancel()));
    QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), dJ2kConversion, SLOT(step()));
    QObject::connect(&futureWatcher, SIGNAL(finished()), dJ2kConversion, SLOT(finished()));

    // Start the computation
    futureWatcher.setFuture(QtConcurrent::map(vector, j2kEncode));

    // open conversion dialog box
    dJ2kConversion->exec();

    // wait to ensure all threads are finished
    futureWatcher.waitForFinished();

    return;
}

void MainWindow::j2kUpdateEndSpinBox() {
    ui->startSpinBox->setMaximum(ui->endSpinBox->value());
}

void MainWindow::j2kCheckLeftInputFiles() {
    QString filter = "*.tif;*.tiff;*.dpx";
    QDir inLeftDir;

    inLeftDir.cd(ui->inImageLeftEdit->text());
    inLeftDir.setFilter(QDir::Files | QDir::NoSymLinks);
    inLeftDir.setNameFilters(filter.split(';'));
    inLeftDir.setSorting(QDir::Name);
    inLeftList = inLeftDir.entryInfoList();

    if (inLeftList.size() < 1) {
        QMessageBox::warning(this, tr("No image files found"),
                                   tr("No valid image files were found in the selected directory"));
        return;
    }

    if (checkFileSequence(inLeftDir.entryList()) != DCP_SUCCESS) {
       return;
    }

    ui->endSpinBox->setValue(inLeftList.size());
    ui->endSpinBox->setMaximum(inLeftList.size());
    ui->startSpinBox->setMaximum(ui->endSpinBox->value());
}

void MainWindow::j2kCheckRightInputFiles() {
    QString filter = "*.tif;*.tiff;*.dpx";
    QDir inRightDir;

    inRightDir.cd(ui->inImageRightEdit->text());
    inRightDir.setFilter(QDir::Files | QDir::NoSymLinks);
    inRightDir.setNameFilters(filter.split(';'));
    inRightDir.setSorting(QDir::Name);
    inRightList = inRightDir.entryInfoList();

    if (inRightList.size() < 1) {
        QMessageBox::warning(this, tr("No image files found"),
                                   tr("No valid image files were found in the selected directory"));
        return;
    }

    if (checkFileSequence(inRightDir.entryList()) != DCP_SUCCESS) {
       return;
    }

    ui->endSpinBox->setValue(inRightList.size());
    ui->startSpinBox->setMaximum(ui->endSpinBox->value());
}

void MainWindow::j2kStart() {
    QString filter = "*.tif;*.tiff;*.dpx";
    QDir inLeftDir;
    QDir inRightDir;

    context = create_opendcp();

    // process options
    context->log_level = 0;
    dcp_set_log_level(context->log_level);

    if (ui->profileComboBox->currentIndex() == 0) {
        context->cinema_profile = DCP_CINEMA2K;
    } else {
        context->cinema_profile = DCP_CINEMA4K;
    }

    if (ui->encoderComboBox->currentIndex() == 0) {
        context->j2k.encoder = J2K_OPENJPEG;
    } else {
        context->j2k.encoder = J2K_KAKADU;
    }

    context->j2k.lut = ui->colorComboBox->currentIndex();

    if (ui->dpxLogCheckBox->checkState()) {
        context->j2k.dpx = 1;
    } else {
        context->j2k.dpx = 0;
    }

    if (ui->stereoscopicCheckBox->checkState()) {
        context->stereoscopic = 1;
    } else {
        context->stereoscopic = 0;
    }

    if (ui->xyzCheckBox->checkState()) {
        context->j2k.xyz = 1;
    } else {
        context->j2k.xyz = 0;
    }

    if (ui->overwritej2kCB->checkState())
        context->no_overwrite = 0;
    else {
        context->no_overwrite = 1;
    }

    context->frame_rate = ui->frameRateComboBox->currentText().toInt();
    context->bw = ui->bwSlider->value() * 1000000;

    // validate destination directories
    if (ui->stereoscopicCheckBox->checkState() == 0) {
        if (ui->inImageLeftEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Source Directory Needed"),tr("Please select a source directory"));
            goto Done;
        } else if (ui->outJ2kLeftEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Destination Directory Needed"),tr("Please select a destination directory"));
            goto Done;
        }
    } else {
        if (ui->inImageLeftEdit->text().isEmpty() || ui->inImageRightEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Source Directories Needed"),tr("Please select source directories"));
            goto Done;
        } else if (ui->outJ2kLeftEdit->text().isEmpty() || ui->outJ2kRightEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Destination Directories Needed"),tr("Please select destination directories"));
            goto Done;
        }
    }

    outLeftDir = ui->outJ2kLeftEdit->text();

    if (ui->stereoscopicCheckBox->checkState()) {
        inRightDir.cd(ui->inImageRightEdit->text());
        inRightDir.setFilter(QDir::Files | QDir::NoSymLinks);
        inRightDir.setNameFilters(filter.split(';'));
        inRightDir.setSorting(QDir::Name);
        inRightList = inRightDir.entryInfoList();
        outRightDir = ui->outJ2kRightEdit->text();
        if (inLeftList.size() != inRightList.size()) {
            QMessageBox::critical(this, tr("File Count Mismatch"),
                                 tr("The left and right image directories have different file counts. They must be the same. Please fix and try again."));
            goto Done;
        }
    }

    j2kConvert();

Done:
    delete_opendcp(context);
}
