#ifndef USER_MODEL_H
#define USER_MODEL_H

#include "user.hpp"

// ���ݿ��Ĳ����࣬�����������������ݿ�һ��
class UserModel
{
public:
    bool insert(User &user);
    User query(int id);

    // �����û�״̬��Ϣ
    bool updateState(User user);

    void resetState();

private:
};

#endif