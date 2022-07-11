/**************************************************************************
 * File name: lockqueue.h
 * Description: 为日志模块提供一个线程安全的队列，基于STL的queue，并使用互斥锁和条件变量来保证线程安全
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/

#ifndef __LOCKQUEUE_H__
#define __LOCKQUEUE_H__

#include <queue>//基于STL的queue改造
#include <thread>//多线程
#include <mutex>//线程互斥
#include <condition_variable>//线程同步

//异步写日志的、线程安全的队列
template<typename T>
class LockQueue
{
public:
    //多个线程都会进入到这里
    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
        m_cond_variable.notify_one();//
    }
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())//使用while防止虚假唤醒
        {
            //日志队列为空，则线程进入wait状态
            m_cond_variable.wait(lock);//会把锁释放掉
            //当收到notify_one()的通知以后，wait从等待变成阻塞，notify_one所在的push函数里的lock出作用域，锁自动释放，这时wait就能抢到锁
        }
        //此时pop又拿到锁了
        T data = m_queue.front();
        m_queue.pop();
        return data;
        
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond_variable;
};




#endif // __LOCKQUEUE_H__