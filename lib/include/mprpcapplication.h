#pragma once

#include "mprpcconfig.h"

// mprpc框架的基础类，设计为单例模式，负责初始化操作
class MprpcApplication
{
public:
    static MprpcApplication& GetInstance();
    static void Init(int argc, char* argv[]);
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;
    MprpcApplication() {};
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(const MprpcApplication&&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;
};