#pragma once
#include "network/net/InetAddress.h"
#include "base/Singleton.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
namespace tmms{
    namespace network{
        using InetAddressPtr = std::shared_ptr<InetAddress>;
        class DnsService:public base::Singleton<DnsService>{
            public:
                DnsService()=default;
                ~DnsService();
                void AddHost(const std::string &host);
                InetAddressPtr GetHostAddrss(const std::string &host,int index = 0);
                std::vector<InetAddressPtr> GetHostAddrss(const std::string &host);
                void UpDateHost(const std::string &host,std::vector<InetAddressPtr>& list);
                std::unordered_map<std::string,std::vector<InetAddressPtr>> GetHosts();
                void SetDnsServiceParam(int32_t interval,int32_t sleep,int32_t retry);
                void GetHostInfo(const std::string &host,std::vector<InetAddressPtr>& list);
                void Start();
                void Stop();
                void OnWork();
            private:
                std::thread thread_;
                bool running{false};
                std::mutex lock_;
                std::unordered_map<std::string,std::vector<InetAddressPtr>> hosts_info;
                int32_t interval_{180};
                int32_t sleep_{200};
                int32_t retry_{3};
            };
            #define sDnsService tmms::base::Singleton<tmms::base::DnsService>::Instance()
    }
}