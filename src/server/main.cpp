#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

// 处理服务器强制退出，重置user状态信息
void resetHandle(int)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./chatclient 127.0.0.1 9999" << endl;
        exit(-1);
    }

    signal(SIGINT, resetHandle);

    EventLoop loop;
    InetAddress addr(argv[1], stoi(argv[2]));
    ChatServer server(&loop, addr, "chat");

    server.start();
    loop.loop();

    return 0;
}