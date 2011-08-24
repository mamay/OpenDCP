#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QDir>
#include <QPixmap>
#include <stdio.h>
#include <opendcp.h>

void MainWindow::connectJ2kSlots()
{
    // connect slots
    connect(ui->stereoscopicCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setJ2kStereoscopicState()));
    connect(ui->qualitySlider,SIGNAL(valueChanged(int)),this, SLOT(qualitySliderUpdate()));
    connect(ui->encodeButton,SIGNAL(clicked()),this,SLOT(startJ2k()));
    connect(ui->profileComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(cinemaProfileUpdate()));

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
    connect(ui->inImageLeftEdit, SIGNAL(textChanged(QString)),this,SLOT(checkInputFiles()));
    connect(ui->endSpinBox, SIGNAL(valueChanged(int)),this,SLOT(updateEndSpinBox()));
}

void MainWindow::setJ2kStereoscopicState() {
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
    } else {
        ui->inImageLeft->setText(tr("Directory:"));
        ui->outJ2kLeft->setText(tr("Directory:"));
        ui->inImageRight->hide();
        ui->inImageRightEdit->hide();
        ui->inImageRightButton->hide();
        ui->outJ2kRight->hide();
        ui->outJ2kRightEdit->hide();
        ui->outJ2kRightButton->hide();
    }
}

void MainWindow::cinemaProfileUpdate() {
    if (ui->profileComboBox->currentIndex() == 0) {
#ifdef Q_WS_WIN
        ui->threadsSpinBox->setMaximum(6);
#endif
        ui->threadsSpinBox->setMaximum(QThreadPool::globalInstance()->maxThreadCount());
        ui->threadsSpinBox->setValue(QThread::idealThreadCount());
    } else {
#ifdef Q_WS_WIN
        ui->threadsSpinBox->setMaximum(2);
#endif
        ui->threadsSpinBox->setMaximum(QThreadPool::globalInstance()->maxThreadCount());
        ui->threadsSpinBox->setValue(QThread::idealThreadCount());
    }
}

void MainWindow::qualitySliderUpdate() {
    int bw = ui->qualitySlider->value();
    QString string;

    string.sprintf("%d mb/s",bw);
    ui->qualityValueLabel->setText(string);
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
    QImage image;
    char *cstrOutputFile;
    char *cstrInputFile;

    fileinfo = inLeftList.at(iteration);
    inputFile = fileinfo.absoluteFilePath();
    baseName = fileinfo.completeBaseName();

    outputFile.sprintf("%s/%s.%s",outLeftDir.toStdString().c_str(),baseName.toStdString().c_str(),"j2c");
    cstrOutputFile = new char [outputFile.toStdString().size()+1];
    cstrInputFile = new char [inputFile.toStdString().size()+1];
    strcpy(cstrOutputFile, outputFile.toStdString().c_str());
    strcpy(cstrInputFile, inputFile.toStdString().c_str());
    //image.load(inputFile);
    //MainWindow::showImage(image);
    QFile f(cstrOutputFile);
    if (!f.exists() || context->no_overwrite == 0) {
        f.close();
        convert_to_j2k(context,cstrInputFile,cstrOutputFile, NULL);
    }

    if (context->stereoscopic) {
        fileinfo = inRightList.at(iteration);
        inputFile = fileinfo.absoluteFilePath();
        baseName = fileinfo.completeBaseName();

        outputFile.sprintf("%s/%s.%s",outRightDir.toStdString().c_str(),baseName.toStdString().c_str(),"j2c");
        cstrOutputFile = new char [outputFile.toStdString().size()+1];
        cstrInputFile = new char [inputFile.toStdString().size()+1];
        strcpy(cstrOutputFile, outputFile.toStdString().c_str());
        strcpy(cstrInputFile, inputFile.toStdString().c_str());
        QFile f(cstrOutputFile);
        if (!f.exists() || context->no_overwrite == 0) {
            f.close();
            convert_to_j2k(context,cstrInputFile,cstrOutputFile, NULL);
        }
    }

    //qDebug() << "jpeg convert " << inputFile << "->" << outputFile << " in thread" << QThread::currentThreadId();
}

