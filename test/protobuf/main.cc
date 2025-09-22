#include "test.pb.h"
#include <iostream>
using namespace fixbug;

// 方法测试案例

#if 0
int main()
{
    // 封装数据
    LoginRequest reg;
    reg.set_name("张三");
    reg.set_pwd("123456");
    
    // 对象数据序列化 =》 char*
    std::string send_str;
    if(reg.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    // 反序列化
    LoginRequest nreg;
    if(nreg.ParseFromString(send_str))
    {
        std::cout << nreg.name() << std::endl;
        std::cout << nreg.pwd() << std::endl;
    }
    return 0;
}
#endif

#if 0
int main()
{
    LoginResponse rsp;
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("错误");

    std::string msg;
    if (rsp.SerializePartialToString(&msg))
    {
        std::cout << msg << std::endl;
    }

    GetFriendListsResponse lrsp;
    User* user1 =lrsp.add_friend_list();
    user1->set_name("zhang shan");
    user1->set_sex(User::MAN);

    User* user2 =lrsp.add_friend_list();
    user2->set_name("li shi");
    user2->set_sex(User::WOMAN);

    std::cout << lrsp.friend_list_size() << std::endl;

    return 0;
}
#endif

int main()
{
    
}