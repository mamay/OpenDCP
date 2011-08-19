#ifndef GENERATETITLE_H
#define GENERATETITLE_H

#include <QDialog>

namespace Ui {
    class GenerateTitle;
}

class GenerateTitle : public QDialog
{
    Q_OBJECT

public slots:
    void updateTitle();

protected:
    void load(const QString& filename);

public:
    explicit GenerateTitle(QWidget *parent = 0);
    ~GenerateTitle();
    QString getTitle();

private:
    Ui::GenerateTitle *ui;
};

#endif // GENERATETITLE_H
