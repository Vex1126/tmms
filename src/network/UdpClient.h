#pragma once
#include"network/net/UdpSocket.h"

namespace tmms{
    namespace network{
        using ConnectCallBack = std::function<void(const UdpSocketPtr &,bool)>;
        class UdpClient:public UdpSocket{
            public:
                UdpClient(EventLoop * loop,const InetAddress & server);
                ~UdpClient(); 
                void Connect();
                void ConnectInLoop();
                void SetConnectCallBack(const ConnectCallBack & cb);
                void SetConnectCallBack(ConnectCallBack && cb);
                void Send(const char * buf,size_t size);
                void Send(std::list<UdpBufferNodePtr>&list);  
                void OnClose() override;              
            private:
                bool connected_{false};
                InetAddress server_addr_;
                ConnectCallBack connect_cb_;
                struct sockaddr_in6 sock_addr_;
                socklen_t len = sizeof(sockaddr_in6);
        };
    }
}