#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr,
               const string &name);

    void start();

private:
    // 连接的回调函数
    void onConnection(const TcpConnectionPtr &conn);
    // 读写事件的回调函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);

    TcpServer server_; // 组合muduo库，实现服务器功能的类对象
    EventLoop *loop_;  // 指向事件循环的指针
};

#endif