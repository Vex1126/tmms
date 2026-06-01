#pragma once
#include "network/base/MsgBuffer.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/Connection.h"
#include <functional>
#include <list>
#include <vector>
#include <sys/uio.h>
namespace tmms{
    namespace network{
        class TcpConnection;
        struct BufferNode;
        using BufferNodePtr = std::shared_ptr<BufferNode>;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using CloseConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        using MessageCallBack = std::function<void(const TcpConnectionPtr&,MsgBuffer&)>;
        using WriteCompleteCallBack = std::function<void(const TcpConnectionPtr&)>;
        using OutTimeCallBack = std::function<void(const TcpConnectionPtr&)>;
        struct TimeoutEntry{
            public:
                TimeoutEntry(const TcpConnectionPtr& c):conn(c){
                
                }
                ~TimeoutEntry();
                std::weak_ptr<TcpConnection> conn;
        };
        class TcpConnection:public Connection{    
            public:
                TcpConnection(EventLoop * loop,int fd,const InetAddress &local,const InetAddress &peer);
                ~TcpConnection();
                void SetCloseCallBack(const CloseConnectionCallBack & cb);
                void SetCloseCallBack(CloseConnectionCallBack && cb);
                void SetRecvMsgCallBack(const MessageCallBack & cb);
                void SetRecvMsgCallBack(MessageCallBack && cb);
                void SetWriteCompleteCallBack(const WriteCompleteCallBack & cb);
                void SetWriteCompleteCallBack(WriteCompleteCallBack && cb);
                void SetTimeOutCallBack(int outtime,const OutTimeCallBack & cb);
                void SetTimeOutCallBack(int outtime,OutTimeCallBack && cb);
                void OnClose() override;
                void ForceClose() override;
                void OnRead() override;
                void OnError(const std::string &msg) override;
                void OnWrite() override;
                void Send(const char * buf,size_t len);
                void Send(std::list<BufferNodePtr>&list);
                void OnTimeOut();
                void EnableCheckIdleTimeOut(int max_time);
                void ExtendLife();
            private:
                void SendInLoop(const char * buf,size_t len);
                void SendInLoop(std::list<BufferNodePtr>&list);            
                bool close_{false};
                CloseConnectionCallBack close_cb_;
                MsgBuffer msg_buffer_;
                MessageCallBack msg_cb_;
                WriteCompleteCallBack write_cp_cb;
                OutTimeCallBack outtime_cb;
                std::vector<struct iovec> io_vec_list;
                std::weak_ptr<TimeoutEntry> timeout_entry;
                int32_t max_idld_time{30};
        };
        struct BufferNode{
            BufferNode(void *buf,size_t len){
                addr = buf;
                this->len = len;
            }
            void *addr{nullptr};
            size_t len;
        };
    }
}
