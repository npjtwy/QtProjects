#ifndef TOOLBOX1_H
#define TOOLBOX1_H
#include <QWidget>
#include <QToolButton>
#include "widget.h"
#include <QObject>
#include <QEvent>
#include <QTcpSocket>
#include <QToolBox>
#include <QAbstractSocket>


const int CLINETCOUNT  = 256;    //最大客户端数量
const int BODYBUF = 1024;
//消息结构
typedef struct msg_t
{
    unsigned char   head[4];
    char            body[BODYBUF];
} msg_t;


//myQQ主窗口类
class ToolBox1 : public QToolBox
{
    Q_OBJECT
public:
    explicit ToolBox1(QWidget *parent = 0);
    virtual ~ToolBox1(){}
    //向服务器发送消息
    void send_Msg(int d_userid, const char *msgBody);

    QString getThisUserName() const;

private:
    //创建256个button和widget
    QToolButton *toolBtn[CLINETCOUNT];
    Widget *child[CLINETCOUNT];
    QString userName[CLINETCOUNT];

    QTcpSocket *sockClient;

    int     userid;
    QString passwd;
    QString hostip;
    int     hostport;

    bool isLogin;


    void init_userName();
    void init_toolBtn();
    //向服务器发送登录消息
    void login_Msg();

    void system_Msg(int);
    void recv_send_Msg(int o_userid, const char *msgBody);
    void userStatus_Msg(const char * msgBody);//来自server的deliver消息


signals:
public slots:
    bool eventFilter(QObject *watched, QEvent *event);
    //socket处理相关槽函数
    void sock_Error(QAbstractSocket::SocketError sockErr);
    void read_Msg();
    void socket_connected();

    void closeEvent(QCloseEvent *event);
};

#endif // TOOLBOX1_H
