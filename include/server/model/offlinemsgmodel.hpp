#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H

#include <vector>
#include <string>
using namespace std;

class OfflineMsgModel
{
public:
    // �洢�û���������Ϣ
    void insert(int userid, string msg);
    void remove(int userid);
    vector<string> query(int userid);
};

#endif