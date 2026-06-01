#include "network/net/UdpSocket.h"
using namespace tmms::network;
UdpSocket::UdpSocket(EventLoop * loop,int fd,const InetAddress & local,const InetAddress & peer)
:Connection(loop,fd,local,peer){

}

UdpSocket::~UdpSocket(){

}

void UdpSocket::SetCloseCallBack(const UdpCloseConnectionCallBack & cb){
    close_cb_ = cb;
}

void UdpSocket::SetCloseCallBack(UdpCloseConnectionCallBack && cb){
    close_cb_ = std::move(cb);
}

void UdpSocket::SetRecvMsgCallBack(const UdpMessageCallBack & cb){
    message_cb_ = cb;
}
void UdpSocket::SetRecvMsgCallBack(UdpMessageCallBack && cb){
    message_cb_ = std::move(cb);
}

void UdpSocket::SetWriteCompleteCallBack(const UdpWriteCompleteCallBack & cb){
    write_complete_cb_ = cb;
}

void UdpSocket::SetWriteCompleteCallBack(UdpWriteCompleteCallBack && cb){
    write_complete_cb_ = std::move(cb);
}

void UdpSocket::SetTimeOutCallBack(int outtime,const UdpOutTimeCallBack & cb){
    auto us = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->RunAfter(outtime,[this,cb,us](){
        cb(us);
    });
}

void UdpSocket::SetTimeOutCallBack(int outtime,UdpOutTimeCallBack && cb){
    auto us = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->RunAfter(outtime,[this,cb,us](){
        cb(us);
    });
}

void UdpSocket::OnTimeOut(){
    NET_ERROR<<"host:"<<peer_addr_.ToIpPort()<<"couttime close";
    OnClose();
}

void UdpSocket::OnRead(){
    if(closed_){
        NET_WARN << "host:" <<peer_addr_.ToIpPort()<<"has closed";
        OnClose();
        return;
    }
    ExtendLife();
    while(true){
        struct sockaddr_in6 socket_addr;
        socklen_t len = sizeof(sockaddr_in6);

        auto ret = ::recvfrom(fd_,message_buffer.beginWrite(),message_max_len,0,(sockaddr*)&socket_addr,&len);
        if(ret > 0){
            InetAddress peeraddr;   
            message_buffer.hasWritten(ret);   
            if(socket_addr.sin6_family == AF_INET){
                char ip[16]={0,};
                struct sockaddr_in *saddr=(sockaddr_in*)&socket_addr;
                ::inet_ntop(AF_INET,&saddr->sin_addr.s_addr,ip,sizeof(ip));
                peeraddr.SetAddr(ip);
                peeraddr.SetPort(ntohs(saddr->sin_port));
            }else if(socket_addr.sin6_family == AF_INET6){

                char ip[INET6_ADDRSTRLEN]={0,};
                ::inet_ntop(AF_INET6,&socket_addr.sin6_addr,ip,sizeof(ip));
                peeraddr.SetAddr(ip);
                peeraddr.SetPort(ntohs(socket_addr.sin6_port));
                peeraddr.SetIsIPV6(true);
            }
            if(message_cb_){
                message_cb_(peeraddr,message_buffer);
            }

        }else if(ret<0){
            if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK){
                NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"read fail erron:" <<errno;
                OnClose();
                return;  
            }
        }
    }
}

void UdpSocket::OnError(const std::string &msg){

}

void UdpSocket::OnWrite(){
    if(closed_){
        NET_WARN << "host:" <<peer_addr_.ToIpPort()<<"has closed";
        OnClose();
        return;
    }    

    while(true){
        if(!buffer_list_.empty()){
            auto buf = buffer_list_.front();
            auto ret = sendto(fd_,buf->addr,buf->len,0,buf->addr_,buf->len_);
            if(ret > 0){
                buffer_list_.pop_front();
            }else if(ret < 0){
                if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK){
                    NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"read fail erron:" <<errno;
                    OnClose();
                    return;  
                }
                break;
            }
        }
        if(buffer_list_.empty()){
            write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        }        
    }

}

void UdpSocket::OnClose(){
    if(!closed_){
        closed_ = true;
        if(closed_){
        close_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));            
        }
        Event::Close();
    }
}

void UdpSocket::EnableCheckIdleTimeOut(int max_time){
    auto tp = std::make_shared<UdpTimeoutEntry>(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
    max_idle_time = max_time;
    timeout_entry = tp;
    loop_->InsertEntry(max_idle_time,tp);
}

void UdpSocket::ExtendLife(){
    auto tp = timeout_entry.lock();
    if(tp){
      loop_->InsertEntry(max_idle_time,tp);  
    }
}

void UdpSocket::Send(const char * buf,size_t len,struct sockaddr* addr,socklen_t len_){
    loop_->RunInLoop([this,buf,len,addr,len_](){
        SendInLoop(buf,len,addr,len_);
    });
}
void UdpSocket::Send(std::list<UdpBufferNodePtr>&list){
    loop_->RunInLoop([this,&list](){
        SendInLoop(list);
    });
}
void UdpSocket::SendInLoop(const char * buf,size_t len,struct sockaddr* addr,socklen_t len_){
    if(buffer_list_.empty()){
        auto ret = ::sendto(fd_,buf,len,0,addr,len_);
        if(ret > 0){
            return;
        }
    }

    auto node = std::make_shared<UdpBufferNode>((void*)buf,len,addr,len_);
    buffer_list_.push_back(node);
    EnableWriting(true);
}
void UdpSocket::SendInLoop(std::list<UdpBufferNodePtr>&list){
    for(auto &l:list){
        buffer_list_.emplace_back(l);
    }
    if(!buffer_list_.empty()){
        EnableWriting(true);
    }
}

void UdpSocket::ForceClose(){
    loop_->RunInLoop([this](){
        OnClose();
    });    
}