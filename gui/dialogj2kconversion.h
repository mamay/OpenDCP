#ifndef DIALOGJ2KCONVERSION_H
#define DIALOGJ2KCONVERSION_H

#include "ui_dialogj2kconversion.h"

class DialogJ2kConversion : public QDialog, private Ui::DialogJ2kConversion {
    Q_OBJECT

public:
    DialogJ2kConversion(QWidget *parent = 0);
    void setup(int imageCount, int threadCount);
    void conversionStatus(int conv_status, QString fileName);
    void setErrorMsg(QString err_status);

private:
    int totalCount;
    int currentCount;
    QString m_msg;

signals:
    void cancel();

public slots:
    void resetButtons();
    void setRange(int min, int max);
    void step(int iteration);
    //void step(int conv_status, QString fileName);

private slots:
    void abort();
};

#endif // DIALOGJ2KCONVERSION_H
