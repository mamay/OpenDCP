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
#include "generatetitle.h"
#include "dialogj2kconversion.h"
#include "dialogmxfconversion.h"
#include "mxf-writer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    generateTitle   = new GenerateTitle(this);
    dJ2kConversion  = new DialogJ2kConversion();
    dMxfConversion  = new DialogMxfConversion();
    mxfWriterThread = new MxfWriter(this);

    setInitialUiState();
    connectSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSlots()
{
    j2kConnectSlots();
    mxfConnectSlots();
    connectXmlSlots();

    // set initial directory
    lastDir = QString::null;

    // connect tool button to line edits
    connect(&signalMapper, SIGNAL(mapped(QWidget*)),this, SLOT(getPath(QWidget*)));
}

void MainWindow::setInitialUiState()
{
    ui->mxfSoundRadio2->setChecked(1);

    // set initial screen indexes
    j2kSetStereoscopicState();
    mxfSetStereoscopicState();
    mxfSetSoundState();

    // Check For Kakadu
    QProcess *kdu;
    kdu = new QProcess(this);
    int exitCode = kdu->execute("kdu_compress", QStringList() << "-version");
    if (exitCode) {
        int value = ui->encoderComboBox->findText("Kakadu");
        ui->encoderComboBox->removeItem(value);
    }
    delete kdu;

    // Set thread count
#ifdef Q_WS_WIN
    ui->threadsSpinBox->setMaximum(6);
#endif
    ui->threadsSpinBox->setMaximum(QThreadPool::globalInstance()->maxThreadCount());
    ui->threadsSpinBox->setValue(QThread::idealThreadCount());

    ui->mxfTypeComboBox->setCurrentIndex(1);
    ui->tabWidget->setCurrentIndex(0);

    // File Menu
    ui->menuFile->addSeparator();
    ui->menuFile->addAction(tr("E&xit"), this, SLOT(close()),QKeySequence(tr("Ctrl+Q")));

    // Help Menu
    ui->menuHelp->addSeparator();
    ui->menuHelp->addAction(tr("About OpenDCP"), this, SLOT(about()));
}

void MainWindow::getPath(QWidget *w)
{
    QString path;
    QString filter;

    if (w->objectName().contains("picture") || w->objectName().contains("Image") || w->objectName().contains("J2k")) {
        if (w->objectName().contains("picture") && ui->mxfSourceTypeComboBox->currentIndex() == 1) {
            filter = "*.m2v";
            path = QFileDialog::getOpenFileName(this, tr("Choose an mpeg2 file"),lastDir);
        } else {
            path = QFileDialog::getExistingDirectory(this, tr("Choose a directory of images"),lastDir);
        }
    } else if (w->objectName().contains("subIn")) {
        filter = "*.xml";
        path = QFileDialog::getOpenFileName(this, tr("DCDM Subtitle File"),lastDir);
    } else if (w->objectName().contains("MxfOut")) {
        filter = "*.mxf";
        path = QFileDialog::getSaveFileName(this, tr("Save MXF as"),lastDir,filter);
    } else {
        filter = "*.wav";
        path = QFileDialog::getOpenFileName(this, tr("Choose a file to open"),lastDir,filter);
    }

    w->setProperty("text", path);
    lastDir = path;
}

int findSeqOffset(const char str1[], const char str2[]) {
    int i = 0;
    while(1) {
        if(str1[i] != str2[i])
                return i;
        if(str1[i] == '\0' || str2[i] == '\0')
                return 0;
        i++;
    }
}

/* check if filelist is sequential */
int MainWindow::checkFileSequence(QStringList list) {
    int sequential = 1;
    int x = 0;

    for (x = 0; x < (list.size()-1) && sequential; x++) {
        sequential = checkSequential(list.at(x).toLocal8Bit().constData(), list.at(x+1).toLocal8Bit().constData());
    }

    if (sequential) {
        return 0;
    } else {
        return x;
    }
}

/* check if two strings are sequential */
int MainWindow::checkSequential(const char *str1, const char *str2) {
    int x,y;
    int offset, len;

    offset = findSeqOffset(str2,str1);

    if (offset) {
        len = offset;
    } else {
        len = strlen(str1);
    }

    char *seq = (char *)malloc(len+1);

    strncpy(seq,str1+offset,len);
    x = atoi(seq);

    strncpy(seq,str2+offset,len);
    y = atoi(seq);

    if (seq) {
        free(seq);
    }

    if ((y - x) == 1) {
        return 1;
    } else {
        return 0;
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About OpenDCP"),
                       tr(
                          "OpenDCP Version 0.21\n\n"
                          "Copyright 2010-2011 Terrence Meiczinger. All rights reserved.\n\n"
                          "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                          "http://www.opendcp.org"
                         ));
}

