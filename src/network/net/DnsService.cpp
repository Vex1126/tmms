#include"network/net/DnsService.h"
#include <netdb.h>
#include <functional>
using namespace tmms::network;
namespace{
    static InetAddressPtr inet_addr_null = nullptr;
}
DnsService::~DnsService(){

}

void DnsService::AddHost(const std::string &host){
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = hosts_info.find(host);
    if(iter != hosts_info.end()){
        return;
    }
    hosts_info[host] = std::vector<InetAddressPtr>();
}

InetAddressPtr DnsService::GetHostAddrss(const std::string &host,int index){
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = hosts_info.find(host);
    if(iter != hosts_info.end()){
        auto list = iter->second;
        return list[index%list.size()];
    }
    return inet_addr_null;
}

std::vector<InetAddressPtr> DnsService::GetHostAddrss(const std::string &host){
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = hosts_info.find(host);
    if(iter != hosts_info.end()){
        auto list = iter->second;
        return list;
    }
    return std::vector<InetAddressPtr>();   
}

void DnsService::UpDateHost(const std::string &host,std::vector<InetAddressPtr>& list){
    std::lock_guard<std::mutex> lock(lock_);
    hosts_info[host].swap(list);
}

std::unordered_map<std::string,std::vector<InetAddressPtr>> DnsService::GetHosts(){
    return hosts_info;
}

void DnsService::SetDnsServiceParam(int32_t interval,int32_t sleep,int32_t retry){
    interval_ = interval;
    sleep_ = sleep;
    retry_ = retry;
}
void DnsService::GetHostInfo(const std::string &host,std::vector<InetAddressPtr>& list){
    struct addrinfo ainfo,*res;
    memset(&ainfo,0x00,sizeof(ainfo));
    ainfo.ai_family = AF_UNSPEC;
    ainfo.ai_socktype = SOCK_STREAM;
    ainfo.ai_flags = AI_PASSIVE;
    auto ret = ::getaddrinfo(host.c_str(),nullptr,&ainfo,&res);
    if(ret == -1 || res == nullptr){
        return;
    }
    struct addrinfo *rp = res;
    for(;rp!=nullptr;rp=rp->ai_next){
        InetAddressPtr peeraddr = std::make_shared<InetAddress>();
        if(rp->ai_family == AF_INET){
            char ip[16]={0,};
            struct sockaddr_in *saddr=(sockaddr_in*)&rp->ai_addr;
            ::inet_ntop(AF_INET,&saddr->sin_addr.s_addr,ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(saddr->sin_port));
        }else if(rp->ai_family  == AF_INET6){

            char ip[INET6_ADDRSTRLEN]={0,};            
            struct sockaddr_in6 *saddr=(sockaddr_in6*)&rp->ai_addr;
            ::inet_ntop(AF_INET6,&saddr->sin6_addr,ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(saddr->sin6_port));
            peeraddr->SetIsIPV6(true);
        }    
        list.push_back(peeraddr); 
    }
}

void DnsService::Start(){
    running = true;
    thread_ = std::thread(std::bind(&DnsService::OnWork,this));
}

void DnsService::Stop(){
    running = false;
    if(thread_.joinable()){
        thread_.join();
    }
}

void DnsService::OnWork(){
    while(running){
        auto host_info = GetHosts();

            for(int i =0;i<retry_;i++){
               for(auto host:host_info){ 
                    std::vector<InetAddressPtr> list;
                    GetHostInfo(host.first,list); 
                    if(list.size()>0){
                        UpDateHost(host.first,list);
                        break;
                    }       
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_));   
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_));   
    }
}