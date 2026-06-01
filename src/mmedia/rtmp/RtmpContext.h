#pragma once
#include"network/net/TcpConnection.h"
#include"mmedia/rtmp/RtmpHandler.h"
#include"mmedia/rtmp/RtmpHandShake.h"
#include"mmedia/base/MMediaLog.h"
#include"mmedia/base/Packet.h"
#include"mmedia/base/RtmpHeader.h"
#include<unordered_map>
#include"mmedia/base/BytesReader.h"
#include"mmedia/base/BytesWriter.h"
#include"mmedia/rtmp/amf/AMFObject.h"
#include<functional>
#include<memory>
using namespace tmms::mm;
namespace tmms{
    namespace mm{
        enum{
            kRtmpHandShake = 0,
            kRtmpWaitingDone = 1,
            kRtmpMessage = 2
        };
        using CommandCall = std::function<void (AMFObject &obj)>;
        class RtmpContext{
            public:
                RtmpContext(const TcpConnectionPtr & con,RtmpHandler* handler,bool client = false);
                ~RtmpContext()=default;

                int32_t Parse(MsgBuffer & buf);
                void StartHandShake();
                void WriteComplete();
                
                int32_t MessagePrase(MsgBuffer & buf);
                void MessageComplete(PacketPtr&&data);

                bool BuildChunk(const PacketPtr &packet,uint32_t timestamp=0,bool fmt0=false); 

                void Send();
                void CheckAndSend();
                void HandleChunkSize(PacketPtr &packet);
                void HandleAckWindowSize(PacketPtr &packet);
                void HandleUserMessage(PacketPtr &packet);
                void HandleAMFCommeand(PacketPtr &packet,bool fmt3=false);
                void SendConnect();
                void HandleConnect(AMFObject &obj);
                void CreateStream();
                void HandleCreateStream(AMFObject &obj);
                void SendStatus(const std::string & level,const std::string & code,const std::string & description);
                void SendPlay();
                void HandlePlay(AMFObject &obj);
                void ParseNameAndUrl();
                void SendPublish();
                void HandlePublish(AMFObject &obj);  
                void Play(const std::string &url);
                void Publish(const std::string &url);
                void HandleResult(AMFObject &obj);
                void HandleError(AMFObject &obj);  
                void SetPacketType(PacketPtr &packet);
            private:
                bool Ready()const;
                void PushOutPacket(PacketPtr && packet);               
                void SendSetChunkSize();
                void SendAckWindowSize();
                void SendPeerBandWidth();
                void SetBytesRevc();
                void SetUserCtlMessage(short nType,int32_t value_1,int32_t value_2);
                bool BuildChunk(PacketPtr &&packet,uint32_t timestamp=0,bool fmt0=false);            
                TcpConnectionPtr connection_;
                RtmpHandler* rtmp_handler_{nullptr};
                RtmpHandShake hand_shake_;
                int32_t state_{kRtmpHandShake};
                std::unordered_map<uint32_t,RtmpMsgHeaderPtr> in_message_headers_;
                std::unordered_map<uint32_t,PacketPtr> in_packets_;
                std::unordered_map<uint32_t,uint32_t> in_deltas_;
                std::unordered_map<uint32_t,bool> in_ext_;
                int32_t in_chunk_size_{128};
                char out_buffer_[4096];
                char *out_current_{nullptr};
                std::unordered_map<uint32_t,RtmpMsgHeaderPtr> out_message_headers_; 
                std::unordered_map<uint32_t,uint32_t> out_deltas_;  
                int32_t out_chunk_size_{4096};  
                std::list<PacketPtr> out_wait_queue_; 
                std::list<BufferNodePtr> sending_bufs_;
                std::list<PacketPtr> out_sending_packet_;    
                bool sending_{false};
                int32_t ack_size_{2500000};
                int32_t in_bytes{0};
                int32_t last_left_{0};   
                std::string app_;    
                std::string tc_url_;     
                std::string name_;
                std::string session_name_;
                std::string parma_;
                bool isplaying{false};
                std::unordered_map<std::string,CommandCall> commands_;
                bool is_client{false};
        };
        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    }
} 

