#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "login.h"
#include <QTcpSocket>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class login;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void connectToServer();

    ~MainWindow();
public:
    struct MSG
    {
        short type;
        char name[64]={};
        char data[256] ={};
    };

    enum
    {
        R = 1, // register
        L, // login
        Q, // query
        H //history
    };
private slots:
    void on_serch_pb_clicked();
    void on_exit_pb_clicked();
    void on_histroy_clicked();

signals:
    void quit();
private:
    Ui::MainWindow *ui;
    QTcpSocket *m_tcp;
    QLabel *username;
    login *my_log;
};
#endif // MAINWINDOW_H
