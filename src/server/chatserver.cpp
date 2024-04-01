#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// ֻ�����������������Ȼ�󽻸�serviceȥ�����崦��
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : server_(loop, listenAddr, name),
      loop_(loop)
{
    // ע��ص�����
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
    // �ͻ��˶Ͽ�����
    if (!conn->connected())
    {
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// �ͻ��˷�����Ϣ
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // �����л�
    json js = json::parse(buf);

    // ͨ��js["msgid"]��ȡ��Ӧҵ���handler
    // ����ģ��ֻ�������л���ִ��ҵ����
    auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());

    // ҵ����
    msgHandler(conn, js, time);
}