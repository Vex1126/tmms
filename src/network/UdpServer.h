#pragma once
#include"network/net/UdpSocket.h"
namespace tmms{
    namespace network{
        class UdpServer:public UdpSocket{
            public:
                UdpServer(EventLoop * loop,const InetAddress & server_addr);
                virtual ~UdpServer();
                void Start();
                void Stop();
            private:
                void Open();
                InetAddress server_addr_;
        };
    }
}