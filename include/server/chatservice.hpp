#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

// �ص�����������
using MsgHandler = function<void(const TcpConnectionPtr &, json &, Timestamp)>;

// ֻ����ҵ�����߼���ʵ�ʷ������ݿ�Ĳ�������usermodel
class ChatService
{
public:
    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;
    static ChatService *getInstance();

    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void logon(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // ��ȡ��Ӧ�Ĵ��������Թ�server�����
    MsgHandler getHandler(int msgid);
    // ����ͻ����쳣�˳�
    void clientCloseException(const TcpConnectionPtr &conn);
    // �������˳���������Ϣ
    void reset();

    // Ԥ��redis�ϱ��ص�

    void handleRedisSubscribeMsg(int userid, string msg);

private:
    ChatService();

private:
    // �洢��Ϣ���ͺͶ�Ӧ��ҵ������
    unordered_map<int, MsgHandler> msgHandlerMap_;

    // �洢�û������ӣ�����ת����Ϣ
    unordered_map<int, TcpConnectionPtr> userConnMap_;
    // ���������û�����map����Ҫ�̰߳�ȫ
    mutex connMapMutex_;

    // ���ݲ�������󣬸������ݿ�ľ������
    UserModel userModel_;
    OfflineMsgModel offlineModel_;
    FriendModel friendModel_;
    GroupModel groupModel_;
    Redis redis_;
};

#endif