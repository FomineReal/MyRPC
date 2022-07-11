/**************************************************************************
 * File name: myrpcchannel.h
 * Description: 框架提供给caller方的一个类，继承了protobuf的抽象类RpcChannel，重写了其中的CallMethod函数
 *              这是因为所有通过stub代理对象调用的rpc方法，都会调用CallMethod，因此必须重写该函数，
 *              CallMethod主要做的事情是：rpc方法的数据序列化和网络发送（简单使用socket发送，不使用muduo库）
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/


#ifndef __MYRPCCHANNEL_H__
#define __MYRPCCHANNEL_H__


#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

class MyrpcChannel : public google::protobuf::RpcChannel
{
public:

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
private:
    /* data */

};






#endif // __MYRPCCHANNEL_H__