#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

// Ⱥ��Ĳ����ӿ�
class GroupModel
{
public:
    // ����Ⱥ��
    bool createGroup(Group &group);
    // ����Ⱥ��
    void addGroup(int userid, int groupid, string role);
    // ��ѯ�û����ڵ�����Ⱥ��Ϣ(����Ⱥ��ĳ�Ա)
    vector<Group> queryGroup(int userid);
    // ��ѯָ��Ⱥ����û�id�б������Լ�����Ҫ����Ⱥ��ҵ�񣬸�Ⱥ���������ԱȺ����Ϣ
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif