#include "network/net/TcpConnection.h"
#include "network/base/NetWork.h"
#include <iostream>
using namespace tmms::network;
TcpConnection::TcpConnection(EventLoop * loop,int fd,const InetAddress &local,const InetAddress &peer)
:Connection(loop,fd,local,peer){

}

TcpConnection::~TcpConnection(){
    // 析构阶段直接关闭fd，避免 shared_from_this 触发 bad_weak_ptr
    Event::Close();
}

void TcpConnection::SetCloseCallBack(const CloseConnectionCallBack & cb){
    close_cb_ = cb;
}
void TcpConnection::SetCloseCallBack(CloseConnectionCallBack && cb){
    close_cb_ = std::move(cb);
}

void TcpConnection::OnClose(){
    loop_->AssertInLoopThread();
    if(!close_){
        if(close_cb_){
            close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
        Event::Close();
    }
    close_ = true;
}
void TcpConnection::ForceClose(){
    loop_->RunInLoop([this]{
        OnClose();
    });
}

void TcpConnection::SetRecvMsgCallBack(const MessageCallBack & cb){
    msg_cb_ = cb;
}

void TcpConnection::SetRecvMsgCallBack(MessageCallBack && cb){
    msg_cb_ = std::move(cb);
}

void TcpConnection::OnRead(){
    if(close_){
        NET_ERROR << "host:" << peer_addr_.ToIpPort() <<" is close";
        OnClose();
        return;
    }
    ExtendLife();
    while(true){
        int err = 0;
        auto ret = msg_buffer_.readFd(fd_,&err);
        if(ret > 0){
            if(msg_cb_){
                msg_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()),msg_buffer_);
            }
            continue;
        }else if(ret == 0){
            OnClose();
            break;      
        }else{
            if(err == EINTR){
                continue;
            }
            if(err == EAGAIN || err == EWOULDBLOCK){
                break;
            }
            NET_ERROR << "read error";
            OnClose();                
            break;
        }
    }

}

void TcpConnection::OnError(const std::string &msg){
    NET_ERROR<< "host:" << peer_addr_.ToIpPort() << "error:"<<msg;
    OnClose();                  
}

void TcpConnection::SetWriteCompleteCallBack(const WriteCompleteCallBack & cb){
    write_cp_cb = cb;
}

void TcpConnection::SetWriteCompleteCallBack(WriteCompleteCallBack && cb){
    write_cp_cb = std::move(cb);
}

void TcpConnection::OnWrite(){
    if(close_){
        NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"is close";
        return;
    }
    ExtendLife();
    if(!io_vec_list.empty()){
        while(true){
            auto ret = ::writev(fd_,&io_vec_list[0],io_vec_list.size());
            if(ret >= 0){
                while(ret > 0){
                    if(io_vec_list.front().iov_len > ret){
                        io_vec_list.front().iov_base = (char*)io_vec_list.front().iov_base + ret;
                        io_vec_list.front().iov_len-=ret;
                        ret = 0;
                        break;
                    }else{
                        ret-=io_vec_list.front().iov_len;
                        io_vec_list.erase(io_vec_list.begin());
                    }
                }
                if(io_vec_list.empty()){
                    if(write_cp_cb){
                        write_cp_cb(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
                    }
                    if(io_vec_list.empty()){
                        EnableWriting(false);
                        if(msg_buffer_.readableBytes() > 0 && msg_cb_){
                            auto conn = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
                            msg_cb_(conn, msg_buffer_);
                        }
                    } else {
                        EnableWriting(true);
                    }
                    return;
                }
            }else{
                if(ret!=EINTR && ret!=EAGAIN && ret!=EWOULDBLOCK){
                    NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"write failed";
                    OnClose();
                    return;
                }
                break;
            }
        }        
    }else{
        EnableWriting(false);
        if(write_cp_cb){
            write_cp_cb(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }        
    }
}

void TcpConnection::Send(const char * buf,size_t len){
    loop_->RunInLoop([this,buf,len](){
        SendInLoop(buf,len);
    });
}

void TcpConnection::Send(std::list<BufferNodePtr>&list){
    loop_->RunInLoop([this,&list](){
        SendInLoop(list);
    });
}

void TcpConnection::SendInLoop(const char * buf,size_t len){
    if(close_){
        NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"is close";
        return;
    }
    ssize_t  send_len = 0;
    if(io_vec_list.empty()){
        send_len = ::write(fd_,buf,len);
        if(send_len < 0){
            if(errno !=EINTR && errno !=EAGAIN && errno !=EWOULDBLOCK){
                NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"write failed";
                OnClose();
                return;
            }
            send_len = 0;            
        }
        len-=send_len;
        if(len == 0){
            if(write_cp_cb){
                write_cp_cb(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
            }
        }

        if(msg_buffer_.readableBytes() > 0 && msg_cb_){
            auto conn = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
            msg_cb_(conn, msg_buffer_);
        }
    }
    if(len>0){
        struct iovec vec;
        vec.iov_base = (char*)buf+send_len;
        vec.iov_len = len;
        io_vec_list.push_back(vec);
        EnableWriting(true);
    }

}

void TcpConnection::SendInLoop(std::list<BufferNodePtr>&list){
    if(close_){
        NET_ERROR << "host:" <<peer_addr_.ToIpPort()<<"is close";
        return;
    }    
    for(auto &l:list){
        struct iovec vec;
        vec.iov_base = (char*)l->addr;
        vec.iov_len = l->len;
        io_vec_list.push_back(vec);
        EnableWriting(true);
    }
    if(!io_vec_list.empty()){
        EnableWriting(true);       
    }
}

void TcpConnection::SetTimeOutCallBack(int outtime,const OutTimeCallBack & cb){
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(outtime,[&cp,&cb](){
        cb(cp);
    });
}

void TcpConnection::SetTimeOutCallBack(int outtime,OutTimeCallBack && cb){
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(outtime,[&cp,cb](){
        cb(cp);
    });
}

void TcpConnection::OnTimeOut(){
    NET_ERROR<<"host:"<<peer_addr_.ToIpPort()<<"couttime close";
    OnClose();
}

void TcpConnection::EnableCheckIdleTimeOut(int max_time){
    auto tp = std::make_shared<TimeoutEntry>(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    max_idld_time = max_time;
    timeout_entry = tp;
    loop_->InsertEntry(max_idld_time,tp);
}

void TcpConnection::ExtendLife(){
    auto tp = timeout_entry.lock();
    if(tp){
      loop_->InsertEntry(max_idld_time,tp);  
    }
}

TimeoutEntry::~TimeoutEntry(){
    auto co = conn.lock();
    if (co) {
        co->OnTimeOut();
    }
}
