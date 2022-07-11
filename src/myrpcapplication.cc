#include "myrpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MyrpcConfig MyrpcApplication::config_;//静态变量在类外进行初始化

void showArgHelp()
{
    std::cout<<"Tips:"<<std::endl;
    std::cout<<"command -i <configfile>"<<std::endl;
}

void MyrpcApplication::Init(int argc,char ** argv)
{
    if (argc < 2)
    {
        showArgHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;//后面配置文件的路径将保存到这个变量中
    while((c = getopt(argc,argv,"i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            /* code */
            config_file = optarg;
            break;
        case '?':
            std::cout<<"Invalid args"<<std::endl;
            showArgHelp();
            exit(EXIT_FAILURE);
        case ':':
            showArgHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //加载配置文件 rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
    config_.loadConfigFile(config_file.c_str());
    //std::cout<<"rpcserver_port  "<<config_.load("rpcserver_port")<<std::endl;

}

MyrpcApplication &MyrpcApplication::getInstance()
{
    static MyrpcApplication app;
    return app;
}

MyrpcConfig& MyrpcApplication::getConfig()
{
    return config_;
}