#include "toolbox1.h"
#include <QPixmap>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHostAddress>
#include <QAbstractSocket>
#include "logindlg.h"
#include <string.h>

ToolBox1::ToolBox1(QWidget *parent) :
    QToolBox(parent)
{
    setWindowIcon(QPixmap(":/images/3.png"));

    loginDlg login;
    login.exec();   //弹出模式对话框

    setWindowTitle(userName[login.userid]);
    userid = login.userid;
    passwd = login.passwd;
    hostip = login.hostip;
    hostport = login.hostport;

    //根据登录对话框中用户输入的服务器IP和端口号连接到服务器
    sockClient = new QTcpSocket(this);
    //设置socket出错时的槽函数
    connect(sockClient, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(sock_Error(QAbstractSocket::SocketError)));

    //设置socket读消息的槽函数
    connect(sockClient, SIGNAL(readyRead()), this, SLOT(read_Msg()));

    //设置socket完成连接消息的槽函数
    connect(sockClient, SIGNAL(connected()), this, SLOT(socket_connected()));
    QHostAddress hostAddr(hostip);
    sockClient->connectToHost(hostAddr, hostport);

    init_userName();
    init_toolBtn();

}

void ToolBox1::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button =
            QMessageBox::question(this, tr("退出程序"),
                                  QString(tr("是否退出?")),
                                  QMessageBox::Yes | QMessageBox::No);

    if (button == QMessageBox::Yes)
    {
        event->accept();  //接受退出信号，程序退出
        if (sockClient->isOpen())
            sockClient->close();

        for(int i=0;i<CLINETCOUNT;i++)
        {
            delete child[i];
        }
    }
    else
    {
        event->ignore();  //忽略退出信号，程序继续运行
    }

}

bool ToolBox1::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)//如果是鼠标按键信号
    {
        int i = watched->objectName().toInt();
        child[i]->hide();
        child[i]->showNormal();//将toolbtn下对应的child显示到屏幕
    }
    return QToolBox::eventFilter(watched, event);//其他信号交给父类进行默认处理
}

void ToolBox1::sock_Error(QAbstractSocket::SocketError sockErr)
{
    switch (sockErr)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    default:
        QMessageBox::information(this, tr("错误"), sockClient->errorString());
    }
}

void ToolBox1::read_Msg()
{
    this->isLogin = true;
    struct msg_t msg;
    while(sockClient->bytesAvailable() > 0)
    {
        memset(&msg, 0, sizeof(msg));
        sockClient->read((char*)&msg, sizeof(msg));

        switch (msg.head[0]) {
        case 0://来自server的send消息
            recv_send_Msg(msg.head[1], msg.body);
            break;
        case 1://来自server的用户状态消息
            userStatus_Msg(msg.body);
            break;
        case 2://serve 系统消息
            system_Msg(msg.head[1]);
            break;
        default:
            break;
        }

    }

}

void ToolBox1::socket_connected()
{
    login_Msg();

}

//初始化所有用户btn
void ToolBox1::init_toolBtn()
{
    QString imagename;

    for (int i = 0; i < CLINETCOUNT; ++i)
    {
        toolBtn[i] = new QToolButton();
        toolBtn[i]->setObjectName(QString::number(i));
        //将点击事件由toolbox1完成
        toolBtn[i]->installEventFilter(this);
        toolBtn[i]->setText(userName[i]);
        imagename.sprintf(":/images/%d.png", i);
        //设置每个button的图像和图像大小
        toolBtn[i]->setIcon(QPixmap(imagename));
        toolBtn[i]->setIconSize(QPixmap(imagename).size());
        toolBtn[i]->setAutoRaise(true);
        toolBtn[i]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolBtn[i]->setToolTip(tr("未知"));
        child[i] = new Widget(toolBtn[i]->icon(), i, toolBtn[i]->text(), this);
    }
    QGroupBox *groupBox[8];
    QVBoxLayout *layout[8];
    static int index = 0;

    for (int i = 0; i < 8 ; ++i)
    {
        groupBox[i] = new QGroupBox;
        layout[i] = new QVBoxLayout(groupBox[i]);
        layout[i]->setMargin(10);
        layout[i]->setAlignment(Qt::AlignLeft);

        //每个layout里面有32个toolbutton
        for (int j = 0; j < 32; ++j)
        {
            layout[i]->addWidget(toolBtn[index++]);
        }

        //出入占位符
        layout[i]->addStretch();
    }
    addItem((QWidget *)groupBox[0], tr("好友"));
    addItem((QWidget *)groupBox[1], tr("陌生人"));
    addItem((QWidget *)groupBox[2], tr("家人"));
    addItem((QWidget *)groupBox[3], tr("同学"));
    addItem((QWidget *)groupBox[4], tr("同事"));
    addItem((QWidget *)groupBox[5], tr("网友"));
    addItem((QWidget *)groupBox[6], tr("驴友"));
    addItem((QWidget *)groupBox[7], tr("社区"));



}

