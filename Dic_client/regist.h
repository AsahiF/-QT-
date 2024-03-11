#ifndef REGIST_H
#define REGIST_H

#include <QMainWindow>
#include <QTcpSocket>
#include "mainwindow.h"
namespace Ui {
class regist;
}

class regist : public QMainWindow
{
    Q_OBJECT

public:
    explicit regist(QWidget *parent = nullptr, QTcpSocket *socket = nullptr);
    ~regist();

private slots:
    void on_r_regist_pb_clicked();
    void on_pushButton_clicked();

signals:
    void regist_ok();
    void r_quit();
private:
    Ui::regist *ui;
    QTcpSocket *m_tcp;
};

#endif // REGIST_H
