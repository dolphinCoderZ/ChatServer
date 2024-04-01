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

// ��ʼ����Ϣ��Ӧ��ҵ����
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
        // �����ϱ���Ϣ�ص�
        redis_.initNotifyHanlder(std::bind(&ChatService::handleRedisSubscribeMsg, this, _1, _2));
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {
        // û����Ӧ�Ĵ������ͣ��ͷ��ش�ӡ������Ϣ��������������
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

// ��¼��ҵ�������̣����ݿ�ķ��ʷֲ㵽model
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // �����ݿ��ѯ����¼���û�
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = userModel_.query(id);

    // У������
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
                // ��¼�û���������Ϣ
                userConnMap_.insert({id, conn});
            }

            // ���ĸ���Ȥ��ͨ����Ҳ���ǵ�ǰ�û���id
            redis_.subscribe(id);

            // ��¼�ɹ����������ݿ��е��û�״̬
            user.setState("online");
            userModel_.updateState(user);

            // ��Ӧ��Ϣ
            json response;
            response["msgid"] = LOGIN_RES_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // ��ѯ�û��Ƿ���������Ϣ
            vector<string> OfflineMsgVec = offlineModel_.query(id);
            if (!OfflineMsgVec.empty())
            {
                response["offlinemsg"] = OfflineMsgVec;
                offlineModel_.remove(id);
            }

            // �����û��б�
            vector<User> userVec = friendModel_.query(id);
            if (!userVec.empty())
            {
                // ���Զ�������ת��json�ַ���
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

            // �����û�����Ⱥ��Ⱥ��Ա
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

// ע��ģ��
void ChatService::logon(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // �ӿͻ����õ��û���������
    string username = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(username);
    user.setPwd(pwd);

    // ��ӵ����ݿ�
    bool res = userModel_.insert(user);
    if (res)
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        // �����û�id
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

    // ȡ������
    redis_.unsubscribe(userid);

    User user(userid, "", "offline");
    userModel_.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        // �����û�����ӳ�����Ҫ�̰߳�ȫ
        lock_guard<mutex> lock(connMapMutex_);

        auto it = userConnMap_.begin();
        for (; it != userConnMap_.end(); it++)
        {
            if (it->second == conn)
            {
                // ɾ��������Ϣ
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    redis_.unsubscribe(user.getId());
    // ����״̬
    user.setState("offline");
    userModel_.updateState(user);
}

// ��Ե�����
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toId = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(connMapMutex_);
        auto it = userConnMap_.find(toId);
        if (it != userConnMap_.end())
        {
            // �Զ����ߣ�ת����Ϣ
            it->second->send(js.dump());
            return;
        }
    }

    // ���ܶԶ����ӵ�����һ�������������Ȳ�ѯ�Զ��û��Ƿ�����
    User user = userModel_.query(toId);
    if (user.getState() == "online")
    {
        redis_.publish(toId, js.dump());
        return;
    }

    // �Զ˲����ߣ��洢������Ϣ
    offlineModel_.insert(toId, js.dump());
}

// ��Ӻ���
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friendModel_.insert(userid, friendid);
}

// ����Ⱥ��
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

// ����Ⱥ��
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
}

// Ⱥ����
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

// redis�ϱ��ص�
void ChatService::handleRedisSubscribeMsg(int userid, string msg)
{
    lock_guard<mutex> lock(connMapMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // �����ڷ����ڼ䣬�Զ����ߣ�����Ҫ�洢������Ϣ
    offlineModel_.insert(userid, msg);
}
