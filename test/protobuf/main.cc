#include "test.pb.h"
#include <iostream>
#include <string>

using namespace fixbug;

int main()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    std::string send_str;
    req.SerializeToString(&send_str);
    std::cout<<send_str<<std::endl;

    LoginRequest reqB;
    reqB.ParseFromString(send_str);
    std::cout<<reqB.name()<<std::endl;
    std::cout<<reqB.pwd()<<std::endl;

    GetFriendListsResponse rsp;
    ResultCode * rc = rsp.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("");

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);
}

