#include "network/net/Connection.h"
using namespace tmms::network;
Connection::Connection(EventLoop * loop,int fd,const InetAddress &local,const InetAddress &peer)
:Event(loop,fd),local_addr_(local),peer_addr_(peer){

}

void Connection::SetLocalAddr(const InetAddress &local){
    local_addr_ = local;
}

void Connection::SetPeerAddr(const InetAddress &peer){
    peer_addr_ = peer;
}

void Connection::SetContext(int type,const std::shared_ptr<void> &context){
    context_[type] = context;
}

void Connection::SetContext(int type,std::shared_ptr<void> &&context){
    context_[type] = std::move(context);
}   

const InetAddress &Connection::GetLocalAddr(){
    return local_addr_;
}

const InetAddress &Connection::GetPeerAddr(){
    return peer_addr_;
}

void Connection::ClearContext(int type){
    context_[type].reset();
}

void Connection::ClearContext(){
    context_.clear();
}

void Connection::SetActiveCallBack(const ActiveCallBack & cb){
    active_cb_ = cb;
}

void Connection::SetActiveCallBack(ActiveCallBack && cb){
    active_cb_ = std::move(cb);
}

void Connection::Active(){
    if(!active_.load()){
        loop_->RunInLoop([this](){
            active_.store(true);
            if(active_cb_){
                active_cb_(std::dynamic_pointer_cast<Connection>(shared_from_this()));
            }
        });
    }
}

void Connection::Deactive(){
    active_.store(false);
}