#include "rpcprovider.h"
#include "mprpccontroller.h"
#include "logger.h"
#include "zookeeperutil.h"

void RpcProvider::NotifyService(google::protobuf::Service* service)
{
    ServiceInfo service_info;
    service_info.m_service = service;

    const google::protobuf::ServiceDescriptor * pserviceDesc = service->GetDescriptor();
    // 获取方法的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象方法的数量
    int methodCnt = pserviceDesc->method_count();

    LOG_INFO("server_name:%s", service_name.c_str());   
    for(int i = 0; i < methodCnt; ++i) {
        // 根据服务对象指定的下标获得方法描述
        const google::protobuf::MethodDescriptor* pmethodDesc =  pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run()
{
    // rpcservice 的ip和port信息
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port, false);

    // 创建TcpServer
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");
    // 绑定连接和信息读写回调
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置现muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client发现服务
    ZkClient zkCli;
    zkCli.Start();
    for (auto& sp : m_serviceMap) {
        // 创建服务节点为永久性
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto& mp : sp.second.m_methodMap) {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = { 0 };
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // 服务提供的方法节点为临时性
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    server.start();
    std::cout << "RpcProvider started server at ip:" << ip << " port:" << port << std::endl;
    m_eventloop.loop();
}

// muduo库的连接回调
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected()) {
        // 连接断开
        conn->shutdown();
    }
}

// 消息回调
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer* buf,
                        muduo::Timestamp time)
{
    // 网络上接收的远程请求的字符流
    std::string recv_buf = buf->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据head_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str)) {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else {
        // 数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << "parse error!" << std::endl;
        return;
    }

    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "--------------------------------" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "---------------=================" << std::endl;

    // 获取service对象和method对象

    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end()) {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end()) {
        std::cout << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service* service = it->second.m_service;
    const google::protobuf::MethodDescriptor* method = mit->second;

    // 生成rpc方法调用的请求request和响应的responst参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个回调 **参数推演失败，通过模板指定类型
    google::protobuf::Closure* done = google::protobuf::NewCallback<RpcProvider
                                                                    , const muduo::net::TcpConnectionPtr&
                                                                    , google::protobuf::Message*>(this
                                                                        , &RpcProvider::sendRpcResponse
                                                                        , conn, response);

    // 根据远程通信rpc请求，调用相应的方法
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr& conn,
                        google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        // 序列化成功，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
    } else {
        std::cout << "serialize response error!" << std::endl;
    }

    conn->shutdown(); // 模拟http的短连接服务，由rpcprovider主动断开连接
}