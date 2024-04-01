#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>
using namespace std;
using namespace muduo;
using namespace placeholders;

ChatService *ChatService::getInstance()
{
    static ChatService service;
    return &service;
}

// 初始化消息对应的业务处理
ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MES, std::bind(&ChatService::login, this, _1, _2, _3)});

    msgHandlerMap_.insert({REG_MSG, std::bind(&ChatService::logon, this, _1, _2, _3)});

    msgHandlerMap_.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});

    msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});

    msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});

    msgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});

    msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    if (redis_.connect())
    {
        // 设置上报消息回调
        redis_.initNotifyHanlder(std::bind(&ChatService::handleRedisSubscribeMsg, this, _1, _2));
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {
        // 没有相应的处理类型，就返回打印错误信息的匿名函数对象
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid :" << msgid << " can't find handler";
        };
    }
    else
    {
        return msgHandlerMap_[msgid];
    }
}

void ChatService::reset()
{
    userModel_.resetState();
}

// 登录的业务处理流程，数据库的访问分层到model
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 从数据库查询待登录的用户
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = userModel_.query(id);

    // 校验密码
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_RES_ACK;
            response["errno"] = 2;
            response["errmsg"] = "already online!";
            response["id"] = user.getId();
            response["name"] = user.getName();
            conn->send(response.dump());
        }
        else
        {
            {
                lock_guard<mutex> lock(connMapMutex_);
                // 记录用户的连接信息
                userConnMap_.insert({id, conn});
            }

            // 订阅感兴趣的通道，也就是当前用户的id
            redis_.subscribe(id);

            // 登录成功，更新数据库中的用户状态
            user.setState("online");
            userModel_.updateState(user);

            // 响应消息
            json response;
            response["msgid"] = LOGIN_RES_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询用户是否有离线消息
            vector<string> OfflineMsgVec = offlineModel_.query(id);
            if (!OfflineMsgVec.empty())
            {
                response["offlinemsg"] = OfflineMsgVec;
                offlineModel_.remove(id);
            }

            // 返回用户列表
            vector<User> userVec = friendModel_.query(id);
            if (!userVec.empty())
            {
                // 将自定义类型转成json字符串
                vector<string> jsonVec;
                jsonVec.reserve(10);
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    jsonVec.emplace_back(js.dump());
                }
                response["friends"] = jsonVec;
            }

            // 返回用户所在群及群成员
            vector<Group> groupVec = groupModel_.queryGroup(id);
            if (!groupVec.empty())
            {
                vector<string> groupJson;
                groupJson.reserve(10);
                for (Group &group : groupVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();

                    vector<string> userVec;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userVec.emplace_back(js.dump());
                    }

                    grpjson["users"] = userVec;
                    groupJson.emplace_back(grpjson.dump());
                }
                response["groups"] = groupJson;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        json response;
        response["msgid"] = LOGIN_RES_ACK;
        response["errno"] = 1;
        response["errmsg"] = "error id or password!";
        conn->send(response.dump());
    }
}

// 注册模块
void ChatService::logon(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 从客户端拿到用户名和密码
    string username = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(username);
    user.setPwd(pwd);

    // 添加到数据库
    bool res = userModel_.insert(user);
    if (res)
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        // 返回用户id
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(connMapMutex_);
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
        {
            userConnMap_.erase(it);
        }
    }

    // 取消订阅
    redis_.unsubscribe(userid);

    User user(userid, "", "offline");
    userModel_.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        // 操作用户连接映射表需要线程安全
        lock_guard<mutex> lock(connMapMutex_);

        auto it = userConnMap_.begin();
        for (; it != userConnMap_.end(); it++)
        {
            if (it->second == conn)
            {
                // 删除连接信息
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    redis_.unsubscribe(user.getId());
    // 更新状态
    user.setState("offline");
    userModel_.updateState(user);
}

// 点对点聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toId = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(connMapMutex_);
        auto it = userConnMap_.find(toId);
        if (it != userConnMap_.end())
        {
            // 对端在线，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // 可能对端连接到了另一个服务器，需先查询对端用户是否在线
    User user = userModel_.query(toId);
    if (user.getState() == "online")
    {
        redis_.publish(toId, js.dump());
        return;
    }

    // 对端不在线，存储离线消息
    offlineModel_.insert(toId, js.dump());
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friendModel_.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group;
    group.setName(name);
    group.setDesc(desc);
    if (groupModel_.createGroup(group))
    {
        groupModel_.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
}

// 群聊天
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> toIdVec = groupModel_.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(connMapMutex_);
    for (int id : toIdVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else
            {
                offlineModel_.insert(id, js.dump());
            }
        }
    }
}

// redis上报回调
void ChatService::handleRedisSubscribeMsg(int userid, string msg)
{
    lock_guard<mutex> lock(connMapMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // 可能在发布期间，对端下线，还需要存储离线消息
    offlineModel_.insert(userid, msg);
}
