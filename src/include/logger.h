/**************************************************************************
 * File name: logger.h
 * Description: 单例模式的日志模块，从缓冲队列中取出数据向磁盘写入日志信息，
 *              类似生产者-消费者模式，多个工作线程生产日志信息，添加到缓冲队列中，磁盘I/O线程消费日志信息
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/

#ifndef __LOGGER_H__
#define __LOGGER_H__
#include "lockqueue.h"
#include <string>

enum LogLevel
{
    INFO,//普通日志信息
    ERROR//错误日志
};

class Logger//单例模式
{
public:
    static Logger& getInstance();//获取唯一实例
    void setLogLevel(LogLevel level);//设置日志级别
    void Log(std::string msg);
private:
    int m_loglevel;//记录日志的级别
    LockQueue<std::string> m_lckQue;//日志缓冲队列
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
};
#endif // __LOGGER_H__


// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")，方便用户使用
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0)

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::getInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0)

