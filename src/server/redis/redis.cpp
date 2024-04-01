#include "redis.hpp"
#include <iostream>
using namespace std;

Redis::Redis()
    : publishContext(nullptr), subscribeContext(nullptr)
{
}

Redis::~Redis()
{
    if (publishContext)
    {
        redisFree(publishContext);
    }
    if (subscribeContext)
    {
        redisFree(subscribeContext);
    }
}

// 连接redis服务器
bool Redis::connect()
{
    publishContext = redisConnect("127.0.0.1", 6379);
    if (publishContext == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    subscribeContext = redisConnect("127.0.0.1", 6379);
    if (publishContext == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    thread t([&]()
             { observeChannelMsg(); });
    t.detach();

    cout << "connect redis success!" << endl;
    return true;
}

// 向指定通道发布消息
bool Redis::publish(int channel, string msg)
{
    redisReply *reply = (redisReply *)redisCommand(publishContext, "PUBLISH %d %s", channel, msg.c_str());
    if (reply == nullptr)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 订阅指定通道的消息
bool Redis::subscribe(int channel)
{
    // 只负责订阅通道，不阻塞接收通道消息，交给独立线程去接收
    // 相当于只发送了命令，不需要reply
    if (REDIS_ERR == redisAppendCommand(subscribeContext, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subscribeContext, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }

    return true;
}

// 取消订阅
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(subscribeContext, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subscribeContext, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }

    return true;
}

// 独立线程中接收订阅通道的消息
void Redis::observeChannelMsg()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(subscribeContext, (void **)&reply))
    {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            notifyMsgHandler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
}

// 向业务层上报消息的回调
void Redis::initNotifyHanlder(function<void(int, string)> func)
{
    notifyMsgHandler = func;
}