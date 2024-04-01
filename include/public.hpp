#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MES = 1, // 登录信息
    LOGIN_RES_ACK, // 登录响应信息
    LOGOUT_MSG,

    REG_MSG,        // 注册信息
    REG_MSG_ACK,    // 注册响应信息
    ONE_CHAT_MSG,   // 一对一聊天信息
    ADD_FRIEND_MSG, // 添加好友信息

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG,    // 加入群组
    GROUP_CHAT_MSG,   // 群聊天
};

#endif