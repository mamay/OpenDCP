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

#include <QtGui/QApplication>
#include "mainwindow.h"
#include "generatetitle.h"
#include <opendcp.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString langCode;

    qDebug() << QLocale().name();

    if  (QLocale().name().isEmpty()) {
        langCode = "en";
    } else {
        langCode = QLocale().name();
        langCode.truncate(langCode.lastIndexOf('_'));
    }

    langCode = "fr";

#ifdef Q_WS_MAC
    QFileInfo appFileInfo(QApplication::applicationDirPath());
    QString appPath = appFileInfo.absoluteDir().absolutePath();
    appPath = appPath + "/Resources";
#endif 

#ifdef Q_WS_WIN
    QFileInfo appFileInfo(QApplication::applicationDirPath());
    QString appPath = appFileInfo.absoluteDir().absolutePath();
    appPath.truncate(appPath.lastIndexOf('/'));
#endif

#ifdef Q_WS_X11
    QString appPath = "/usr/share/opendcp";
#endif

    QString fname = appPath + "/" + "translation/opendcp_" + langCode + ".qm";

    qDebug() << fname;

     QTranslator qtTranslator;
     qtTranslator.load("qt_" + QLocale::system().name(),
             QLibraryInfo::location(QLibraryInfo::TranslationsPath));
     a.installTranslator(&qtTranslator);

     QTranslator myTranslator;
     myTranslator.load(fname);
     //myappTranslator.load("/Users/tmeiczin/Development/OpenDCP/build/gui/translation/opendcp_" + QLocale::system().name());
     a.installTranslator(&myTranslator);

    MainWindow w;
    GenerateTitle g;
    w.show();

    return a.exec();
}
