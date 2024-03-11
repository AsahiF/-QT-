#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->addWidget(new QLabel("用户名："));
    username = new QLabel;
    ui->statusbar->addWidget(username);

    //连接到服务器
    connectToServer();
    // 创建登录界面
    my_log = new login(nullptr, m_tcp);

    // 延迟显示主界面
    QTimer::singleShot(200, this, [=]() {
        // 显示登录界面
        my_log->show();
        // 隐藏主界面
        this->hide();
    });

    //登入成功
    connect(my_log,&login::login_ok,this,[=](QString name){
        // 显示登录界面
        my_log->hide();
        // 隐藏主界面
        this->show();
        //设置状态栏用户名
        qDebug()<<name;
        username->setText(name);
    });
    //退出登陆输入框重置
    connect(this,&MainWindow::quit,my_log,&login::quit);
    //退出登陆
    connect(this,&MainWindow::quit,this,[=](){
        //重置各种输入框
        username->clear();
        ui->histroy_list->clear();
        ui->word->clear();
        ui->word_serched->clear();

        //connect(this,&MainWindow::quit,my_log,&login::quit); 第一次退出时信号和槽还没有建立连接，应当事先进行连接

        // 显示登录界面
        my_log->show();
        // 隐藏主界面
        this->hide();
    });




}

void MainWindow::connectToServer()
{
    // 创建并连接 QTcpSocket
    m_tcp = new QTcpSocket(this);

    connect(m_tcp, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,[=](QAbstractSocket::SocketError socketError){
       qDebug() << "Socket error:" << socketError;
    });

    // 连接到服务器
    m_tcp->connectToHost("192.168.137.128", 10000);

    //连接到服务器后QTcpSocket会发出connected信号
    connect(m_tcp, &QTcpSocket::connected, this, [=](){
       qDebug() << "Connected to server";
    });

}



MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_serch_pb_clicked()
{
    QString word = ui->word->text();
    QString name = username->text();

    MSG msg;
    msg.type = Q;

    // 将 QString 转换为 C 风格字符串并复制给 msg 对象
    strncpy(msg.name, name.toUtf8().constData(), username->text().size());
    msg.name[username->text().size()] = '\0'; // 添加 null 结尾
    strncpy(msg.data, word.toUtf8().constData(), ui->word->text().size());
    msg.data[ui->word->text().size()] = '\0'; // 添加 null 结尾

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
       //等待所有数据被写入套接字
       if (!m_tcp->waitForBytesWritten())
       {
           qDebug() << "Failed to write data to socket: " << m_tcp->errorString();
           return;
       }

       qDebug() << "Register request sent to server.";
    }


    // 延迟接受服务器返回的消息
    QTimer::singleShot(100, this, [=]() {
        QByteArray data = m_tcp->read(sizeof(MainWindow::MSG));
        if(data.size()>0)
        {
            MainWindow::MSG msg1;
            if (data.size() == sizeof(MainWindow::MSG)) {
                // 将字节流转换为 MSG 结构体
                memcpy(&msg1, data.data(), sizeof(MainWindow::MSG));

                if(strcmp(msg1.data,"Not found!\n") == 0)
                {
                    QMessageBox::information(this,"提示","没有查询到该单词");
                }
                else
                {
                    ui->word_serched->setText(msg1.data);
                }
                // 在这里对接收到的数据进行处理
                qDebug() << "Received name: " << msg1.name;
                qDebug() << "Received data: " << msg1.data;
            }
        }
    });
}

void MainWindow::on_exit_pb_clicked()
{
    emit quit();
}

void MainWindow::on_histroy_clicked()
{
    //QString word = ui->word->text();
    QString name = username->text();

    MSG msg;
    msg.type = H;

    // 将 QString 转换为 C 风格字符串并复制给 msg 对象
    strncpy(msg.name, name.toUtf8().constData(), username->text().size());
    msg.name[username->text().size()] = '\0'; // 添加 null 结尾
    //strncpy(msg.data, word.toUtf8().constData(), ui->word->text().size());
    //msg.data[ui->word->text().size()] = '\0'; // 添加 null 结尾

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

    // 延迟接受服务器返回的消息
    QTimer::singleShot(100, this, [=]()
    {
        QStringList records;
        while (m_tcp->bytesAvailable() > 0)
        {
           MSG msg;
           m_tcp->read(reinterpret_cast<char*>(&msg), sizeof(MSG));
            //qDebug() << msg.type;
           // 将接收到的历史记录存储在某种数据结构中
           // 这里假设你的数据结构是一个 QStringList
           QString record = QString::fromUtf8(msg.data);
           records.append(record);
         }

        // 将历史记录显示在 QTextEdit 上
        ui->histroy_list->clear();
        for (const QString &record : records)
        {
           ui->histroy_list->append(record);
        }
    });


}
