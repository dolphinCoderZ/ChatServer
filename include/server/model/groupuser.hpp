#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <string>
#include "user.hpp"
using namespace std;

// 群组中的用户类，额外显示群角色信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role;
};

#endif