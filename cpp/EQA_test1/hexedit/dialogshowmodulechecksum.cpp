#include "dialogshowmodulechecksum.h"
#include "ui_dialogshowmodulechecksum.h"
#include <QDebug>

DialogShowModuleCheckSum::DialogShowModuleCheckSum(const quint32 &result, const QByteArray &now, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogShowModuleCheckSum)
{
    ui->setupUi(this);
    this->setWindowModality(Qt::ApplicationModal);

    quint8 result_0 = result;
    quint8 result_1 = result>>8;
    quint8 result_2 = result>>16;
    quint8 result_3 = result>>24;

    ui->label->setText(QString("%1").arg(result_0,2,16));
    ui->label_2->setText(QString("%1").arg(result_1,2,16));
    ui->label_3->setText(QString("%1").arg(result_2,2,16));
    ui->label_4->setText(QString("%1").arg(result_3,2,16));

    quint8 now_0 = now.at(0);
    quint8 now_1 = now.at(1);
    quint8 now_2 = now.at(2);
    quint8 now_3 = now.at(3);

    ui->label_5->setText(QString("%1").arg(now_0,2,16));
    ui->label_6->setText(QString("%1").arg(now_1,2,16));
    ui->label_7->setText(QString("%1").arg(now_2,2,16));
    ui->label_8->setText(QString("%1").arg(now_3,2,16));

    if(ui->label->text()!=ui->label_5->text())
    {
        ui->label->setStyleSheet("QLabel{background-color:red}");
        ui->label_5->setStyleSheet("QLabel{background-color:red}");
    }
    else
    {
        ui->label->setStyleSheet("QLabel{background-color:green}");
        ui->label_5->setStyleSheet("QLabel{background-color:green}");
    }

    if(ui->label_2->text()!=ui->label_6->text())
    {
        ui->label_2->setStyleSheet("QLabel{background-color:red}");
        ui->label_6->setStyleSheet("QLabel{background-color:red}");
    }
    else
    {
        ui->label_2->setStyleSheet("QLabel{background-color:green}");
        ui->label_6->setStyleSheet("QLabel{background-color:green}");
    }

    if(ui->label_3->text()!=ui->label_7->text())
    {
        ui->label_3->setStyleSheet("QLabel{background-color:red}");
        ui->label_7->setStyleSheet("QLabel{background-color:red}");
    }
    else
    {
        ui->label_3->setStyleSheet("QLabel{background-color:green}");
        ui->label_7->setStyleSheet("QLabel{background-color:green}");
    }

    if(ui->label_4->text()!=ui->label_8->text())
    {
        ui->label_4->setStyleSheet("QLabel{background-color:red}");
        ui->label_8->setStyleSheet("QLabel{background-color:red}");
    }
    else
    {
        ui->label_4->setStyleSheet("QLabel{background-color:green}");
        ui->label_8->setStyleSheet("QLabel{background-color:green}");
    }
}

DialogShowModuleCheckSum::~DialogShowModuleCheckSum()
{
    delete ui;
}

void DialogShowModuleCheckSum::on_pushButton_clicked()
{
    this->close();
}
