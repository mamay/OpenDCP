

#include <QDir>
#include <QtGui>
#include "translator.h"

Translator::Translator()
{
#ifdef Q_OS_MAC
    m_langPath = qApp->applicationDirPath();
    m_langPath.truncate(m_langPath.lastIndexOf('/'));
    m_langPath.append("/Resources");
#endif

#ifdef Q_OS_WIN
    m_langPath = QApplication::applicationDirPath();
    m_langPath.truncate(m_langPath.lastIndexOf('/'));
#endif

#ifdef Q_OS_LINUX
    m_langPath = "/usr/share/opendcp";
#endif

    m_langPath.append("/translation");

    loadSettings();
}

QTranslator *Translator::Qtranslation()
{
    QTranslator *translator = new QTranslator;

    translator->load(m_langPath + "/" + m_langFile);

    return translator;
}


void Translator::saveSettings(int index)
{
    QSettings settings;

     QDir dir(m_langPath);
     QStringList fileList = dir.entryList(QStringList("opendcp_*.qm"));
 
     m_currLang = filenameToLanguage(fileList[index]);
     m_langFile = fileList[index];

    settings.setValue("Options/language",m_langFile);
}

void Translator::loadSettings()
{
    QSettings settings;

    m_currLang = "English";
    m_langFile = "opendcp_en.qm";

    if (settings.contains("Options/language")) {
        m_langFile = settings.value("Options/language").value<QString>();
        m_currLang = filenameToLanguage(m_langFile);
    }
}

QString Translator::filenameToLanguage(const QString &filename)
{
    QString locale;

    locale = filename;                          // "opendcp_xx.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "opendcp_xx"
    locale.remove(0, locale.indexOf('_') + 1);  // "xx"

    QString language = QLocale::languageToString(QLocale(locale).language());
 
    return(language);
}

QString Translator::filenameToLocale(const QString &filename)
{
    QString locale;

    locale = filename;                          // "opendcp_xx.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "opendcp_xx"
    locale.remove(0, locale.indexOf('_') + 1);  // "xx"

    return(locale);
}

QStringList Translator::loadLanguages(void)
{
    QStringList languageList;

    QDir dir(m_langPath);
    QStringList fileNames = dir.entryList(QStringList("opendcp_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        QString language = filenameToLanguage(fileNames[i]);

        languageList << language;
    }

    return languageList;
}

void Translator::setCurrentLanguage(const QString &language)
{
    m_currLang = language;
}

QString Translator::currentLanguage()
{
    return m_currLang;
}
