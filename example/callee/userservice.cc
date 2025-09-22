#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    // 重写基类的rpc方法
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 执行本地服务
        bool login_result = Login(name, pwd);

        // 写入执行结果
        fixbug::ResultCode* resultcode = response->mutable_result();
        
        if (login_result) {
            resultcode->set_errmsg("登录成功");
        } else {
            resultcode->set_errcode(0);
            resultcode->set_errmsg("登录失败");
        }
        response->set_success(login_result);

        // 执行回调
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        std::cout << id << " " << name << " " << pwd << std::endl;
        fixbug::ResultCode* resultcode = response->mutable_result();
        resultcode->set_errcode(0);
        resultcode->set_errmsg("注册成功");
        response->set_success(1);
        done->Run();
    }
};

int main(int argc, char** argv)
{
    LOG_INFO("log message!");
    LOG_ERROR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // LOG_ERROR("%s:%");
    // 调用框架的初始化操作
    // MprpcApplication::Init(argc, argv);
    MprpcApplication::Init(argc, argv);

    // provider 是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点 Run以后阻塞等待远程的rpc调用请求
    provider.Run();
    return 0;
}