#pragma once
#include "network/base/MsgBuffer.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/Connection.h"
#include "network/net/TcpConnection.h"
#include <functional>
#include <list>
#include <vector>
#include <sys/uio.h>
namespace tmms{
    namespace network{
        class UdpSocket;
        struct UdpBufferNode;
        using UdpSocketPtr = std::shared_ptr<UdpSocket>;
        using UdpBufferNodePtr = std::shared_ptr<UdpBufferNode>;
        using UdpCloseConnectionCallBack = std::function<void(const UdpSocketPtr&)>;
        using UdpMessageCallBack = std::function<void(const InetAddress & addr,MsgBuffer& buf)>;
        using UdpWriteCompleteCallBack = std::function<void(const UdpSocketPtr&)>;
        using UdpOutTimeCallBack = std::function<void(const UdpSocketPtr&)>;
        struct UdpTimeoutEntry{
            public:
                UdpTimeoutEntry(const UdpSocketPtr& c):conn(c){
                
                }
                ~UdpTimeoutEntry(){}
                std::weak_ptr<UdpSocket> conn;
        };
        class UdpSocket:public Connection{
            public:
                UdpSocket(EventLoop * loop,int fd,const InetAddress & local,const InetAddress & peer);
                ~UdpSocket();
                void OnTimeOut();
                void OnRead() override;
                void OnError(const std::string &msg) override;
                void OnWrite() override;
                void OnClose() override;
                void SetCloseCallBack(const UdpCloseConnectionCallBack & cb);
                void SetCloseCallBack(UdpCloseConnectionCallBack && cb);
                void SetRecvMsgCallBack(const UdpMessageCallBack & cb);
                void SetRecvMsgCallBack(UdpMessageCallBack && cb);
                void SetWriteCompleteCallBack(const UdpWriteCompleteCallBack & cb);
                void SetWriteCompleteCallBack(UdpWriteCompleteCallBack && cb);
                void SetTimeOutCallBack(int outtime,const UdpOutTimeCallBack & cb);
                void SetTimeOutCallBack(int outtime,UdpOutTimeCallBack && cb);
                void EnableCheckIdleTimeOut(int max_time);
                void ExtendLife();
                void Send(const char * buf,size_t len,struct sockaddr* addr,socklen_t len_);
                void Send(std::list<UdpBufferNodePtr>&list);
                void SendInLoop(const char * buf,size_t len,struct sockaddr* addr,socklen_t len_);
                void SendInLoop(std::list<UdpBufferNodePtr>&list);   
                void ForceClose() override;
            private:
                std::list<UdpBufferNodePtr> buffer_list_;
                bool closed_{false};
                int32_t message_max_len{65535};
                MsgBuffer message_buffer;
                UdpMessageCallBack message_cb_;
                UdpWriteCompleteCallBack write_complete_cb_;
                UdpOutTimeCallBack timeout_cb_;
                UdpCloseConnectionCallBack close_cb_;
                std::weak_ptr<UdpTimeoutEntry> timeout_entry;
                int32_t max_idle_time{30};
        };
        struct UdpBufferNode:public BufferNode{
            UdpBufferNode(void *buf,size_t len,struct sockaddr *addr,socklen_t slen):BufferNode(buf,len){
                addr_ = addr;
                len_ = slen;
            }
            struct sockaddr *addr_{nullptr};
            socklen_t len_;
        };
    }
}