#include "login.h"
#include "ui_login.h"

#include <QMessageBox>
#include <QTimer>

login::login(QWidget *parent, QTcpSocket *socket) :
    QMainWindow(parent),
    ui(new Ui::login),
    m_tcp(socket)
{
    ui->setupUi(this);
    my_reg = new regist(nullptr,m_tcp);
    //注册完毕回到登陆页面
    connect(my_reg,&regist::regist_ok,this,[=](){
        // 显示登录界面
        my_reg->hide();
        // 隐藏主界面
        this->show();
    });

    //勾选密码显示框
    ui->password->setEchoMode(QLineEdit::Password);//默认不显示
    connect(ui->seePass_cb, &QCheckBox::stateChanged, [this](int state){
        if (state == Qt::Checked) {
            ui->password->setEchoMode(QLineEdit::Normal); // 显示密码
        } else {
            ui->password->setEchoMode(QLineEdit::Password); // 隐藏密码
        }
    });
    //注册界面的返回按钮
    connect(my_reg,&regist::r_quit,this,[=](){
        // 显示登录界面
        my_reg->hide();
        // 隐藏主界面
        this->show();
    });

}

login::~login()
{
    delete ui;
}
//注册按钮
void login::on_regist_pb_clicked()
{
    // 显示注册界面
    my_reg->show();
    // 隐藏登陆界面
    this->hide();

}
//登陆按钮
void login::on_login_pb_clicked()
{
    MainWindow::MSG msg;
    msg.type = MainWindow::L;

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

                if(strcmp(msg1.data,"OK") == 0)
                {
                    //QMessageBox::information(this,"提示","登入成功！");
                    emit login_ok(msg1.name);//发送登入成功信号
                }
                else
                {
                    QMessageBox::information(this,"提示","登入失败！用户名或密码错误");
                }
                // 在这里对接收到的数据进行处理
                qDebug() << "Received name: " << msg1.name;
                qDebug() << "Received data: " << msg1.data;
            }
        }
        qDebug() << data.size() << "  pass: " << sizeof(MainWindow::MSG);
    });

}

void login::quit()
{
    ui->name->setText("");
    ui->password->setText("");
}
