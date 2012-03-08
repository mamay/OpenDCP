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

#ifndef J2KCONVERSIONDIALOG_H
#define J2KCONVERSIONDIALOG_H

#include "ui_conversion.h"

class J2kConversionDialog : public QDialog, private Ui::ConversionDialog {
    Q_OBJECT

public:
    J2kConversionDialog(QWidget *parent = 0);
    void init(int imageCount, int threadCount);

private:
    int totalCount;
    int currentCount;
    int cancelled;
    int done;

signals:
    void cancel();

public slots:
    void setButtons(int);
    void step();
    void finished();

private slots:
    void abort();
};

#endif // J2KCONVERSIONDIALOG_H
