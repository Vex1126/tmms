#pragma once
#include"mmedia/rtmp/RtmpContext.h"
#include<memory>
#include<functional>
#include"mmedia/rtmp/RtmpHandler.h"
#include"network/net/EventLoop.h"
#include"network/TcpClient.h"
#include"network/net/InetAddress.h"

namespace tmms{
    namespace mm{
        using TcpClientPtr = std::shared_ptr<TcpClient>;
        class RtmpClient{
            public: 
                RtmpClient(EventLoop * loop,RtmpHandler *handler);
                ~RtmpClient();
                void OnWriteComplete(const TcpConnectionPtr & con);
                void OnConnection(const TcpConnectionPtr & con,bool connected);
                void OnMessage(const TcpConnectionPtr & con,MsgBuffer &msg);
                void SetCloseCallBack(const CloseConnectionCallBack & cb);
                void SetCloseCallBack(CloseConnectionCallBack && cb);
                void Play(const std::string &url);
                void Publish(const std::string &url);
                void CreateTcpClient();
                bool ParseUrl(const std::string &url);
            private:
                EventLoop * loop_{nullptr};
                InetAddress addr_;
                TcpClientPtr tcp_client_;
                RtmpHandler * handler_{nullptr};
                bool is_playing{false};
                CloseConnectionCallBack close_cb_;
                std::string url_;
        };
    }
}