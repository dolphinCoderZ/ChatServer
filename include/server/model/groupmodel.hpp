#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

// 群组的操作接口
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    void addGroup(int userid, int groupid, string role);
    // 查询用户所在的所有群信息(包括群里的成员)
    vector<Group> queryGroup(int userid);
    // 查询指定群组的用户id列表，除了自己，主要用于群聊业务，给群组的其他成员群发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif