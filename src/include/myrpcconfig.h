/**************************************************************************
 * File name: myrpcconfig.h
 * Description: 配置相关的类，提供从json文件中反序列化得到配置信息的接口，保存在成员变量的哈希表中，
 *              还提供查找配置信息的接口，可以根据查询字段从哈希表中查询信息返回给调用方
 * Version: 1.0
 * Author: Fuming Liu
 * Date: July 11, 2022
 **************************************************************************/

#ifndef __MYRPCCONFIG_H__
#define __MYRPCCONFIG_H__
#include <unordered_map>
#include <string>


class MyrpcConfig
{
public:
    void loadConfigFile(const char *config_file);
    std::string load(const std::string &key);

private:
    std::unordered_map<std::string,std::string> m_config_Map;
};


#endif // __MYRPCCONFIG_H__