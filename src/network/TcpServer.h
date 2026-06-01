#pragma once
#include "network/net/Acceptor.h"
#include "network/net/TcpConnection.h"
#include "network/base/SocketOpt.h"
#include "network/net/Connection.h"
#include <unordered_set>
namespace tmms{
    namespace network{
        using NewConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        using DestroyConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        class TcpServer{
            public: 
                TcpServer(EventLoop * loop,const InetAddress & addr);
                virtual ~TcpServer();
                void SetNewConnectionCallBack(const NewConnectionCallBack &cb);
                void SetNewConnectionCallBack(NewConnectionCallBack &&cb);
                void SetDestroyConnectionCallBack(const DestroyConnectionCallBack &cb);
                void SetDestroyConnectionCallBack(DestroyConnectionCallBack &&cb);
                void OnConnectionClose(const TcpConnectionPtr &con);
                void SetActiveCallBack(const ActiveCallBack &cb);
                void SetActiveCallBack(ActiveCallBack &&cb);
                void SetMessageCallBack(const MessageCallBack &cb);
                void SetMessageCallBack(MessageCallBack &&cb);
                void SetWriteCompleteCallBack(const WriteCompleteCallBack &cb);
                void SetWriteCompleteCallBack(WriteCompleteCallBack &&cb);
                void OnAccet(int fd,const InetAddress & addr);
                virtual void Start();
                virtual void Stop();
            private:
                EventLoop * loop_{nullptr};
                InetAddress addr_;
                std::shared_ptr<Acceptor> acceptor_;
                NewConnectionCallBack new_connect_cb;
                DestroyConnectionCallBack destroy_connect_cb;
                std::unordered_set<TcpConnectionPtr> connections_;
                MessageCallBack message_cb;
                ActiveCallBack active_cb;
                WriteCompleteCallBack write_cp_cb;
        };
    }
}

