/**************************************************************************
 * File name: zookeeperutil.h
 * Description: 主要是对ZooKeeper提供的API进行封装，主要功能：连接、创建节点和获取节点数据
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/


#ifndef __ZOOKEEPERUTIL_H__
#define __ZOOKEEPERUTIL_H__

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的zk客户端类，主要功能：连接、创建节点和获取节点数据
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int datalen, int state=0);
    // 根据参数指定的znode节点路径，或者znode节点的值
    std::string GetData(const char *path);
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};

#endif // __ZOOKEEPERUTIL_H__