void ToolBox1::login_Msg()
{
    const char* pw = passwd.toStdString().data();
    if (sockClient->isOpen())
    {
        if (sockClient->state() == QAbstractSocket::ConnectedState)
        {
            //新建消息
            msg_t msg;
            memset(static_cast<void*>(&msg), 0, sizeof(msg));

            //设置消息头为登录信息
            msg.head[0] = 1;    //表示消息类型为登录消息
            msg.head[1] = userid;
            msg.head[2] = 0;
            msg.head[3] = 0;

            //将消息体设置为密码
            strncpy(msg.body, pw, strlen(pw));
            //发送消息
            sockClient->write((const char*)(&msg), sizeof(msg.head) + strlen(msg.body));
        }
    }
}

void ToolBox1::system_Msg(int stat)
{
    switch (stat) {
    case 0:
        QMessageBox::information(this, tr("来自server的消息"), tr("无法识别的消息"));
        isLogin = false;
        break;
    case 1:
        QMessageBox::information(this, tr("来自server的消息"), tr("无效userid"));
        isLogin = false;
        break;
    case 2:
        QMessageBox::information(this, tr("来自server的消息"), tr("无效密码"));
        isLogin = false;
        break;
    case 3:
        QMessageBox::information(this, tr("来自server的消息"), tr("userid已经登录"));
        isLogin = false;
        break;
    case 4:
        QMessageBox::information(this, tr("来自server的消息"), tr("其他"));
        isLogin = false;
        break;
    default:
        QMessageBox::information(this, tr("来自server的消息"), tr("未知消息"));
        isLogin = false;
    }

}

void ToolBox1::init_userName()
{
    for (int i = 0; i < CLINETCOUNT; ++i)
    {
        userName[i] = tr("用户") + QString::number(i);
    }
}

//来自server的send消息
void ToolBox1::recv_send_Msg(int o_userid, const char *msgBody)
{

    child[o_userid]->add_msg(userName[o_userid], msgBody);
    child[o_userid]->show();
}

void ToolBox1::userStatus_Msg(const char *msgBody)//来自server的deliver消息
{
    for(int i=0;i<CLINETCOUNT;i++)
    {
        if(msgBody[i] == '1')
        {
            toolBtn[i]->setToolTip(tr("在线"));//设置toolBtn 鼠标提示为“在线”
            //设置child窗口中pushButton控件状态为可用
            child[i]->pushButton->setEnabled(true);
        }
        else
        {
            toolBtn[i]->setToolTip(tr("离线"));//设置toolBtn 鼠标提示为“离线”
            //设置child窗口中pushButton控件状态为不可用
            child[i]->pushButton->setEnabled(false);
        }
    }
}



void ToolBox1::send_Msg(int d_userid, const char *msgBody)//发往server的send消息
{
    if(sockClient->isOpen())
    {
        if (sockClient->state() == QAbstractSocket::ConnectedState)
        {
            struct msg_t msg;
            memset(&msg, 0, sizeof(msg));
            msg.head[0] = 0;//设置消息为send消息
            msg.head[1] = userid;//设置消息源用户ID
            msg.head[2] = d_userid;//设置消息目的用户ID
            msg.head[3] = 0; //暂时保留，填0
            strncpy(msg.body, msgBody, strlen(msgBody));//设置send消息内容
            sockClient->write((const char *)&msg, strlen(msg.body) + sizeof(msg.head));
        }
    }
}

QString ToolBox1::getThisUserName() const
{
    return userName[userid];
}
