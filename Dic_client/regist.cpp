#include "regist.h"
#include "ui_regist.h"

#include <QTextCodec>
#include <qtimer.h>
#include <QMessageBox>
#include <string.h>
regist::regist(QWidget *parent, QTcpSocket *socket) :
    QMainWindow(parent),
    ui(new Ui::regist),
    m_tcp(socket)
{
    ui->setupUi(this);
}

regist::~regist()
{
    delete ui;
}


void regist::on_r_regist_pb_clicked()
{
    MainWindow::MSG msg;
    msg.type = MainWindow::R;

    // 将 QString 转换为 C 风格字符串并复制给 msg 对象
    strncpy(msg.name, ui->name->text().toUtf8().constData(), ui->name->text().size());
    msg.name[ui->name->text().size()] = '\0'; // 添加 null 结尾
    strncpy(msg.data, ui->password->text().toUtf8().constData(), ui->password->text().size());
    msg.data[ui->password->text().size()] = '\0'; // 添加 null 结尾

    // 将 MSG 对象转换为 QByteArray
    QByteArray byteArray(reinterpret_cast<const char*>(&msg), sizeof(MainWindow::MSG));

    // 将字节流发送给服务器
    if (m_tcp != nullptr && m_tcp->state() == QAbstractSocket::ConnectedState)
    {
       qint64 bytesWritten = m_tcp->write(byteArray);

       if (bytesWritten == -1)
       {
           qDebug() << "Error writing to socket: " << m_tcp->errorString();
           return;
       }

       if (!m_tcp->waitForBytesWritten())
       {
           qDebug() << "Failed to write data to socket: " << m_tcp->errorString();
           return;
       }

       qDebug() << "Register request sent to server.";
    }

    connect(m_tcp, &QTcpSocket::connected, this, [=](){
       qDebug() << "Connected to server";
    });

    // 延迟接受服务器返回的消息
    QTimer::singleShot(100, this, [=]() {
        QByteArray data = m_tcp->read(sizeof(MainWindow::MSG));
        if(data.size()>0)
        {
            MainWindow::MSG msg1;
            if (data.size() == sizeof(MainWindow::MSG)) {
                // 将字节流转换为 MSG 结构体
                memcpy(&msg1, data.data(), sizeof(MainWindow::MSG));
                qDebug() << strcmp(msg1.data,"OK!");

                if(strcmp(msg1.data,"OK!") == 0)
                {
                    QMessageBox::information(this,"提示","注册成功！");
                    emit regist_ok();//发出注册成功信号

                }
                else
                {
                    QMessageBox::information(this,"提示","注册失败！用户名已存在");
                }
                // 在这里对接收到的数据进行处理
                qDebug() << "Received name: " << msg1.name;
                qDebug() << "Received data: " << msg1.data;
            }
        }
        //qDebug() << data.size() << "  pass: " << sizeof(MainWindow::MSG);
    });

    //qDebug() << "name:" << msg.name << "  pass: " << msg.data;




}

void regist::on_pushButton_clicked()
{
    ui->name->clear();
    ui->password->clear();
    emit r_quit();
}
