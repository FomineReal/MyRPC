/**************************************************************************
 * File name: myrpcchannel.h
 * Description: MyrpcController类继承了protobuf的抽象类RpcController，重写了其中的SetFailed和ErrorText等函数
 *              主要作用是记录调用远程服务处于什么状态，过程中的出错信息
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/

#ifndef __MYRPCCONTROLLER_H__
#define __MYRPCCONTROLLER_H__

#include <google/protobuf/service.h>
#include <string>

class MyrpcController : public google::protobuf::RpcController
{
public:
    MyrpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    // 目前未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;//RPC方法执行过程中的状态
    std::string m_errText;//RPC方法执行过程中的错误信息
};

#endif // __MYRPCCONTROLLER_H__