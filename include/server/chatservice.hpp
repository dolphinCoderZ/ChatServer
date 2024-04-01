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

// 回调函数的类型
using MsgHandler = function<void(const TcpConnectionPtr &, json &, Timestamp)>;

// 只负责业务处理逻辑，实际访问数据库的操作交给usermodel
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

    // 获取对应的处理器，以供server层调用
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 服务器退出，重置信息
    void reset();

    // 预置redis上报回调

    void handleRedisSubscribeMsg(int userid, string msg);

private:
    ChatService();

private:
    // 存储消息类型和对应的业务处理方法
    unordered_map<int, MsgHandler> msgHandlerMap_;

    // 存储用户的连接，方便转发消息
    unordered_map<int, TcpConnectionPtr> userConnMap_;
    // 并发操作用户连接map，需要线程安全
    mutex connMapMutex_;

    // 数据操作类对象，负责数据库的具体操作
    UserModel userModel_;
    OfflineMsgModel offlineModel_;
    FriendModel friendModel_;
    GroupModel groupModel_;
    Redis redis_;
};

#endif