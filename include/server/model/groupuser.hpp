#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <string>
#include "user.hpp"
using namespace std;

// Ⱥ���е��û��࣬������ʾȺ��ɫ��Ϣ
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role;
};

#endif