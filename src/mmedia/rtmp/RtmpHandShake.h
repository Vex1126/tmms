#pragma once
#include<cstdint>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <random>
#include <cstring>
#include "network/net/TcpConnection.h"
#include "network/base/MsgBuffer.h"
using namespace tmms::network;
namespace tmms{
    namespace mm{
        const int kRtmpHandShakePacketSize = 1536;
        enum RtmpHandShakeState{

            kHandShakeInit,
            kHandShakePostC0C1,
            kHandShakeWaitS0S1,
            kHandShakePostC2,
            kHandShakeWaitS2,
            kHandShakeDoning,

            kHandShakeWaitC0C1,
            kHandShakePostS0S1,
            kHandShakePostS2,
            kHandShakeWaitC2,
        };
        class RtmpHandShake{
            public:
                RtmpHandShake(const TcpConnectionPtr& con ,bool client=false);
                ~RtmpHandShake()=default;
                void Start();
                void CreateC1S1();
                int32_t CheckC1S1(const char * data ,int bytes);
                void SendC1S1();

                void CreateC2S2(const char * data ,int bytes,int offset);
                bool CheckC2S2(const char * data ,int bytes);
                void SendC2S2();

                int HandShake(MsgBuffer& msg);
                void WriteComplete();
            private:
                TcpConnectionPtr connection_;
                uint8_t GetRandom();
                bool is_client_{false};
                bool is_complex_handshake_{true};
                uint8_t digest_[SHA256_DIGEST_LENGTH];
                uint8_t C1S1_[kRtmpHandShakePacketSize+1];
                uint8_t C2S2_[kRtmpHandShakePacketSize];
                RtmpHandShakeState state_{kHandShakeInit};
        };
        using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
    }
}