/*
muduo������ṩ��������Ҫ����
TcpServer �� ���ڱ�д����������
TcpClient �� ���ڱ�д�ͻ��˳���
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;

/*
1.���TcpServer����
2.����EventLoop�¼�ѭ������ָ��
3.��ȷTcpServer���캯���Ĳ���
4.ע��ص�����
5.���÷������߳�������muduo���Լ�����IO�̺߳�worker�߳�
*/
class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress listenAddr, const string &nameArg)
        : server_(loop, listenAddr, nameArg), loop_(loop)
    {
        // ��������ע���û����ӵĴ������Ͽ��ص�
        server_.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

        // ��������ע���û���д�¼��ص�
        server_.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

        // ���÷��������߳���
        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state online" << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state offline" << endl;
            conn->shutdown();
        }
    }

    void onMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "time: " << time.toString() << " recv data: " << buf << endl;
        conn->send(buffer);
    }

private:
    muduo::net::TcpServer server_;
    muduo::net::EventLoop *loop_; // epoll
};

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 9999);

    ChatServer server(&loop, addr, "chat");

    server.start();

    loop.loop(); // epoll_wait

    return 0;
}
