#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;

bool UserModel::insert(User &user)
{
    // ƴ��sql
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connectDB())
    {
        if (mysql.update(sql))
        {
            // �����ݿ��ȡ������idֵ��������id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    MySQL mysql;
    User user;
    if (mysql.connectDB())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row)
            {
                user.setId(stoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
            }
        }
    }
    return user;
}

bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connectDB())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = 'offline' where state = 'online'");

    MySQL mysql;
    if (mysql.connectDB())
    {
        mysql.update(sql);
    }
}
