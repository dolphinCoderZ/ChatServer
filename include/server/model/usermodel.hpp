#ifndef USER_MODEL_H
#define USER_MODEL_H

#include "user.hpp"

// 数据库表的操作类，方法命名尽量与数据库一致
class UserModel
{
public:
    bool insert(User &user);
    User query(int id);

    // 更新用户状态信息
    bool updateState(User user);

    void resetState();

private:
};

#endif