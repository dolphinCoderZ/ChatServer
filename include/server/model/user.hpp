#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

// user表的ORM类，用来映射数据库表的字段
class User
{
public:
    User(int uId = -1, string uName = "", string pwd = "", string uState = "offline") : id(uId), name(uName), password(pwd), state(uState)
    {
    }

    void setId(int uId) { id = uId; }
    void setName(string uName) { name = uName; }
    void setPwd(string pwd) { password = pwd; }
    void setState(string uState) { state = uState; }

    int getId() { return id; }
    string getName() { return name; }
    string getPwd() { return password; }
    string getState() { return state; }

protected:
    int id;
    string name;
    string password;
    string state;
};

#endif