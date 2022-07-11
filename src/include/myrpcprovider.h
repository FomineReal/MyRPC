/**************************************************************************
 * File name: myrpcprovider.h
 * Description: 框架提供给callee方的一个类，对外含有两个接口：
 *              NotifyService：用户把想要发布的服务通过该接口向provider进行注册
 *              Run：启动rpc服务节点，开始对外提供rpc远程网络调用节点，执行后相当于会epoll_wait()，监听网络事件
 *              私有的一些事件的回调函数：
 *              OnConnection：已建立连接用户的读写事件的回调
 *              OnMessage：收到了远程发来的调用请求，解析出请求的服务和方法名称，通过server的CallMethod
 *              SendRpcResponse：Closure的回调操作，用于序列化rpc的相应和网络发送
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/


#ifndef __MYRPCPROVIDER_H__
#define __MYRPCPROVIDER_H__
#include "google/protobuf/service.h"
//#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

//框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    //依赖倒置原则，在框架中不可以用具体的类，而应该用抽象的类，才能解耦合
    //框架提供给外部使用，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    void Run();//启动rpc服务节点，开始提供rpc远程网络调用节点

private:


    //组合EventLoop，相当于epoll
    muduo::net::EventLoop m_eventLoop;

    //新Socket连接的回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);

    //已建立连接用户的读写事件的回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);

    //Closure的回调操作，用于序列化rpc的相应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务对象
        //保存服务对应的方法
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    //存储注册成功的服务对象名字和其服务方法的所有信息
    //都是抽象的信息，不能依赖具体的业务细节
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
};









#endif // __MYRPCPROVIDER_H__