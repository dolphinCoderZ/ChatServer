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

    // ����redis������
    bool connect();
    // ��ָ��ͨ��������Ϣ
    bool publish(int channel, string msg);
    // ����ָ��ͨ������Ϣ
    bool subscribe(int channel);
    // ȡ������
    bool unsubscribe(int channel);
    // �����߳��н��ն���ͨ������Ϣ
    void observeChannelMsg();
    // ҵ��������ϱ���Ϣ�Ļص�
    void initNotifyHanlder(function<void(int, string)> func);

private:
    // �൱��һ��reids�ͻ��ˣ����𷢲���Ϣ
    redisContext *publishContext;
    // ��������Ϣ
    redisContext *subscribeContext;
    // ����ҵ����֪ͨ�ص�
    function<void(int, string)> notifyMsgHandler;
};

#endif