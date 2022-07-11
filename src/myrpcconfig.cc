#include "myrpcconfig.h"
#include <iostream>
#include <fstream>
#include "json.hpp"
void MyrpcConfig::loadConfigFile(const char *config_file)
{
    std::ifstream ifs;
    ifs.open(config_file);
    if(!ifs.is_open())
    {
        std::cout<<config_file <<" is not exist."<<std::endl;
        exit(EXIT_FAILURE);
    }
    nlohmann::json j;
    ifs>>j;
    ifs.close();
    for(auto it = j.begin();it != j.end(); ++it)
    {
        m_config_Map[it.key()] = it.value();
    }
}

std::string MyrpcConfig::load(const std::string &key)
{
    auto it = m_config_Map.find(key);
    if(it == m_config_Map.end())
        return "";
    return it->second;
}