#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "regist.h"
#include <QTcpSocket>
namespace Ui {
class login ;
}
// 前置声明 regist 类
class regist;
class login : public QMainWindow
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr, QTcpSocket *socket = nullptr);

    ~login();
public slots:
    void quit();
private slots:
    void on_regist_pb_clicked();

    void on_login_pb_clicked();

signals:
    void login_ok(QString name);

private:
    Ui::login *ui;
    QTcpSocket *m_tcp;
    regist *my_reg; // 使用前置声明的 regist 类型
};

#endif // LOGIN_H
