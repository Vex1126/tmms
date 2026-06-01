
#include"mmedia/rtmp/RtmpHandShake.h"
#include"mmedia/base/MMediaLog.h"
#include"base/TTime.h"
using namespace tmms::mm;
#include <openssl/ssl.h>
#include <openssl/err.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <openssl/sha.h>
#include <openssl/hmac.h>

#if OPENSSL_VERSION_NUMBER > 0x10100000L
    // OpenSSL 1.1.0+ and 3.0+
    // 注意：在 OpenSSL 3.0 中这些函数被标记为废弃，但我们通过上面的 pragma 忽略了警告
    #define HMAC_setup(ctx, key, len)     ctx = HMAC_CTX_new(); HMAC_Init_ex(ctx, key, len, EVP_sha256(), 0)
    #define HMAC_crunch(ctx, buf, len)    HMAC_Update(ctx, buf, len)
    #define HMAC_finish(ctx, dig, dlen)   HMAC_Final(ctx, dig, &dlen); HMAC_CTX_free(ctx)
#else
    // Old OpenSSL
    #define HMAC_setup(ctx, key, len)     HMAC_CTX_init(&ctx); HMAC_Init_ex(&ctx, key, len, EVP_sha256(), 0)
    #define HMAC_crunch(ctx, buf, len)    HMAC_Update(&ctx, buf, len)
    #define HMAC_finish(ctx, dig, dlen)   HMAC_Final(&ctx, dig, &dlen); HMAC_CTX_cleanup(&ctx)
#endif
namespace
{
    static const uint8_t rtmp_server_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
        'S', 'e', 'r', 'v', 'e', 'r', ' ', '0', '0', '1',
        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };   

    #define PLAYER_KEY_OPEN_PART_LEN 30 ///< length of partial key used for first client digest signing

    static const uint8_t rtmp_player_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ', '0', '0', '1',

        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };

    #define SERVER_KEY_OPEN_PART_LEN 36 ///< length of partial key used for first server digest signing


    void CalculateDigest(const uint8_t *src, int len, int gap, const uint8_t *key, int keylen, uint8_t *dst){
        uint32_t digestLen=0;
        #if OPENSSL_VERSION_NUMBER > 0x10100000L
        HMAC_CTX *ctx;
        #else
        HMAC_CTX ctx;
        #endif
        HMAC_setup(ctx, key, keylen);
        if(gap<=0)
        {
            HMAC_crunch(ctx, src, len);
        }
        else
        {
            HMAC_crunch(ctx, src, gap);
            HMAC_crunch(ctx, src+gap+SHA256_DIGEST_LENGTH, len-gap-SHA256_DIGEST_LENGTH);
        }

        HMAC_finish(ctx, dst, digestLen);
    }

    bool VerifyDigest(uint8_t *buf, int digest_pos, const uint8_t *key, size_t keyLen){
        uint8_t digest[SHA256_DIGEST_LENGTH];
        CalculateDigest(buf, kRtmpHandShakePacketSize, digest_pos, key, keyLen, digest);
        return memcmp(&buf[digest_pos], digest, SHA256_DIGEST_LENGTH) == 0;
    }

    int32_t GetDigestOffset(const uint8_t *buf, int off, int mod_val)
    {
        uint32_t offset = 0;
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(buf + off);
        uint32_t res;

        offset = ptr[0]+ptr[1]+ptr[2]+ptr[3];
        
        res = (offset % mod_val) + (off+4);
        return res;
    }

    static const unsigned char rtmp_server_ver[4] = {
        0x0D,0x0E,0x0A,0x0D
    };

    static const unsigned char rtmp_client_ver[4] = {
        0x0C,0x00,0x0D,0x0E
    };
}

RtmpHandShake::RtmpHandShake(const TcpConnectionPtr& con ,bool client):connection_(con),is_client_(client){

}

void RtmpHandShake::Start(){
        CreateC1S1();     
    if(is_client_){
        state_ = kHandShakePostC0C1;
        SendC1S1();
    }else{
        state_ = kHandShakeWaitC0C1;
    }
}

uint8_t RtmpHandShake::GetRandom(){
    std::mt19937 mt{std::random_device{}()};
    std::uniform_int_distribution<> rand(0,256);
    return rand(mt)%256;
}

