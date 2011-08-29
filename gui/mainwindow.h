#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <opendcp.h>
#include "dialogj2kconversion.h"

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
    void addCompleted(int iteration, QString file);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSignalMapper signalMapper;
    GenerateTitle *generateTitle;
    //DialogJ2kConversion *dJ2kConversion;
    QString lastDir;
    QFutureWatcher<void> futureWatcher;
    DialogJ2kConversion *dJ2kConversion;
};

#endif // MAINWINDOW_H
