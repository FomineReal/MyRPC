#include "myrpcprovider.h"
#include "myrpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"


//框架提供给外部使用，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    service_info.m_service = service;
    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();

    //获取服务的名字
    std::string service_name = pServiceDesc->name();

    //获取服务对象service方法的数量
    int methodCnt = pServiceDesc->method_count();

    LOG_INFO("service_name:%s",service_name.c_str());
    for (int i = 0; i < methodCnt; ++i)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述） UserService   Login
        const google::protobuf::MethodDescriptor *pmethodDesc = pServiceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s",method_name.c_str());
    }
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run()
{
    std::string ip = MyrpcApplication::getInstance().getConfig().load("rpcserver_ip");
    uint16_t port = atoi(MyrpcApplication::getInstance().getConfig().load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip, port);
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    //绑定连接回调和消息读写回调方法，分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4); // 1个I/O线程，3个工作线程，典型的基于Reactor的服务器


    ZkClient zkcli;
    zkcli.Start();
    //service_name为永久节点，method_name为临时节点
    for (auto &sp : m_serviceMap)
    {
        std::string service_path = "/" + sp.first;//serviceMap的first是service_name
        zkcli.Create(service_path.c_str(),nullptr,0);
        for(auto&mp : sp.second.m_methodMap)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            //ZOO_EPHEMERAL表示临时节点，原因：当该rpc节点断开时，不应再继续提供方法
            zkcli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }


    server.start();
    std::cout << "RpcProvider start service at ip:" << ip << "  port:" << port << std::endl;
    m_eventLoop.loop(); //相当于启动了epoll_wait()
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    //短连接
    if (!conn->connected()) //如果连接关闭了
    {
        conn->shutdown(); //相当于关闭文件描述符
    }
}

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
{
    // header_size(4个字节) + header_str + args_str
    //取出网络中的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字符流中读取前4个字节的内容,不是把数字当成字符
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    //根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //反序列化失败，记录日志
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        //请求了不存在的服务,记录日志
        return;
    }

    auto m_it = it->second.m_methodMap.find(method_name);
    if (m_it == it->second.m_methodMap.end())
    {
        //服务中不存在该方法，记录日志
        return;
    }

    google::protobuf::Service *service = it->second.m_service; //获取service对象
    const auto *method = m_it->second;                         //获取method对象

    //生成rpc方法调用的请求和response
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        //参数解析失败，记录日志
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给method方法调用绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &RpcProvider::SendRpcResponse,
                                                                                                 conn, response);

    //框架上根据远端rpc请求，框架来调用当前rpc节点上发布的方法
    //此处真正开始做业务
    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        //将序列化的返回信息通过网络发送给rpc的调用方
        conn->send(response_str);

    }
    else
    {
        //序列化失败，记录日志
    }
    conn->shutdown();//模拟http短连接服务，由rpcprovider主动断开连接
}