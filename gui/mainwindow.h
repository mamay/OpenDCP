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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <opendcp.h>
#include "dialogj2kconversion.h"
#include "dialogmxfconversion.h"
#include "mxf-writer.h"

class GenerateTitle;
class DialogJ2kConversion;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void setJ2kStereoscopicState();
    void setMxfStereoscopicState();
    void setMxfSoundState();
    void getPath(QWidget *w);
    void setPictureTrack();
    void setSoundTrack();
    void setSubtitleTrack();
    void startJ2k();
    void startMxf();
    void startDcp();
    void updatePictureDuration();
    void updateSoundDuration();
    void updateSubtitleDuration();
    void updateEndSpinBox();
    void checkInputFiles();
    void mxfSourceTypeUpdate();
    void qualitySliderUpdate();
    void cinemaProfileUpdate();
    void getTitle();
    void about();
    void createSubtitleMxf();
    void mxfDone();

protected:
    void setInitialUiState();
    void connectSlots();
    void connectJ2kSlots();
    void connectMxfSlots();
    void connectXmlSlots();
    void convertJ2k();
    void showImage(QImage image);
    void createPictureMxf();
    void createAudioMxf();
    int  checkFileSequence(QStringList list);
    int  checkSequential(const char str1[], const char str2[]); 

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow      *ui;
    QSignalMapper       signalMapper;
    GenerateTitle       *generateTitle;
    QString             lastDir;
    DialogJ2kConversion *dJ2kConversion;
    DialogMxfConversion *dMxfConversion;
    MxfWriter           *mxf;
};

#endif // MAINWINDOW_H
