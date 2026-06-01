#pragma once
#include"mmedia/base/MMediaHandler.h"
#include"mmedia/base/Packet.h"
namespace tmms{
    namespace mm{
        class RtmpHandler:public MMediaHandler{
            public:
                RtmpHandler(){}
                virtual ~RtmpHandler(){}
                virtual bool OnPlay(const TcpConnectionPtr &con,const std::string &session_mame,const std::string &parma){return false;}
                virtual bool OnPublish(const TcpConnectionPtr &con,const std::string &session_mame,const std::string &parma){return false;}
                virtual void OnPause(const TcpConnectionPtr &con,bool Pause){}
                virtual void OnSeek(const TcpConnectionPtr &con,bool time){}

                virtual void OnNewConnection(const TcpConnectionPtr &conn) override {}
                virtual void OnConnectionDestroy(const TcpConnectionPtr &conn) override {}
                virtual void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override {}
                virtual void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) override {}
                virtual void OnActive(const ConnectionPtr &conn) override {}
        };
    }
}