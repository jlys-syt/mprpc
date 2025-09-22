#pragma once
#include "google/protobuf/service.h"

class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    // 由stub代理对象调用的rpc方法，由CllMethod统一处理，进行数据的序列化和发送
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);
};