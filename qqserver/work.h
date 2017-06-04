#ifndef WORK_H
#define WORK_H

#include "mysqlhelper.h"
#define CLIENTCOUNT 256
#define BODYBUF 1024

struct msg_t
{
    unsigned char   head[4];
    char            body[BODYBUF];
};

class work
{
public:
    work(int port);
    ~work();
    void run();

private:
//     //将accept的客户端连接安装到socket_client[10]的数组中
    void fix_socket_client(int index, int st);
    int auth_passwd(int userid, const char *passwd);
//    //向socket_client[]数组中所有client广播用户状态消息
    void broadcast_user_status();
    void sendmsg(const struct msg_t *msg, int msglen); //处理send消息
    void loginmsg(int st, int o_userid, const char *passwd); //处理login消息
    void user_logout(int st); //client socket连接断开
    int socket_recv(int st);
    int socket_accept();

    void sqlInit(const string& sHost, const string& sUser = "", const string& sPasswd = "",
                 const string& sDatabase = "", int port = 0, int iFlag = 0);
    void connect2db();

    mysqlhelper::MysqlHelper sqlHelper;

    int listen_st;
    //申明socket_client数组，管理client的socket连接
    int socket_client[CLIENTCOUNT];

};

#endif // WORK_H
