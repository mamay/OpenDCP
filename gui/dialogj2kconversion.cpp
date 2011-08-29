
#include <QDir>
#include "dialogj2kconversion.h"

DialogJ2kConversion::DialogJ2kConversion(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint);

    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(buttonStop, SIGNAL(clicked()), this, SLOT(abort()));
}

void DialogJ2kConversion::setup(int imageCount, int threadCount)
{
    QString labelText;
    treeWidget->clear();

    currentCount = 0;
    totalCount = imageCount;

    progressBar->reset();
    progressBar->setMinimum(0);
    progressBar->setMaximum(totalCount);

    labelText.sprintf("Conversion using %d threads(s)",threadCount);
    lblThreadCount->setText(labelText);

    buttonClose->setEnabled(false);
    buttonStop->setEnabled(true);

    m_msg.clear();
}

void DialogJ2kConversion::setRange(int min, int max)
{
    progressBar->setMinimum(0);
    progressBar->setMaximum(max-1);
}

void DialogJ2kConversion::step(int iteration)
{
    QString labelText;
    //counter(conv_status);
    //conversionStatus(conv_status, fileName);
    currentCount = iteration;
    labelText.sprintf("Completed %d of %d",currentCount,totalCount);
    labelTotal->setText(labelText);
    progressBar->setValue(currentCount);

    conversionStatus(1, "Foo");

    if (currentCount == totalCount) {
        resetButtons();
    }
}

void DialogJ2kConversion::conversionStatus(int conv_status, QString fileName)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

    if (conv_status == 1)
        m_msg = "Converted";

    item->setText(0, fileName);
    item->setText(1, m_msg);
}

void DialogJ2kConversion::abort()
{
    resetButtons();
    emit cancel();
}

void DialogJ2kConversion::resetButtons()
{
    buttonClose->setEnabled(true);
    buttonStop->setEnabled(false);
}

void DialogJ2kConversion::setErrorMsg(QString err_status)
{
    m_msg = err_status;
}
