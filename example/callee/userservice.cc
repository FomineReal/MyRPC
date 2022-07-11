#include <iostream>
#include <string>
#include "user.pb.h"
#include "myrpcapplication.h"
#include "myrpcprovider.h"
#include "logger.h"


class UserService : public myproto::UserServiceRpc //使用在rpc服务发布端
{
public:
    //本地业务Login
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : Login " << std::endl;
        std::cout << "name: " << name << "   pwd: " << pwd << std::endl;
    }
    //本地业务Register
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service : Register" << std::endl;
        std::cout << "id: " << id << "   name: " << name << "  pwd:" << pwd << std::endl;
        return true;
    }

    /*重写基类UserServiceRpc的虚函数，框架会直接调用
    1.caller  ：Login(LoginRequest)  -> moduo -> callee
    2.callee  ：Login(LoginRequest)  ->
    */
    // void Login_rpc(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
    void Login_rpc(::google::protobuf::RpcController *controller,
                   const ::myproto::LoginRequest *request,
                   ::myproto::LoginResponse *response,
                   ::google::protobuf::Closure *done)
    {
        //上报请求参数request，业务获取相应数据，做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool ret = Login(name, pwd); //本地做业务
        //把相应写入rpc
        myproto::ResultCode *rescode = response->mutable_result();
        rescode->set_errcode(0);
        rescode->set_errmsg("");
        response->set_success(ret);

        //执行回调操作，相应对象数据的序列化和网络发送
        done->Run();
    }

    //框架会调用
    void Register(::google::protobuf::RpcController* controller,
                       const ::myproto::RegisterRequest* request,
                       ::myproto::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        //调用本地的方法，需要参数，而参数就从request中取得
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);
        
        //执行完以后，执行Run，把返回的结果发送给调用者
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("hello my log system");
    MyrpcApplication::Init(argc, argv); //框架的初始化操作

    RpcProvider provider;                      //
    provider.NotifyService(new UserService()); // UserService对象发布到rpc节点上
    provider.Run();                            //启动rpc服务发布节点
    return 0;
}