#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MES = 1, // ��¼��Ϣ
    LOGIN_RES_ACK, // ��¼��Ӧ��Ϣ
    LOGOUT_MSG,

    REG_MSG,        // ע����Ϣ
    REG_MSG_ACK,    // ע����Ӧ��Ϣ
    ONE_CHAT_MSG,   // һ��һ������Ϣ
    ADD_FRIEND_MSG, // ��Ӻ�����Ϣ

    CREATE_GROUP_MSG, // ����Ⱥ��
    ADD_GROUP_MSG,    // ����Ⱥ��
    GROUP_CHAT_MSG,   // Ⱥ����
};

#endif