void RtmpHandShake::CreateC1S1(){
    for(int i=0;i<kRtmpHandShakePacketSize+1;i++){
        C1S1_[i] = GetRandom();
    }

    C1S1_[0] = 0x03;
    memset(C1S1_+1,0x00,4);
    if(!is_complex_handshake_){
        memset(C1S1_+5,0x00,4);
    }else{
        auto offset = GetDigestOffset(C1S1_+1,8,728);
        uint8_t *data = C1S1_+1+offset;
        if(is_client_){
            memcpy(C1S1_+5,rtmp_client_ver,4);
            CalculateDigest(C1S1_+1,kRtmpHandShakePacketSize,offset,rtmp_player_key,PLAYER_KEY_OPEN_PART_LEN,data);
        }else{
            memcpy(C1S1_+5,rtmp_server_ver,4);
            CalculateDigest(C1S1_+1,kRtmpHandShakePacketSize,offset,rtmp_server_key,SERVER_KEY_OPEN_PART_LEN,data);
        }
    }
}

int32_t RtmpHandShake::CheckC1S1(const char * data ,int bytes){
    if(bytes!=1537){
        RTMP_ERROR << "C1S1 数据长度错误: " << bytes << " (期望 1537)";
        return -1;
    }   
    if(data[0]!=0x03){
        RTMP_ERROR << "C1S1 版本错误: 0x" << std::hex << (int)data[0] << std::dec;
        return -1;        
    }
    uint8_t *handshake = (uint8_t *)data+1;    
    auto offset = GetDigestOffset(handshake,8,728);
    uint32_t *version = (uint32_t*)(data+5);
    if(*version==0){
        is_complex_handshake_ = false;
        return 0;
    }
    if(is_complex_handshake_){
        if(is_client_){
            if(!VerifyDigest(handshake,offset,rtmp_server_key,SERVER_KEY_OPEN_PART_LEN)){
                offset = GetDigestOffset(handshake,772,728);
                if(!VerifyDigest(handshake,offset,rtmp_server_key,SERVER_KEY_OPEN_PART_LEN)){
                    return -1;
                }
            }
        }else{
            if(!VerifyDigest(handshake,offset,rtmp_player_key,PLAYER_KEY_OPEN_PART_LEN)){
                offset = GetDigestOffset(handshake,772,728);
                if(!VerifyDigest(handshake,offset,rtmp_player_key,PLAYER_KEY_OPEN_PART_LEN)){
                    return -1;
                }
            }            
        }
    }else {
        // 服务端检查 C1 (用 Player Key 验证)
        offset = GetDigestOffset(handshake, 8, 728);
        if(!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN)){
            offset = GetDigestOffset(handshake, 772, 728);
            if(!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN)){
                return -1;
            }
        }
    }

    return offset;
}

void RtmpHandShake::SendC1S1(){
    connection_->Send((const char *)C1S1_,1537);
}

void RtmpHandShake::CreateC2S2(const char * data ,int bytes,int offset){
    memcpy(C2S2_, data, kRtmpHandShakePacketSize);
    memcpy(C2S2_,data,8);
    int64_t timestamp_val = TTime::Now();
    
    uint32_t time_32 = (uint32_t)timestamp_val;

    // 手动按大端序写入 (Big-Endian)
    C2S2_[0] = (time_32 >> 24) & 0xFF;
    C2S2_[1] = (time_32 >> 16) & 0xFF;
    C2S2_[2] = (time_32 >> 8)  & 0xFF;
    C2S2_[3] = (time_32)       & 0xFF;
    if(is_complex_handshake_){
        uint8_t digest[32];
        if(is_client_){
            CalculateDigest(digest_,32,0,rtmp_player_key,sizeof(rtmp_player_key),digest);
        }else{
            CalculateDigest(digest_,32,0,rtmp_server_key,sizeof(rtmp_server_key),digest);
        }
        CalculateDigest(C2S2_,kRtmpHandShakePacketSize-32,0,digest,32,&C2S2_[kRtmpHandShakePacketSize-32]);
    }

}

bool RtmpHandShake::CheckC2S2(const char * data ,int bytes){
    return true;
}

void RtmpHandShake::SendC2S2(){
    connection_->Send((const char *)C2S2_,1536);
}

