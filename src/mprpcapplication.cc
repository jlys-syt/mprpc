#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>

MprpcConfig MprpcApplication::m_config;

void ShowArgHelp()
{
    std::cout << "invaild args! Command -i <configfile>" << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

void MprpcApplication::Init(int argc, char* argv[])
{
    if (argc < 2) {
        ShowArgHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgHelp();
            exit(EXIT_FAILURE);
        case ':':
        std::cout << "nead <configfile>" << std::endl;
            ShowArgHelp();
            exit(EXIT_FAILURE);
        default:
            exit(EXIT_FAILURE);
        }
    }
    // 加载配置文件 rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
    m_config.LoadConfigFile(config_file.c_str());
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}