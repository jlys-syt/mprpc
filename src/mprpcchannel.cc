#include "mprpcchannel.h"
#include "google/protobuf/descriptor.pb.h"
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include "mprpcapplication.h"
#include <unistd.h>
#include "user.pb.h"
#include "zookeeperutil.h"

/* 
header_size + (service_name + method_name + args_size) + args
*/
// 由stub代理对象调用的rpc方法，由CllMethod统一处理，进行数据的序列化和发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor* serviceDesc = method->service();
    std::string service_name = serviceDesc->name();
    std::string method_name = method->name();
    std::string args_str;
    if (!request->SerializeToString(&args_str)) {
        controller->SetFailed("request serialize error!");
        return;
    }
    uint32_t args_size = args_str.size();
    
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (!rpcheader.SerializeToString(&rpc_header_str)) {
        controller->SetFailed("serialize rpc header error!");
        return;
    }
    header_size = rpc_header_str.size();

    // 组织结构
    std::string send_rpc_str;
    send_rpc_str.insert(0, (char*)&header_size, 4);
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "--------------------------------" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "--------------------------------" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        char buf[512] = { 0 };
        sprintf(buf, "create socket error! errno:%d", errno);
        controller->SetFailed(buf);
        return;
    }


    // std::string ip = MprpcApplication::GetConfig().Load("rpcserverip");
    // uint32_t port = atoi(MprpcApplication::GetConfig().Load("rpcserverport").c_str());
    ZkClient zkcli;
    zkcli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkcli.GetData(method_path.c_str());
    
    if (host_data == "") {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }

    int idx = host_data.find(":");
    if (idx == -1) {
        controller->SetFailed(method_path + "address is invaild!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx - 1).c_str());
    std::cout << "---------ip:" << ip << " port:" << port << "----------" << std::endl;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(cfd, (sockaddr*)&server_addr, sizeof(server_addr))) {
        controller->SetFailed("connect error!");
        return;
    }
    if (-1 == send(cfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        controller->SetFailed("send error!");
        close(cfd);
        return;
    }

    char recv_buf[1024] = { 0 };
    int recv_size = 0;
    while ((recv_size = recv(cfd, recv_buf, sizeof(recv_buf), 0))> 0) {
        if (recv_size == 0) {
            break;
        } else if (recv_size > 0) {
            std::string response_str(recv_buf, 0, recv_size);
            if (response->ParseFromString(response_str) == -1) {
                
                controller->SetFailed("response parse error! response_str:" + response_str);
                break;
            }
        } else {
            controller->SetFailed("recv error! errno:" + std::to_string(errno));
            break;
        }
    }
    close(cfd);
}