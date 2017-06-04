#include "work.h"
#include "pub.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>
#include <iostream>
#include <sstream>
using namespace std;

work::work(int port) : sqlHelper()
{
    memset(socket_client, 0, sizeof(socket_client));
    listen_st = socket_create(port);//创建server端socket
    if (listen_st == 0)
        exit(-1);//创建socket失败，程序退出
}

work::~work()
{
    if (listen_st)
        close(listen_st);

}

void work::run()
{
    //初始化和连接数据库
    sqlInit("localhost", "root", "root","myqq");
    connect2db();


    struct epoll_event ev, events[CLIENTCOUNT];
    setnonblocking(listen_st);
    int epfd = epoll_create(CLIENTCOUNT);
    ev.data.fd = listen_st;
    //监控epoll消息
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;

    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st,&ev);

    int clist = 0;
    while (1) {
        //epoll_wait 阻塞， 一直到epoll中的socket有消息产生
        int nfds = epoll_wait(epfd, events, CLIENTCOUNT, -1);

        //第一次循环返回的是listen或出错
        if (nfds == -1)
        {
            perror("epoll_wait:");
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd < 0)
                continue;
            //listen socket 收到了来自客户端的连接
            if (events[i].data.fd == listen_st)
            {
                clist = socket_accept();
                if (clist >= 0)
                {
                    setnonblocking(clist);
                    ev.data.fd = clist;
                    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clist,&ev);
                    break;
                }
            }
            //有来自客户端的数据
            if (events[i].events & EPOLLIN) //socket收到数据
            {
                printf("有来自客户端的消息：\n");
                clist = events[i].data.fd;
                if (socket_recv(clist) <= 0)
                {
                    user_logout(clist);
                    events[i].data.fd = -1;
                }
            }
            if (events[i].events & EPOLLERR) //socket错误。
            {
                clist = events[i].data.fd;
                user_logout(clist);
                events[i].data.fd = -1;
            }

            if (events[i].events & EPOLLHUP) //socket被挂断。
            {
                clist = events[i].data.fd;
                user_logout(clist);
                events[i].data.fd = -1;
            }


        }
    }
    close(epfd);
}

void work::fix_socket_client(int index, int st)
{
    //同一个userid没有下线，却又在另一个终端登录，拒绝登录
    if (socket_client[index] != 0)
    {
        printf("%d:userid already login\n", index);
        struct msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.head[0] = 2; //系统消息
        msg.head[1] = 3; //userid已经登录
        msg.head[2] = 0; //暂时保留，填0
        msg.head[3] = 0; //暂时保留，填0
        //给client端socket下发系统消息
        send(st, (char *) &msg, sizeof(msg.head), 0);
        close(st);
    } else
    {
        //如果socket_client[index] 等于0，将client端socket赋予socket_client[index]
        socket_client[index] = st;
    }

}

//验证密码
int work::auth_passwd(int userid, const char *passwd)
{
    if (strncmp(passwd, "123456", 6) == 0)//如果密码等于"123456"验证通过
        return 1;
    else
        return 0;//密码验证失败
}

void work::broadcast_user_status()
{
    struct msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.head[0] = 1; //设置消息类型为用户状态消息

    for (int i = 0; i < CLIENTCOUNT; i++) //设置用户状态消息体
    {
        if (socket_client[i] != 0)
        {
            msg.body[i] = '1'; //设置相应userid状态为在线
        } else
        {
            msg.body[i] = '0'; //设置相应userid状态为离线
        }
    }

    //向socket_client数组中每个client广播用户状态消息
    for (int i = 0; i < CLIENTCOUNT; i++)
    {
        if (socket_client[i] != 0)
        {
            send(socket_client[i], &msg, strlen(msg.body) + sizeof(msg.head),
                    0);
            printf("%d:broadcast, %s\n", i, msg.body);
        }
    }

}

void work::sendmsg(const msg_t *msg, int msglen)
{
    if ((msg->head[2] < 0) || (msg->head[2] >= CLIENTCOUNT)) //没有这个目标用户
    {
        printf("%d:have not this userid\n", msg->head[2]);
    } else
    {
        if (socket_client[msg->head[2]] == 0) //目的user不在线
        {
            printf("%d:userid not online\n", msg->head[2]);
        } else
        {
            //给client端socket下发send消息
            send(socket_client[msg->head[2]], (const char *) msg, msglen, 0);
            printf("send message:%dto%d-%s\n", msg->head[1], msg->head[2],
                    msg->body);
        }
    }
}

