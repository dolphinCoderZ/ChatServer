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
    // ���ӵĻص�����
    void onConnection(const TcpConnectionPtr &conn);
    // ��д�¼��Ļص�����
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);

    TcpServer server_; // ���muduo�⣬ʵ�ַ��������ܵ������
    EventLoop *loop_;  // ָ���¼�ѭ����ָ��
};

#endif