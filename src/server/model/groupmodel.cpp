#include "groupmodel.hpp"
#include "db.h"
using namespace std;

// ����Ⱥ��
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connectDB())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// ����Ⱥ��
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.connectDB())
    {
        mysql.update(sql);
    }
}

// ��ѯ�û����ڵ�����Ⱥ��Ϣ
vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname,a.groupdesc from allgroup a inner join groupuser u on a.id = u.groupid where u.userid = %d", userid);

    vector<Group> groupVec;
    groupVec.reserve(10);

    MySQL mysql;
    if (mysql.connectDB())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            // ��ѯ�û����ڵ�Ⱥ
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.emplace_back(group);
            }
        }
        mysql_free_result(res);
    }

    for (Group &group : groupVec)
    {
        sprintf(sql, "select u.id,u.name,u.state,g.grouprole from user u inner join groupuser g on g.userid = u.id where g.groupid = %d", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {

                // �����û����ڵ�Ⱥ����ѯÿ��Ⱥ�����г�Ա
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]);
                groupUser.setRole(row[3]);
                group.getUsers().emplace_back(groupUser);
            }
        }
        mysql_free_result(res);
    }
    return groupVec;
}

// ��ѯָ��Ⱥ����û�id�б������Լ�����Ҫ����Ⱥ��ҵ�񣬸�Ⱥ���������ԱȺ����Ϣ
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);

    // Ⱥ����Ϣ���������û���id
    vector<int> toIdVec;
    toIdVec.reserve(20);

    MySQL mysql;
    if (mysql.connectDB())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                toIdVec.emplace_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return toIdVec;
}