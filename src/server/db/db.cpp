#include "db.h"
#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "root";
static string password = "root";
static string dbname = "chat";

MySQL::MySQL()
{
    // 初始化连接
    conn_ = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (conn_)
    {
        mysql_close(conn_);
    }
}

bool MySQL::connectDB()
{
    // 创建连接
    MYSQL *p = mysql_real_connect(conn_, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p)
    {
        mysql_query(conn_, "set names gbk");
        LOG_INFO << "connectDB success!";
    }
    else
    {
        LOG_INFO << "connectDB failed!";
    }
    return p;
}

MYSQL *MySQL::getConnection()
{
    return conn_;
}

bool MySQL::update(string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __FILE__ ":" << sql << " update failed!";
        return false;
    }
    return true;
}

MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __FILE__ << ":" << sql << " query failed!";
        return nullptr;
    }
    return mysql_use_result(conn_);
}