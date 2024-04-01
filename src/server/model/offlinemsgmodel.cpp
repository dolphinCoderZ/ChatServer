#include "offlinemsgmodel.hpp"
#include "db.h"

void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values (%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connectDB())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);

    MySQL mysql;
    if (mysql.connectDB())
    {
        mysql.update(sql);
    }
}

vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    MySQL mysql;
    vector<string> vec;
    // 只预留空间，容器还是为空
    vec.reserve(10);
    if (mysql.connectDB())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.emplace_back(row[0]);
            }
        }
        mysql_free_result(res);
    }
    return vec;
}