#include"network/base/SocketOpt.h"
#include"network/base/NetWork.h"
using namespace tmms::network;
SocketOpt::SocketOpt(int sock,bool v6):sock_(sock),is_v6(v6){
 
}

SocketOpt::~SocketOpt(){

}

int SocketOpt::CreateNonblockingTcpSocket(int family){
    int sock = ::socket(family,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
    if(sock<0){
        NET_ERROR << "socke failed";
    }
    return sock;
}

int SocketOpt::CreateNonblockingUdpSocket(int family){
    int sock = ::socket(family,SOCK_DGRAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_UDP);
    if(sock<0){
        NET_ERROR << "socke failed";
    }
    return sock;
}

int SocketOpt::BindAddress(const InetAddress &lcoaladdr){
    if(lcoaladdr.IsIpV6()){
        struct sockaddr_in6 addr_in6;
        lcoaladdr.GetSockAddr((sockaddr *)&addr_in6);
        socklen_t len = sizeof(sockaddr_in6);
        return ::bind(sock_,(sockaddr *)&addr_in6,len);
    }else{
        struct sockaddr_in addr_in;
        lcoaladdr.GetSockAddr((sockaddr *)&addr_in);
        socklen_t len = sizeof(sockaddr_in);
        return ::bind(sock_,(sockaddr *)&addr_in,len);        
    }
}

int SocketOpt::Listen(){
    return ::listen(sock_,SOMAXCONN);
}

int SocketOpt::Accept(InetAddress *peeraddr){
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(sockaddr_in6);
    int sock=::accept4(sock_,(sockaddr *)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(sock>0){
        if(addr.sin6_family == AF_INET){
            char ip[16]={0,};
            struct sockaddr_in *saddr=(sockaddr_in*)&addr;
            ::inet_ntop(AF_INET,&saddr->sin_addr.s_addr,ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(saddr->sin_port));
        }else if(addr.sin6_family == AF_INET6){

            char ip[INET6_ADDRSTRLEN]={0,};
            ::inet_ntop(AF_INET6,&addr.sin6_addr,ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(addr.sin6_port));
            peeraddr->SetIsIPV6(true);
        }
    }
    return sock;
}

int SocketOpt::Connect(const InetAddress &addr){
    struct sockaddr_in6 addr_in;
    socklen_t len;
    if(addr.IsIpV6()){
        len=sizeof(struct sockaddr_in6);
    }else{
        len=sizeof(struct sockaddr_in);
    }
    memset(&addr_in,0x00,sizeof(struct sockaddr_in6));
    addr.GetSockAddr((sockaddr*)&addr_in);
    return connect(sock_,(sockaddr*)&addr_in,len);
}

InetAddressPtr SocketOpt::GetLocalAddr(){
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    ::getsockname(sock_,(sockaddr*)&addr,&len);
    InetAddressPtr localaddr = std::make_shared<InetAddress>();
    if(addr.sin6_family == AF_INET){
        char ip[16]={0,};
        struct sockaddr_in *saddr=(sockaddr_in*)&addr;
        ::inet_ntop(AF_INET,&saddr->sin_addr.s_addr,ip,sizeof(ip));
        localaddr->SetAddr(ip);
        localaddr->SetPort(ntohs(saddr->sin_port));
    }else if(addr.sin6_family == AF_INET6){
        char ip[INET6_ADDRSTRLEN]={0,};
        ::inet_ntop(AF_INET6,&addr.sin6_addr,ip,sizeof(ip));
        localaddr->SetAddr(ip);
        localaddr->SetPort(ntohs(addr.sin6_port));
        localaddr->SetIsIPV6(true);
    } 
    return localaddr;
}

InetAddressPtr SocketOpt::GetPeerAddr(){
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    ::getpeername(sock_,(sockaddr*)&addr,&len);
    InetAddressPtr peeraddr = std::make_shared<InetAddress>();
    if(addr.sin6_family == AF_INET){
        char ip[16]={0,};
        struct sockaddr_in *saddr=(sockaddr_in*)&addr;
        ::inet_ntop(AF_INET,&saddr->sin_addr.s_addr,ip,sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(saddr->sin_port));
    }else if(addr.sin6_family == AF_INET6){
        char ip[INET6_ADDRSTRLEN]={0,};
        ::inet_ntop(AF_INET6,&addr.sin6_addr,ip,sizeof(ip));
        peeraddr->SetAddr(ip);
        peeraddr->SetPort(ntohs(addr.sin6_port));
        peeraddr->SetIsIPV6(true);
    } 
    return peeraddr;
}

void SocketOpt::SetTcpNoDelay(bool on){
    int optvalue = on?1:0;
    ::setsockopt(sock_,IPPROTO_TCP,TCP_NODELAY,&optvalue,sizeof(optvalue));
}

void SocketOpt::SetReuseAddr(bool on){
    int optvalue = on?1:0;
    ::setsockopt(sock_,SOL_SOCKET,SO_REUSEADDR,&optvalue,sizeof(optvalue));
}

void SocketOpt::SetPortAddr(bool on){
    int optvalue = on?1:0;
    ::setsockopt(sock_,SOL_SOCKET,SO_REUSEPORT,&optvalue,sizeof(optvalue));
}

void SocketOpt::SetKeepAlive(bool on){
    int optvalue = on?1:0;
    ::setsockopt(sock_,SOL_SOCKET,SO_KEEPALIVE,&optvalue,sizeof(optvalue));
}

void SocketOpt::SetNonBlocking(bool on){
    int flag = ::fcntl(sock_,F_GETFL,0);
    if(on){
        flag |= O_NONBLOCK;
    }else{
        flag &= ~O_NONBLOCK;
    }
    ::fcntl(sock_,F_SETFL,flag);
}