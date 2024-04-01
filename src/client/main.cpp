#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#include <ctime>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <atomic>
#include <semaphore.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;
// 控制主菜单程序
bool isMainMenuRunning = false;
// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};

// 显示登录用户的信息
void showCurrentUserData();
// 接收线程
void readTaskHandler(int clientfd);
string getCurrentTime();
// 聊天菜单
void mainMenu(int clientfd);

// 主线程只负责发送信息包，子线程负责接收处理
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./chatclient 127.0.0.1 9999" << endl;
        exit(-1);
    }
    // 解析ip和port
    char *ip = argv[1];
    uint16_t port = stoi(argv[2]);
    // 创建socket文件描述符
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error!" << endl;
        exit(-1);
    }
    // 填写server对端信息
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error!" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化信号量
    sem_init(&rwsem, 0, 0);
    // 启动接收子线程
    thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    for (;;)
    {
        cout << "====================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "====================" << endl;
        cout << "choice: ";
        int choice = 0;
        cin >> choice;
        // 读掉缓冲区残留的回车
        cin.get();

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid: ";
            cin >> id;
            cin.get();
            cout << "user password: ";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MES;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoginSuccess = false;
            // 主线程只负责发送，如果主线程和子线程都在接收，会发生recv阻塞现象
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (-1 == len)
            {
                cerr << "send login msg error!" << endl;
            }
            // 等待子线程处理登陆
            sem_wait(&rwsem);
            if (g_isLoginSuccess)
            {
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        }
        break;
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username: ";
            cin.getline(name, 50); // 可以加空格
            cout << "password: ";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send logon msg error!" << endl;
            }

            // 等待子线程处理注册
            sem_wait(&rwsem);
        }
        break;
        case 3:
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            // 如果输入非数字，需要处理缓冲区
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }

    return 0;
}

void showCurrentUserData()
{
    cout << "===============login user===============" << endl;
    cout << "current login user => id: " << g_currentUser.getId() << " name: " << g_currentUser.getName() << endl;

    cout << "---------------friend list---------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }

    cout << "---------------group list---------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;

            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "========================================" << endl;
}

void doLoginResponse(json &response)
{
    if (response["errno"].get<int>() != 0)
    {
        cerr << response["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else
    {
        // 设置当前用户信息
        g_currentUser.setId(response["id"].get<int>());
        g_currentUser.setName(response["name"]);

        // 当前用户的好友列表信息
        if (response.contains("friends"))
        {
            g_currentUserFriendList.clear();
            vector<string> vec = response["friends"];
            g_currentUserFriendList.reserve(10);

            for (string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"]);
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.emplace_back(user);
            }
        }

        // 当前用户的群组信息
        if (response.contains("groups"))
        {
            g_currentUserGroupList.clear();
            vector<string> grpVec = response["groups"];
            g_currentUserGroupList.reserve(10);
            for (string &grpstr : grpVec)
            {
                json grpjs = json::parse(grpstr);
                Group group;
                // 群信息
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);
                // 群成员信息
                vector<string> userVec = grpjs["users"];
                for (string &userStr : userVec)
                {
                    GroupUser user;
                    json js = json::parse(userStr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().emplace_back(user);
                }
                g_currentUserGroupList.emplace_back(group);
            }
        }
        showCurrentUserData();

        // 显示离线消息
        if (response.contains("offlinemsg"))
        {
            vector<string> offVec = response["offlinemsg"];

            for (string &str : offVec)
            {
                json js = json::parse(str);

                if (js["msgid"].get<int>() == ONE_CHAT_MSG)
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "group message[" << js["groupid"] << "]: " << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}

void doLogOnResponse(json &response)
{
    if (response["errno"].get<int>() != 0)
    {
        cerr << "name is alread exist, register error!" << endl;
    }
    else
    {
        cout << "register success, userid is " << response["id"] << ", don't forget it!" << endl;
    }
}

// 一直接收服务器发来的信息包
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }

        json js = json::parse(buffer);
        int msgType = js["msgid"].get<int>();
        if (msgType == ONE_CHAT_MSG)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        if (msgType == GROUP_CHAT_MSG)
        {
            cout << "group message[" << js["groupid"] << "]: " << js["time"].get<string>() << " [" << js["id"] << "] " << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (msgType == LOGIN_RES_ACK)
        {
            doLoginResponse(js);
            sem_post(&rwsem);
            continue;
        }
        if (msgType == REG_MSG_ACK)
        {
            doLogOnResponse(js);
            sem_post(&rwsem);
            continue;
        }
    }
}

void help(int fd = 0, string str = "");
void chat(int fd, string str);
void addfriend(int fd, string str);
void creategroup(int fd, string str);
void addgroup(int fd, string str);
void groupchat(int fd, string str);
void logout(int fd, string str);
// 客户端命令列表
unordered_map<string, string>
    commandMap = {
        // 显示支持命令
        {"help", "format: help"},
        // 一对一聊天
        {"chat", "format: chat:friendid:message"},
        // 添加好友
        {"addfriend", "format: addfriend:friendid"},
        // 创建群组
        {"creategroup", "format: creategroup:groupname:groupdesc"},
        // 加入群组
        {"addgroup", "format: addgroup:groupid"},
        // 群聊
        {"groupchat", "format: groupchat:groupid:message"},
        // 注销
        {"logout", "format: logout"}};

unordered_map<string, function<void(int, string)>>
    commandHandlerMap = {
        {"help", help},
        {"chat", chat},
        {"addfriend", addfriend},
        {"creategroup", creategroup},
        {"addgroup", addgroup},
        {"groupchat", groupchat},
        {"logout", logout}};

// 二级菜单，用户输入对应指令调用相应的操作
void mainMenu(int clientfd)
{
    help();
    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            // help和logout
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

void help(int fd, string str)
{
    cout << "show command list >>>" << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void chat(int fd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "chat command invalid" << endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(fd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

void addfriend(int fd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buf = js.dump();

    int len = send(fd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addfriend msg error -> " << buf << endl;
    }
}

void creategroup(int fd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string buf = js.dump();
    int len = send(fd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send creategroup msg error -> " << buf << endl;
    }
}

void addgroup(int fd, string str)
{
    int groupid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;

    string buf = js.dump();
    int len = send(fd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addgroup msg error -> " << buf << endl;
    }
}

void groupchat(int fd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "groupchat command invalid!" << endl;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buf = js.dump();
    int len = send(fd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send groupchat msg error -> " << buf << endl;
    }
}

void logout(int fd, string str)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buf = js.dump();

    int len = send(fd, buf.c_str(), strlen(buf.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send logout msg error -> " << buf << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

string getCurrentTime()
{
    auto t = chrono::system_clock::to_time_t(chrono::system_clock::now());

    struct tm *ptm = localtime(&t);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday, (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

    return string(date);
}