void MainWindow::showImage(QImage image) {
    ui->previewLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::convertJ2k() {
    iterations = ui->endSpinBox->value() - ui->startSpinBox->value() + 1;
    //iterations = inLeftList.size();

    // set thread limit in win32
    QThreadPool::globalInstance()->setMaxThreadCount(ui->threadsSpinBox->value());

    // Prepare the vector.
    QVector<int> vector;
    for (int i = ui->startSpinBox->value() - 1; i < ui->endSpinBox->value(); i++)
        vector.append(i);

    // Create a progress dialog.
    QProgressDialog dialog;
    dialog.setLabelText(QString("JPEG2000 conversion using %1 thread(s)...").arg(QThreadPool::globalInstance()->maxThreadCount()));

    // Create a QFutureWatcher and conncect signals and slots.
    QFutureWatcher<void> futureWatcher;
    QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    QObject::connect(&dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
    QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int,int)), &dialog, SLOT(setRange(int,int)));
    QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));

    // Start the computation.
    futureWatcher.setFuture(QtConcurrent::map(vector, j2kEncode));

    // Display the dialog and start the event loop.
    dialog.exec();

    if (futureWatcher.future().isCanceled()) {
        QMessageBox msgBox;
        msgBox.setText("JPEG2000 conversion cancelled");
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setText("JPEG2000 conversion complete");
        msgBox.exec();
    }

    // wait to ensure all threads are finished
    futureWatcher.waitForFinished();

    // Query the future to check if was canceled.
    //qDebug() << "Canceled?" << futureWatcher.future().isCanceled();

    return;
}

void MainWindow::updateEndSpinBox() {
    ui->startSpinBox->setMaximum(ui->endSpinBox->value());
}

void MainWindow::checkInputFiles() {
    QString filter = "*.tif;*.tiff;*.dpx";
    QDir inLeftDir;
    QDir inRightDir;

    inLeftDir.cd(ui->inImageLeftEdit->text());
    inLeftDir.setFilter(QDir::Files | QDir::NoSymLinks);
    inLeftDir.setNameFilters(filter.split(';'));
    inLeftDir.setSorting(QDir::Name);
    inLeftList = inLeftDir.entryInfoList();

    if (inLeftList.size() < 1) {
        QMessageBox::warning(this, tr("No TIF files found"),
                                   tr("No TIF files were found in the selected directory"));
        return;
    }

    ui->endSpinBox->setValue(inLeftList.size());
    ui->startSpinBox->setMaximum(ui->endSpinBox->value());
}

void MainWindow::startJ2k() {
    QString filter = "*.tif;*.tiff";
    QDir inLeftDir;
    QDir inRightDir;

    context = (opendcp_t *)malloc(sizeof(opendcp_t));
    memset(context,0,sizeof (opendcp_t));

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
        context->xyz = 1;
    } else {
        context->xyz = 0;
    }

    if (ui->overwritej2kCB->checkState())
        context->no_overwrite = 0;
    else {
        context->no_overwrite = 1;
    }

    context->frame_rate = ui->frameRateComboBox->currentText().toInt();
    context->bw = ui->qualitySlider->value() * 1000000;

    // validate destination directories
    if (ui->stereoscopicCheckBox->checkState() == 0) {
        if (ui->inImageLeftEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Source Directory Needed"),tr("Please select a source directory"));
            return;
        } else if (ui->outJ2kLeftEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Destination Directory Needed"),tr("Please select a destination directory"));
            return;
        }
    } else {
        if (ui->inImageLeftEdit->text().isEmpty() || ui->inImageRightEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Source Directories Needed"),tr("Please select source directories"));
            return;
        } else if (ui->outJ2kLeftEdit->text().isEmpty() || ui->outJ2kRightEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Destination Directories Needed"),tr("Please select destination directories"));
            return;
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
            QMessageBox::warning(this, tr("File Count Mismatch"),
                                 tr("The left and right image directories have different file counts. They must be the same. Please fix and try again."));
            return;
        }
    }

    convertJ2k();

    if (context != NULL) {
        free(context);
    }
}
