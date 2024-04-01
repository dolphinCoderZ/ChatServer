/*
muduo网络库提供了两个主要的类
TcpServer ： 用于编写服务器程序
TcpClient ： 用于编写客户端程序
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;

/*
1.组合TcpServer对象
2.创建EventLoop事件循环对象指针
3.明确TcpServer构造函数的参数
4.注册回调函数
5.设置服务器线程数量，muduo库自己分配IO线程和worker线程
*/
class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress listenAddr, const string &nameArg)
        : server_(loop, listenAddr, nameArg), loop_(loop)
    {
        // 给服务器注册用户连接的创建、断开回调
        server_.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

        // 给服务器注册用户读写事件回调
        server_.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置服务器的线程数
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
