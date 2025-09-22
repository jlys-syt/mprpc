#include "logger.h"
#include <time.h>
#include <iostream>

Logger::Logger()
{
    // 启动写日志线程
    std::thread writeLogTask([&](){
        for (;;) {
            // 获取当前时间，打开或创建当日日期日志文件，写入日志信息
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE* pf = fopen(file_name, "a+");
            if (pf == nullptr) {
                std::cout << "logerr file" << file_name << "open error!" << std::endl;
            }

            std::string msg = m_lckQue.Pop();
            char time_buf[128] = { 0 };
            sprintf(time_buf, "%d:%d:%d => ", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
            msg.insert(0,time_buf);
            msg.append("\r");
            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置分离线程
    writeLogTask.detach();
}

// 获取日志的单例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志
void Logger::Log(std::string msg)
{
    switch (m_loglevel)
    {
    case INFO:
        msg = "[INFO]:" + msg;
        break;
    
    case ERROR:
        msg = "[ERROR]:" + msg;
        break;

    default:
        msg = "[INFO]:" + msg;
        break;
    }
    m_lckQue.Push(msg);
}