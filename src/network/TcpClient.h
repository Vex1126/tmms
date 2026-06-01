#pragma once
#include "network/net/TcpConnection.h"
#include "network/base/SocketOpt.h"
namespace tmms{
    namespace network{
        enum {
            kTcpConStatuInit = 0,
            kTcpConStatuConnecting = 1,
            kTcpConStatuConnected = 2,
            kTcpConStatuDisConnected = 3
        };
        using ConnectCallBack = std::function<void (const TcpConnectionPtr & con,bool)>;
        class TcpClient:public TcpConnection{
            public:
                TcpClient(EventLoop * loop,const InetAddress &addr);
                virtual ~TcpClient();
                void SetConnectCallBack(const ConnectCallBack & cb);
                void SetConnectCallBack(ConnectCallBack && cb);
                void Connect();
                void ConnectInLoop();
                void UpdateConnectionStatu();
                void OnRead()override;
                void OnWrite()override;
                void OnClose()override;
                void Send(const char * buf,size_t len);
                void Send(std::list<BufferNodePtr>&list);
            private:
                bool CheckError();                
                InetAddress sever_addr_;
                int32_t statu_;
                ConnectCallBack connect_cb;
        };
    }
}