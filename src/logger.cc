#include "logger.h"
#include <time.h>
#include <iostream>

//这是一个线程安全的懒汉单例模式获取实例的方法，
Logger& Logger::getInstance()
{
    static Logger logger;//这一行语句是线程安全的，因为c++11中，static静态类对象在执行构造函数进行初始化的过程是线程安全的
    return logger;
}


Logger::Logger()//构造函数
{
    //启动专门的写日志线程，涉及磁盘I/O，从queue写入文件中
    std::thread writeLogTask([&]()
    {
        for(;;)
        {
            //获取日期
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name,"%d-%d-%d.log",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE *pf = fopen(file_name,"a+");//a+：无则创建，有则追加
            if (nullptr == pf)
            {
                std::cout <<"logger file: "<<file_name<<"open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.pop();
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                    nowtm->tm_hour, 
                    nowtm->tm_min, 
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach();
    
}
void Logger::setLogLevel(LogLevel level)
{
    m_loglevel = level;
}

//外部调用，把日志信息写入缓冲区当中
void Logger::Log(std::string msg)
{
    m_lckQue.push(msg);
}