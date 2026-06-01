#pragma once
#include"network/base/SocketOpt.h"
#include"network/base/NetWork.h"
#include"network/net/EventLoop.h"
#include"network/net/InetAddress.h"
#include"network/net/Event.h"
#include<functional>
namespace tmms{
    namespace network{
        using AcceptorCallBack = std::function<void(int sock,InetAddress &addr)>;
        class Acceptor:public Event{
            public:
                Acceptor(EventLoop * loop,const InetAddress &addr);
                ~Acceptor();
                void Start();
                void Stop();
                void SetAcceptorCallBack(const AcceptorCallBack &cb);
                void SetAcceptorCallBack(AcceptorCallBack &&cb);
                void OnRead()override;
                void OnError(const std::string &msg)override;
                void OnClose()override;
            private:
                void Open();
                InetAddress addr_;
                AcceptorCallBack accept_cb_;
                SocketOpt *socket_opt{nullptr};
        };
    }
}