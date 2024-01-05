#ifndef DIALOGSHOWMODULECHECKSUM_H
#define DIALOGSHOWMODULECHECKSUM_H

#include <QDialog>

namespace Ui {
class DialogShowModuleCheckSum;
}

class DialogShowModuleCheckSum : public QDialog
{
    Q_OBJECT

public:
    explicit DialogShowModuleCheckSum(const quint32 &result,const QByteArray &now,QWidget *parent = 0);
    ~DialogShowModuleCheckSum();

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogShowModuleCheckSum *ui;
};

#endif // DIALOGSHOWMODULECHECKSUM_H
