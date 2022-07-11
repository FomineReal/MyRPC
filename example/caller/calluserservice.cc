#include <iostream>
#include "myrpcapplication.h"//用户要使用我们开发的rpc框架，应包含该头文件
#include "user.pb.h"


int main(int argc,char **argv)
{
    
    //首先调用框架的初始化函数
    MyrpcApplication::Init(argc,argv);
    //没有默认构造，_Stub类中有一个channel_成员，因此必须给出channel
    myproto::UserServiceRpc_Stub stub(new MyrpcChannel);//要的参数是RpcChannel* channel，这里必须用继承RpcChannel的自己的Channel类，才能实现多态，调用到自己的CallMethod函数，而基类是抽象类类，不能实例化，
    myproto::LoginRequest request;
    request.set_name("zhangsanLogin");
    request.set_pwd("pwd");
    myproto::LoginResponse response;
    //第一个是controller，可以携带远程方法调用的过程中的一些信息，假如远程执行错误，就可以从controller中获取原因
    MyrpcController controller;//MprpcController是框架里面继承了protobuf的RpcController的，重写了一些方法
    stub.Login_rpc(&controller,&request,&response,nullptr);//同步rpc调用过程，会调用channel的CallMethod函数
    
    if (controller.Failed())
    {
        std::cout<<controller.ErrorText()<<std::endl;
        return 0;
    }
    //rpc调用完成，读取调用的结果
    if (0 == response.result().errcode())
    {
        std::cout<<"rpc login response success:" << response.success() << std::endl;
    }
    else
    {
        //调用失败
        std::cout<<"rpc login response error: " << response.result().errmsg() << std::endl;
    }
    return 0;
}