void work::loginmsg(int st, int o_userid, const char *passwd)
/*            来自客户端的socket*， 用户ID，     密码*/
{

    struct msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.head[0] = 2; //系统消息
    msg.head[2] = 0; //暂时保留，填0
    msg.head[3] = 0; //暂时保留，填0

    //验证用户ID

    //将id转为字符串
    stringstream ss;
    ss << o_userid;
    string id_str;
    ss >> id_str;
    //构造sql语句
    string sqlQuery = "select userid, passwd from qquser where userid=" + id_str;
    //执行sql语句并保存结果
    mysqlhelper::MysqlData sqlData = sqlHelper.queryRecord(sqlQuery);

    if (sqlData.size() == 0)//无效的userid
    {
        printf("login failed, %d:invalid userid\n", o_userid);
        msg.head[1] = 1; //无效userid
        send(st, (const char *) &msg, sizeof(msg.head), 0);
        close(st);
        return;
    }
    if (strcmp((sqlData[0])["passwd"].c_str(), passwd))//验证用户登录UserId和密码
    {
        printf("login failed, userid=%d,passwd=%s:invalid password\n",
               o_userid,
               passwd);
        msg.head[1] = 2; //无效密码
        //给client端socket下发系统消息
        send(st, (const char *) &msg, sizeof(msg.head), 0);
        close(st);//验证失败，关闭client socket，函数返回
        return;
    }

    printf("%d:login success\n", o_userid);
    //将登录密码验证通过的客户端client安装到socket_client[]的数组中
    fix_socket_client(o_userid, st);
    broadcast_user_status(); //向socket_client数组中所有socket广播用户状态消息
}

void work::user_logout(int st)
{
    for (int i = 0; i < CLIENTCOUNT; i++)//循环遍历socket_client[]数组
    {
        //找到socket_client[]数组 中与相等的client socket
        if (socket_client[i] == st)
        {
            printf("userid=%d,socket disconn\n", i);
            close (socket_client[i]);//关闭socket_client[]数组中相应的socket
            //将数组socket_client[]中相应的元素初始化为0，以便该userid下次还可以继续登录
            socket_client[i] = 0;
            broadcast_user_status(); //向socket_client数组中所有socket广播用户状态消息
            return;
        }
    }

}

int work::socket_recv(int st)

{
    struct msg_t msg;
    memset(&msg, 0, sizeof(msg));
    //接收来自client socket发送来的消息
    int rc = recv(st, (char *) &msg, sizeof(msg), 0);
    printf("msg.head[0] = %d\n", msg.head[0]);
    printf("msg.body = %s\n",msg.body);
    if (rc <= 0)//接收失败
    {
        if (rc < 0)
            printf("recv failed %s\n", strerror(errno));
    } else//接收成功
    {
        switch (msg.head[0])
        {
        case 0: //send消息
            sendmsg(&msg, rc);
            break;
        case 1: //login消息
            loginmsg(st, msg.head[1], msg.body);
            break;
        default: //无法识别的消息
            printf("login fail, it's not login message,%s\n",
                   (const char *) &msg);
            msg.head[0] = 2; //系统消息
            msg.head[1] = 0; //无法识别的消息
            msg.head[2] = 0; //暂时保留，填0
            msg.head[3] = 0; //暂时保留，填0
            //给client端socket下发系统消息
            send(st, (const char *) &msg, sizeof(msg.head), 0);
            return 0;
        }
    }
    return rc;
}

int work::socket_accept()
{

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));
    //接收到来自client的socket连接
    int client_st = accept(listen_st, (struct sockaddr *) &client_addr, &len);

    if (client_st < 0)
        printf("accept failed %s\n", strerror(errno));
    else
        printf("accept by %s\n", inet_ntoa(client_addr.sin_addr));
    return client_st;//返回来自client端的socket描述符

}

//初始化数据库连接
void work::sqlInit(const string &sHost, const string &sUser,
                   const string &sPasswd, const string &sDatabase, int port, int iFlag)
{
    sqlHelper.init(sHost, sUser, sPasswd, sDatabase, port, iFlag);
}

void work::connect2db()
{
    sqlHelper.connect();
    //设置字符集
    sqlHelper.setDefalutCharacterSet();
}














