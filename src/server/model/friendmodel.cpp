#include "friendmodel.hpp"
#include "db.h"

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);

    MySQL mysql;
    if (mysql.connectDB())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select u.id, u.name, u.state from user u inner join friend f on f.friendid = u.id where f.userid = %d", userid);

    MySQL mysql;
    vector<User> vec;
    vec.reserve(10);
    if (mysql.connectDB())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                // 组装好友信息
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.emplace_back(user);
            }
        }
        mysql_free_result(res);
    }
    return vec;
}