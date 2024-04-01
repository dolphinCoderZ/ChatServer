#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();
    // 向指定通道发布消息
    bool publish(int channel, string msg);
    // 订阅指定通道的消息
    bool subscribe(int channel);
    // 取消订阅
    bool unsubscribe(int channel);
    // 独立线程中接收订阅通道的消息
    void observeChannelMsg();
    // 业务层设置上报消息的回调
    void initNotifyHanlder(function<void(int, string)> func);

private:
    // 相当于一个reids客户端，负责发布消息
    redisContext *publishContext;
    // 负责订阅消息
    redisContext *subscribeContext;
    // 保存业务层的通知回调
    function<void(int, string)> notifyMsgHandler;
};

#endif