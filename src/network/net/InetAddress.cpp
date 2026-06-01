#include"network/net/InetAddress.h"
using namespace tmms::network;
InetAddress::InetAddress(const std::string &ip,uint16_t port,bool bv6):addr_(ip),port_(std::to_string(port)){

}

InetAddress::InetAddress(const std::string &host,bool bv6){
    GetIpAndPort(host,addr_,port_);
    is_ipv6 = bv6;
}

void InetAddress::GetIpAndPort(const std::string &host,std::string &ip,std::string &port){
    auto pos = host.find_last_of(':');
    if(pos!=std::string::npos){
        ip = host.substr(0,pos);
        port = host.substr(pos+1);
    }else{
        ip = host;
    }
}

void InetAddress::SetHost(const std::string &host){
    GetIpAndPort(host,addr_,port_);
}

void InetAddress::SetAddr(const std::string &addr){
    addr_ = addr;
}

void InetAddress::SetPort(uint16_t port){
    port_ = std::to_string(port);
}

void InetAddress::SetIsIPV6(bool is_bv6){
    is_ipv6 = is_bv6;
}

const std::string & InetAddress::Ip()const{
    return addr_;
}

uint32_t InetAddress::IPv4(const char * ip)const{
    struct sockaddr_in addr_in;
    memset(&addr_in,0x00,sizeof(sockaddr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = 0;
    if(::inet_pton(AF_INET,ip,&addr_in.sin_addr)<=0){
        NET_ERROR << "IP:"<<ip<<"conver failed";
    } 
    return ntohl(addr_in.sin_addr.s_addr);
}

uint32_t InetAddress::IPv4()const{
    return IPv4(addr_.c_str());
}

uint16_t InetAddress::Port()const{
    return std::atoi(port_.c_str());
}
std::string InetAddress::ToIpPort()const{
    std::stringstream ss;
    ss<<addr_<<":"<<port_;
    return ss.str();
}

void InetAddress::GetSockAddr(struct sockaddr *saddr)const{
    if(is_ipv6){
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)saddr;
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(std::atoi(port_.c_str()));
        if(::inet_pton(AF_INET6,addr_.c_str(),&addr_in6->sin6_addr)){

        }
        return;        
    }else{
        struct sockaddr_in *addr_in = (struct sockaddr_in *)saddr;
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(std::atoi(port_.c_str()));
        if(::inet_pton(AF_INET,addr_.c_str(),&addr_in->sin_addr)){

        }
        return; 
    }

}

bool InetAddress::IsIpV6()const{
    return is_ipv6;
}

bool InetAddress::IsWanIp()const{
    int32_t a_start = IPv4("0.0.0.0");
    int32_t a_end = IPv4("10.255.255.255");
    int32_t b_start = IPv4("172.16.0.0");
    int32_t b_end = IPv4("172.31.255.255");
    int32_t c_start = IPv4("192.168.0.0");
    int32_t c_end = IPv4("192.168.255.255");
    int32_t ip = IPv4();
    bool is_a = ip>=a_start && ip<=a_end;
    bool is_b = ip>=b_start && ip<=b_end;
    bool is_c = ip>=c_start && ip<=c_end;
    return !is_a&&!is_b&&!is_c&&ip!=IN_LOOPBACKNET;
}

bool InetAddress::IsLanIp()const{
    int32_t a_start = IPv4("0.0.0.0");
    int32_t a_end = IPv4("10.255.255.255");
    int32_t b_start = IPv4("172.16.0.0");
    int32_t b_end = IPv4("172.31.255.255");
    int32_t c_start = IPv4("192.168.0.0");
    int32_t c_end = IPv4("192.168.255.255");
    int32_t ip = IPv4();
    bool is_a = ip>=a_start && ip<=a_end;
    bool is_b = ip>=b_start && ip<=b_end;
    bool is_c = ip>=c_start && ip<=c_end;
    return is_a||is_b||is_c;
}

bool InetAddress::IsLoopBackIp()const{
    return addr_ == "127.0.0.0";
}