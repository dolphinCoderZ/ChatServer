#include "groupmodel.hpp"
#include "db.h"
using namespace std;

// 创建群组
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

// 加入群组
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

// 查询用户所在的所有群信息
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
            // 查询用户所在的群
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

                // 根据用户所在的群，查询每个群的所有成员
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

// 查询指定群组的用户id列表，除了自己，主要用于群聊业务，给群组的其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);

    // 群发消息所有其他用户的id
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