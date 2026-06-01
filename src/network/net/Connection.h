#pragma once
#include "network/net/Event.h"
#include <functional>
#include "network/net/InetAddress.h"
#include <unordered_map>
#include <atomic>
namespace tmms{
    namespace network{
        enum{
            kNormalContext = 0,
            kRtmpContext,
            kHttpContext,
            kUserContext,
            kFlvContext
        };
        class Connection;
        using ContextPtr = std::shared_ptr<void>;
        using ConnectionPtr = std::shared_ptr<Connection>;
        using ActiveCallBack = std::function<void(ConnectionPtr)>;
        class Connection:public Event{
            public:
                Connection(EventLoop * loop,int fd,const InetAddress &local,const InetAddress &peer);
                virtual ~Connection() = default;
                void SetLocalAddr(const InetAddress &local);
                void SetPeerAddr(const InetAddress &peer);  
                void SetContext(int type,const std::shared_ptr<void> &context);    
                void SetContext(int type,std::shared_ptr<void> &&context);   
                const InetAddress &GetLocalAddr();
                const InetAddress &GetPeerAddr();
                template<typename T>
                std::shared_ptr<T> GetContext(int type){
                    auto iter = context_.find(type);
                    if(iter !=context_.end()){
                        return std::static_pointer_cast<T>(iter->second);
                    }
                    return std::shared_ptr<T>();
                }
                void ClearContext(int type);
                void ClearContext(); 
                void SetActiveCallBack(const ActiveCallBack & cb);    
                void SetActiveCallBack(ActiveCallBack && cb);    
                void Active();
                void Deactive();    
                virtual void ForceClose() = 0;
            protected:
                InetAddress local_addr_;
                InetAddress peer_addr_;                
            private:
                std::unordered_map<int,ContextPtr> context_;
                ActiveCallBack active_cb_;

                std::atomic<bool> active_{false};
        };
    }
}