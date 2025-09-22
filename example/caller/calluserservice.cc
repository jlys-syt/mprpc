#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

int main(int argc, char** argv)
{
    // 整个程序启动以后，需要初始化rpc框架，只需要初始化一次
    MprpcApplication::Init(argc, argv);

    // 调用远程的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // 请求方法
    fixbug::LoginRequest request;
    request.set_name("zhang shan");
    request.set_pwd("123456");
    // 响应方法
    fixbug::LoginResponse response;
    MprpcController controller;
    stub.Login(&controller, &request, &response, nullptr);

    // 一次rpc调用完成，读调用的方法
    if (0 == response.result().errcode()) {
        std::cout << "rpc login response:" << response.success() << " msg:" << response.result().errmsg() << std::endl;
    } else {
        std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
    }

    fixbug::RegisterRequest registerRequest;
    fixbug::RegisterResponse registerResponse;
    registerRequest.set_id(1);
    registerRequest.set_name("王五");
    registerRequest.set_pwd("23456");
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    } else {
        stub.Register(nullptr, &registerRequest, &registerResponse, nullptr);
        if (0 == registerResponse.result().errcode()) {
            std::cout << "rpc login response:" << registerResponse.success() << " msg:" << registerResponse.result().errmsg() << std::endl;
        } else {
            std::cout << "rpc login response error:" << registerResponse.result().errmsg() << std::endl;
        }
    }
    
    return 0;
}