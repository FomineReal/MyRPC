/**************************************************************************
 * File name: myrpcapplication.h
 * Description: mprpc框架的基础类，设计成单例，主要作用是从json配置文件中读取rpc服务器和zookeeper服务器的ip和port，
 *              并将这些信息存在成员变量config_中，可以随时根据字段名称查找到对应的ip或port
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/

#ifndef __MYRPCAPPLICATION_H__
#define __MYRPCAPPLICATION_H__

#include "myrpcconfig.h"
#include "myrpcchannel.h"
#include "myrpccontroller.h"

//mprpc框架的基础类，设计成单例
class MyrpcApplication
{
public:
    static void Init(int argc,char ** argv);
    static MyrpcApplication& getInstance();
    static MyrpcConfig& getConfig();

private:
    MyrpcApplication() = default;
    MyrpcApplication(const MyrpcApplication&) = delete;
    MyrpcApplication(MyrpcApplication&&) = delete;
    static MyrpcConfig config_;//静态的成员变量，必须在类外进行初始化

};








#endif // __MYRPCAPPLICATION_H__