int RtmpHandShake::HandShake(MsgBuffer& msg){
    std::string peer = connection_->GetPeerAddr().ToIpPort();
    switch(state_){
        case kHandShakeWaitC0C1:{
                if(msg.readableBytes()<1537){
                    return 1;  // 数据不够，需要等待
                }
                RTMP_TRACE << "[" << peer << "] 收到 C0C1" << std::endl;   
                auto offset =  CheckC1S1(msg.peek(),1537);   
                if(offset>=0){
                    if(is_complex_handshake_){
                    memcpy(digest_, msg.peek() + 1 + offset, SHA256_DIGEST_LENGTH);
                }
                    CreateC2S2(msg.peek()+1,1536,offset);
                    msg.retrieve(1537);
                    state_ = kHandShakePostS0S1;
                    SendC1S1();
                    RTMP_TRACE << "[" << peer << "] 发送 S0S1" << std::endl;
                    return 2;  // 发送了S0S1，等待对方响应
                }else{
                    RTMP_ERROR << "[" << peer << "] C0C1 验证失败";
                    return -1;  // 错误                
                }
        }
        case kHandShakeWaitC2:{
                if(msg.readableBytes()<1536){
                    return 1;  // 数据不够，需要等待
                }
                RTMP_TRACE << "[" << peer << "] 收到 C2" << std::endl;
                if(CheckC2S2(msg.peek(),1536)){
                    RTMP_TRACE << "[" << peer << "] 握手完成 ✓" << std::endl;                       
                    state_ = kHandShakeDoning;
                    msg.retrieve(1536);
                    return 0;  // 握手完成
                }else{
                    RTMP_ERROR << "[" << peer << "] C2 验证失败";
                    return -1;  // 错误                
                }
        }

        case kHandShakeWaitS0S1:{
                if(msg.readableBytes()<1537){
                    return 1;  // 数据不够，需要等待
                }
                RTMP_TRACE << "[" << peer << "] 收到 S0S1" << std::endl;   
                auto offset =  CheckC1S1(msg.peek(),1537);   
                if(offset>=0){
                    CreateC2S2(msg.peek()+1,1536,offset);
                    msg.retrieve(1537);
                    if(msg.readableBytes() >= 1536){
                        if(CheckC2S2(msg.peek(), 1536)){
                            msg.retrieve(1536);
                            state_ = kHandShakeDoning;
                            SendC2S2(); // 发送 C2
                            RTMP_TRACE << "[" << peer << "] 握手完成 ✓" << std::endl;
                            return 0;  // 握手完成
                        } else {
                            return 1;  // 数据不够或验证失败，需要等待
                        }
                        } else {
                            state_ = kHandShakeWaitS2; 
                            SendC2S2();                        
                            RTMP_TRACE << "[" << peer << "] 发送 C2" << std::endl;
                            return 2;  // 发送了C2，等待S2
                        }
                }else{
                    RTMP_ERROR << "[" << peer << "] S0S1 验证失败";
                    return -1;  // 错误                
                }
        }
        case kHandShakeWaitS2:
        case kHandShakePostC2: { 
            if(msg.readableBytes() < 1536){
                return 1;  // 数据不够，需要等待
            }
            if(CheckC2S2(msg.peek(), 1536)){
                RTMP_TRACE << "[" << peer << "] 收到 S2, 握手完成 ✓" << std::endl;
                state_ = kHandShakeDoning;
                msg.retrieve(1536);
                return 0;  // 握手完成
            } else {
                RTMP_ERROR << "[" << peer << "] S2 验证失败";
                return -1;  // 错误
            }
        }
        case kHandShakeDoning:
            return 0;  // 握手已完成
        default: 
            return 1;  // 未知状态，需要等待
    }
}

void RtmpHandShake::WriteComplete(){
    std::string peer = connection_->GetPeerAddr().ToIpPort();
    switch(state_){
        case kHandShakePostS0S1:{
            state_ = kHandShakePostS2;
            SendC2S2();
            RTMP_TRACE << "[" << peer << "] 发送 S2" << std::endl;
            break;            
        }

        case kHandShakePostS2:{
            state_ = kHandShakeWaitC2;
            break;            
        }

        case kHandShakePostC0C1:{
            state_ = kHandShakeWaitS0S1;
            break;            
        }

        case kHandShakePostC2: 
        case kHandShakeWaitS2: {
            if(state_ != kHandShakeDoning) {
                 state_ = kHandShakeWaitS2; // 确保状态是等待 S2
            }
            break;            
        }

        case kHandShakeDoning:{
            state_ = kHandShakeDoning;
            break;            
        } 
        default: break;       

    }
}