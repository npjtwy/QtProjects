## myQQ 简介
一个低端仿QQ界面聊天软件，　属于自己练习用的，　技术含量不高，诸君莫笑

客户端使用Qt编写界面

服务端采用epoll模型，客户端登录以后和服务端保持一个长连接， 双发消息通过服务端送达

images.tar.gz 是头像资源文件　需解压使用

### 消息协议

```c
struct msg_t{
	unsigned char head[4]; //消息头， head[0]指明消息类型
	char body[1024];
};
```

### head[0]说明：

从client到server的消息有
- `head[0] = 0` ：send消息
- `head[0] = 1` ：login登录消息

从server到client的消息有

- `head[0] = 0` ：send消息
- `head[0] = 1` ：status用户状态消息
- `head[0] = 2` ：sys系统消息


### 消息发送流程

1. 客户ID为1端要登录服务端
- QT实现一个TCPSocet，连接到服务端所在的Ip和端口号

- 格式化消息
    ```c
    struct msg_t msg;
    msg.head[0] = 1;//代表login
    msg.head[1] = 1;
    msg.head[2] //暂时保留
    msg.head[3] //暂时保留
    msg.body：//登录密码(字符)
    ```

- 将msg通过TCPSOcket发送到服务端

2. 服务端收到来自客户端的登录消息

- 判断客户端的登录账号，密码是否合法如果合法

- 回复要登录的客户端格式化消息
    ```c
    struct msg_t msg;
    msg.head[0] = 2 //系统消息(整数)
    msg.head[1] //（整数）0无法识别的消息，1无效userid，2无效密码, 3userid已经登录,4其他
    msg.head[2] //暂时保留
    msg.head[3] //暂时保留
    msg.body[1024] //空，暂时保留
    ```

- 如果客户端验证通过，一切合法。通过TCPSocket将消息群发送给cleint

    ```c
    head[0] = 1 //用户状态消息（整数）
    head[1]     //暂时保留
    head[2]     //暂时保留
    head[3]     //暂时保留
    body[0]     //ID号为0的用户在线状态（字符），（'0'离线），（'1'在线）
    ```

4. 客户ID为1要给ID2发送消息
- 设置消息格式
    ```c
    struct msg_t msg;
    msg.head[0] = 0 //send消息（整数）
    msg.head[1]     //1消息源（源用户ID号）（整数）
    msg.head[2]     //2消息目的（目的用户ID号）（整数）
    head[3]         //暂时保留
    body[1024]:"message body"
    ```

- 客户端通过TCPsocket将msg发送到server

- server接收到客户ID为1的消息要把消息转发到ID2， 格式化消息

    ```c
    struct msg_t msg;
    msg.head[0] = 0 //send消息（整数）
    msg.head[1]     //1消息源（源用户ID号）（整数）
    msg.head[2]     //2消息目的（目的用户ID号）（整数）
    head[3]         //暂时保留
    body[1024]:"message body"
    ```

- 服务端端通过TCPsocket将msg发送到ID2对应的客户端
