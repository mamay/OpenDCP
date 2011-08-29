#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "generatetitle.h"
#include "dialogj2kconversion.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    generateTitle = new GenerateTitle(this);
    dJ2kConversion = new DialogJ2kConversion();
    setInitialUiState();
    connectSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSlots()
{
    connectJ2kSlots();
    connectMxfSlots();
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
    setJ2kStereoscopicState();
    setMxfStereoscopicState();
    setMxfSoundState();

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
        path = QFileDialog::getSaveFileName(this, tr("Save MXF as"),QString::null,filter);
    } else {
        filter = "*.wav";
        path = QFileDialog::getOpenFileName(this, tr("Choose a file to open"),QString::null,filter);
    }

    w->setProperty("text", path);
    lastDir = path;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About OpenDCP"),
                       tr(
                          "OpenDCP Version 0.20\n\n"
                          "Copyright 2010-2011 Terrence Meiczinger. All rights reserved.\n\n"
                          "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                          "http://www.opendcp.org"
                         ));
}

