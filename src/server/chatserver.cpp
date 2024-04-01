#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 只负责处理网络请求包，然后交给service去做具体处理
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : server_(loop, listenAddr, name),
      loop_(loop)
{
    // 注册回调函数
    server_.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 客户端发来消息
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 反序列化
    json js = json::parse(buf);

    // 通过js["msgid"]获取相应业务的handler
    // 网络模块只负责反序列化后执行业务处理
    auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());

    // 业务处理
    msgHandler(conn, js, time);
}