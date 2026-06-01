#pragma once
#include "network/TcpServer.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include "mmedia/rtmp/RtmpHandShake.h"
#include "mmedia/base/MMediaLog.h"
#include"mmedia/rtmp/RtmpContext.h"
namespace tmms{
    namespace mm{
        using namespace tmms::network;
        class RtmpServer:public TcpServer{
            public:
                RtmpServer(EventLoop * loop,const InetAddress & local,RtmpHandler *rtmphander = nullptr);
                ~RtmpServer();
                void Start()override;
                void Stop()override;
            private:
                void OnNewConnection(const TcpConnectionPtr & con);
                void OnDestroyed(const TcpConnectionPtr & con);
                void OnMessage(const TcpConnectionPtr & con,MsgBuffer & buf);
                void OnWriteComplete(const ConnectionPtr & con);     
                void OnActive(const ConnectionPtr & con);              
                RtmpHandler *rtmp_hander_ = nullptr;
        };
    }
}
