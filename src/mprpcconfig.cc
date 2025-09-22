#include "mprpcconfig.h"
#include <iostream>
#include <algorithm>

// 加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE* pf = fopen(config_file, "r");
    while (!feof(pf)) {
        char buf[512] = { 0 };
        fgets(buf, sizeof(buf), pf);

        std::string str(buf);
        Trim(str);

        // 一整行为空行或者注释
        if (str.empty() || str[0] == '#') {
            continue;
        }
        
        int idx = 0;
        // 配置项不合法
        idx = str.find('=', 0);
        if (idx == -1) {
            continue;
        }

        std::string key = str.substr(0, idx);
        Trim(key);
        int nidx = str.find('\n');
        std::string value;
        if (nidx != -1) {
            value = str.substr(idx + 1, nidx - idx - 1);
        } else {
            value = str.substr(idx + 1, str.length() - idx - 1);
        }
                      
        Trim(value);
        auto it = m_configMap.find(key);
        if(it == m_configMap.end()) {
            m_configMap.insert({key, value});
        }
    }
    fclose(pf);
}

// 查询配置信息
std::string MprpcConfig::Load(const std::string& key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        return "";
    }

    return it->second;
}

void MprpcConfig::Trim(std::string& str)
{
    // 去除行首空格, 尾换行
    int idx = str.find_first_not_of(' ');
    if (idx != -1) {
        str = str.substr(idx, str.length() - idx);
    }
    
    // 去除尾部空格
    idx = str.find_last_not_of(' ');
    if (idx != -1) {
        str = str.substr(0, idx + 1);
    }   
}