#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QSettings>
#include <QTranslator>
#include <QMainWindow>
#include <QtGui>

class Translator : public QObject
{
    Q_OBJECT

public:
    Translator();

    QTranslator *Qtranslation();
    void        loadSettings(void);
    void        saveSettings(QString language);
    void        loadLanguages(void);
    QStringList languageList(void);
    QString     filenameToLanguage(const QString &filename);
    QString     currentLanguage();
    void        setCurrentLanguage(const QString &language);

protected:
    void loadLanguage(const QString& rLanguage);

private:
    
    void switchTranslator(QTranslator& translator, const QString& filename);

    QHash<QString, QString> m_langHash; /* language code hash */

    QTranslator     m_translator;       /* contains the translations for this application */
    QTranslator     m_translatorQt;     /* contains the translations for qt */
    QString         m_currLang;         /* contains the currently loaded language */
    QString         m_langPath;         /* path of language files. This is always fixed to /translation. */
    QString         m_langFile;         /* current language file */
};

#endif // TRANSLATOR_H
