#include "myrpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "myrpcapplication.h"
#include "myrpccontroller.h"
#include "zookeeperutil.h"

// header_size + service_name method_name args_size +args
//所有通过stub代理对象调用的rpc方法，都走到了这里，统一做rpc方法的数据序列化和网络发送
//调用CallMethod以后会进入阻塞直到远程执行端执行完毕返回执行结果
void MyrpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    //获取参数的序列化字符串长度args_size
    std::string args_str;
    uint32_t args_size = 0;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        //序列化失败，输出日志
        controller->SetFailed("序列化失败");
        return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);
    std::string rpc_header_str;
    uint32_t header_size;
    if (rpcheader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        //序列化失败，输出日志
        return;
    }

    //组织发送编码
    std::string send_rpc_str;
    //固定了字符串的前4个字节，它的内容是header的长度，也就是rpc_header_str的长度
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str; //到时候取出的时候，并不是剩下的都是args_str，因为存在tcp粘包的问题，所以在header_str里面，还记录了args的长度

    //使用tcp网络编程，将上面打包好的send_rpc_str发送出去
    //因为是客户端，所以简单使用socket，不使用muduo库
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        //创建失败，输出日志
        controller->SetFailed("创建socket失败");
        return;
    }

    //读取配置文件
    // std::string ip = MprpcApplication::getInstance().getConfig().load("rpcserver_ip");
    // uint16_t port = atoi(MprpcApplication::getInstance().getConfig().load("rpcserver_port").c_str());

    //原来是从配置文件中读取远程rpc的地址，现在是向zookeeper发起查询，得到远程ip和port
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }
    //从数据里面找到:的位置，截取出前面的ip和后面的port
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + "address is invalid!");
        return;
    }

    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(1 + idx, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // ipv4
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        //连接错误，输出日志
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        //发送失败，输出日志
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        //接收失败，输出日志
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    std::string response_str(recv_buf, 0, recv_size);

    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        //反序列化失败，输出日志
        close(clientfd);
        return;
    }

    close(clientfd);
}