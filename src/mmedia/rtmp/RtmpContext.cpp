#include "mmedia/rtmp/RtmpContext.h"
#include "mmedia/rtmp/amf/AMFObject.h"
#include <vector>
#include <algorithm> // for std::transform
#include "base/StringUtils.h"

using namespace tmms::mm;

RtmpContext::RtmpContext(const TcpConnectionPtr & con, RtmpHandler* handler, bool client)
    : connection_(con), rtmp_handler_(handler), hand_shake_(con, client) {
    
    // 初始化命令回调
    commands_["connect"] = std::bind(&RtmpContext::HandleConnect, this, std::placeholders::_1);
    commands_["createstream"] = std::bind(&RtmpContext::HandleCreateStream, this, std::placeholders::_1);
    commands_["_result"] = std::bind(&RtmpContext::HandleResult, this, std::placeholders::_1);
    commands_["_error"] = std::bind(&RtmpContext::HandleError, this, std::placeholders::_1);
    commands_["play"] = std::bind(&RtmpContext::HandlePlay, this, std::placeholders::_1);
    commands_["publish"] = std::bind(&RtmpContext::HandlePublish, this, std::placeholders::_1);
    
    out_current_ = out_buffer_;
    
    // 显式初始化关键参数，防止意外
    in_chunk_size_ = 128;
    out_chunk_size_ = 4096;
    ack_size_ = 2500000;
}

int32_t RtmpContext::Parse(MsgBuffer & buf) {
    int32_t ret = -1;
    if (state_ == kRtmpHandShake || state_ == kRtmpWaitingDone) {
        ret = hand_shake_.HandShake(buf);
        if (ret == 0) {
            state_ = kRtmpMessage;
            if(is_client){
                SendConnect();
            }
            if (buf.readableBytes() > 0) {
                return Parse(buf);
            }
        } else if (ret == -1) {
            RTMP_ERROR << "host:" << connection_->GetPeerAddr().ToIpPort() << " rtmp handshake error";
        } else if (ret == 2) {
            state_ = kRtmpWaitingDone;
        } else if (ret == 1) {
            if (state_ == kRtmpHandShake) {
                //握手中间状态
            }
        }
    } else if (state_ == kRtmpMessage) {
        while (buf.readableBytes() > 0) {
            auto parsed_len = MessagePrase(buf);
            if (parsed_len < 0) {
                return -1;
            }
            if (parsed_len == 0) {
               
                break;
            }

        }
        last_left_ = buf.readableBytes();
        return 0;
    }
    return ret;
}

void RtmpContext::StartHandShake() {
    hand_shake_.Start();
}

void RtmpContext::WriteComplete() {
    if (state_ == kRtmpHandShake) {
        hand_shake_.WriteComplete();
    }else if(state_ == kRtmpWaitingDone){
        state_ = kRtmpMessage;
            if(is_client){
                SendConnect();
            }
        }
    else if (state_ == kRtmpMessage) {
        CheckAndSend();
    }
}

int32_t RtmpContext::MessagePrase(MsgBuffer & buf) {
    uint8_t fmt = 0;
    uint32_t csid, msg_len = 0, msg_sid, msg_timestamp = 0;
    uint8_t msg_type = 0;
    uint32_t total_bytes = buf.readableBytes();
    int32_t parse = 0;

    if (total_bytes > 0) { 
        const char * pos = buf.peek();
        parse = 0;
        
        if(total_bytes < 1) return 0; 
        
        fmt = (*pos >> 6) & 0x03;
        csid = *pos & 0x3f;
        parse++;

        if (csid == 0) {
            if (total_bytes < 2) return 0; 
            csid = 64;
            csid += *((uint8_t*)(pos + parse));
            parse++;
        } else if (csid == 1) {
            if (total_bytes < 3) return 0; 
            csid = 64;
            csid += *((uint8_t*)(pos + parse));
            parse++;
            csid += *((uint8_t*)(pos + parse)) * 256;
            parse++;
        }

        int size = total_bytes - parse;

        if (size < 0 || (fmt == 0 && size < 11) || (fmt == 1 && size < 7) || (fmt == 2 && size < 3)) {
          
            return 0; 
        }

        msg_len = 0;
        msg_type = 0;
        msg_timestamp = 0;
        msg_sid = 0;
        int32_t ts = 0;
        RtmpMsgHeaderPtr &prve = in_message_headers_[csid];
        if (!prve) {
            prve = std::make_shared<RtmpMsgHeader>();
        }

        if (fmt == kRtmpFmt0) {
            ts = BytesReader::ReadUint24T(pos + parse);
            parse += 3;
            in_deltas_[csid] = 0;
            msg_timestamp = ts;
            msg_len = BytesReader::ReadUint24T(pos + parse);
            parse += 3;
            msg_type = BytesReader::ReadUint8T(pos + parse);
            parse += 1;
            memcpy(&msg_sid, pos + parse, 4);
            parse += 4;
        } else if (fmt == kRtmpFmt1) {
            ts = BytesReader::ReadUint24T(pos + parse);
            parse += 3;
            in_deltas_[csid] = ts;
            msg_timestamp = ts + prve->timestamp;
            msg_len = BytesReader::ReadUint24T(pos + parse);
            parse += 3;
            msg_type = BytesReader::ReadUint8T(pos + parse);
            parse += 1;
            msg_sid = prve->msg_sid;
        } else if (fmt == kRtmpFmt2) {
            ts = BytesReader::ReadUint24T(pos + parse);
            parse += 3;
            in_deltas_[csid] = ts;
            msg_timestamp = ts + prve->timestamp;
            msg_len = prve->msg_len;
            msg_type = prve->msg_type;
            msg_sid = prve->msg_sid;
        } else if (fmt == kRtmpFmt3) {
            ts = in_deltas_[csid]; 
            msg_timestamp = ts + prve->timestamp;
            msg_len = prve->msg_len;
            msg_type = prve->msg_type;
            msg_sid = prve->msg_sid;
        }

        bool ext = (ts == 0xffffff);
        if (fmt == kRtmpFmt3) {
            if(in_ext_.find(csid) != in_ext_.end()) {
                 ext = in_ext_[csid];
            } else {
                 ext = false; 
            }
        } else {
             in_ext_[csid] = ext; // 记录状态供 fmt3 使用
        }
        
        if (ext) {
            if (total_bytes - parse < 4) {
                return 0; // 等待扩展时间戳
            }
            msg_timestamp = BytesReader::ReadUint32T(pos + parse);
            parse += 4;

            if (fmt != kRtmpFmt0) {

                msg_timestamp = ts + prve->timestamp; 
                in_deltas_[csid] = ts;
            }
        }

        PacketPtr & packet = in_packets_[csid];
        if (!packet) {
            packet = Packet::NewPacket(msg_len);
        }
        RtmpMsgHeaderPtr header = packet->Ext<RtmpMsgHeader>();
        if (!header) {
            header = std::make_shared<RtmpMsgHeader>();
            packet->SetExt(header);
        }
        header->msg_len = msg_len;
        header->cs_id = csid;
        header->msg_sid = msg_sid;
        header->msg_type = msg_type;
        header->timestamp = msg_timestamp;

        int byte = std::min(packet->Space(), in_chunk_size_);
        
       
        if (total_bytes - parse < byte) {
           
            return 0; 
        }

        const char * body = packet->PacketSize() + packet->Data();
        memcpy((void*)body, pos + parse, byte);
        packet->UpdataPacketSize(byte);
        parse += byte;

        in_bytes += parse;
        SetBytesRevc();
        buf.retrieve(parse);

        prve->cs_id = csid;
        prve->msg_len = msg_len;
        prve->msg_type = msg_type;
        prve->msg_sid = msg_sid;
        prve->timestamp = msg_timestamp;

        if (packet->Space() == 0) {
            packet->SetPacketType(msg_type);
            packet->SetTimestamp(msg_timestamp);
            RTMP_TRACE << "complete packet type=" << int(msg_type) << " len=" << msg_len << " csid=" << csid << std::endl;
            MessageComplete(std::move(packet));
            packet.reset();
        }
    }
    return parse;
}

void RtmpContext::MessageComplete(PacketPtr&&data) {
    auto type = data->PacketType();

    switch (type) {
    case kRtmpMsgTypeChunkSize: {
        HandleChunkSize(data);
        break;
    }
    case kRtmpMsgTypeBytesRead: {
        RTMP_TRACE << "message bytes read recv:" << std::endl;
        break;
    }
    case kRtmpMsgTypeUserControl: {
        HandleUserMessage(data);
        break;
    }
    case kRtmpMsgTypeWindowACKSize: {
        HandleAckWindowSize(data);
        break;
    }
    case kRtmpMsgTypeAMF3Message: {
        HandleAMFCommeand(data, true);
        break;
    }
    case kRtmpMsgTypeAMFMessage: {
        HandleAMFCommeand(data);
        break;
    }
    case kRtmpMsgTypeAMF3Meta:
    case kRtmpMsgTypeAMFMeta:
    case kRtmpMsgTypeAudio:
    case kRtmpMsgTypeVideo:
    {
        SetPacketType(data);
        if (rtmp_handler_) {
            rtmp_handler_->OnRecv(connection_, data);
        }
        break;
    }
    default:
        RTMP_TRACE << "no support message type " << type << " len:" << data->PacketSize() << " host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;;
        break;
    }
}

void RtmpContext::Send() {
    if (sending_ || out_wait_queue_.empty()) {
        return;
    }
    sending_ = true;
    for (int i = 0; i < 10; i++) {
        if (out_wait_queue_.empty()) {
            break;
        }
        PacketPtr packet = std::move(out_wait_queue_.front());
        out_wait_queue_.pop_front();
        BuildChunk(packet);
    }
    connection_->Send(sending_bufs_);
}

void RtmpContext::SetPacketType(PacketPtr &packet) {
    // 这里的逻辑有点冗余，但保留原样
    if (packet->PacketType() == kRtmpMsgTypeAudio) {
        packet->SetPacketType(kRtmpMsgTypeAudio);
    } else if (packet->PacketType() == kRtmpMsgTypeVideo) {
        packet->SetPacketType(kRtmpMsgTypeVideo);
    } else if (packet->PacketType() == kRtmpMsgTypeMetadata) {
        packet->SetPacketType(kRtmpMsgTypeMetadata);
    } else if (packet->PacketType() == kRtmpMsgTypeAMF3Meta) {
        packet->SetPacketType(kRtmpMsgTypeAMF3Meta);
    }
}

void RtmpContext::CheckAndSend() {
    sending_ = false;
    out_current_ = out_buffer_;
    sending_bufs_.clear();
    out_sending_packet_.clear();
    if (!out_wait_queue_.empty()) {
        Send();
    } else {
        if (rtmp_handler_) {
            rtmp_handler_->OnActive(connection_);
        }
    }
}

bool RtmpContext::Ready() const {
    return sending_;
}

void RtmpContext::PushOutPacket(PacketPtr && packet) {
    out_wait_queue_.emplace_back(std::move(packet));
    Send();
}


bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0){
    RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
    if(h){
        out_sending_packet_.emplace_back(packet);
        RtmpMsgHeaderPtr & prve = out_message_headers_[h->cs_id];
        
        // 【修复点 1】必须检查 msg_type 是否相同，否则不能使用 fmt=2 (仅包含时间戳增量)
        bool use_deltas = !fmt0 && prve && timestamp >= prve->timestamp 
                          && prve->msg_sid == h->msg_sid 
                          && prve->msg_type == h->msg_type; // <--- 加上这句

        if(!prve){
            prve = std::make_shared<RtmpMsgHeader>();
        }
        int fmt = kRtmpFmt0;
        timestamp -= prve->timestamp;
        
        if(use_deltas){
            if(prve->msg_len == h->msg_len){ // msg_type 和 msg_sid 在上面已经检查过了
                fmt = kRtmpFmt2;
                if(timestamp == out_deltas_[h->cs_id]){
                    fmt = kRtmpFmt3;
                }
            }            
        }
        
        char * p = out_current_;
        // 写入 Chunk Basic Header
        if(h->cs_id < 64){
            *p++ = (char)((fmt << 6) | (h->cs_id));
        }else if(h->cs_id < (64 + 256)){
            *p++ = (char)((fmt << 6) | 0);
            *p++ = (char)(h->cs_id - 64);
        }else{
            *p++ = (char)((fmt << 6) | 1);
            uint16_t cs = h->cs_id - 64;
            memcpy(p, &cs, sizeof(cs));
            p += sizeof(uint16_t);
        }

        auto ts = timestamp;
        if(ts >= 0xffffff){
            ts = 0xffffff;
        }

        // 写入 Chunk Message Header
        if(fmt == kRtmpFmt0){
            p += BytesWriter::WriteUint24T(p, ts);
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            p += BytesWriter::WriteUint8T(p, h->msg_type);
            memcpy(p, &h->msg_sid, 4); // msg_sid 是小端序
            p += 4;
            out_deltas_[h->cs_id] = 0;
        }else if(fmt == kRtmpFmt1){
            p += BytesWriter::WriteUint24T(p, ts);
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            p += BytesWriter::WriteUint8T(p, h->msg_type);
            out_deltas_[h->cs_id] = timestamp;            
        }else if(fmt == kRtmpFmt2){
            p += BytesWriter::WriteUint24T(p, ts);
            out_deltas_[h->cs_id] = timestamp;               
        }

        if(ts == 0xffffff){
            BytesWriter::WriteUint32T(p, timestamp); // 扩展时间戳是大端序
            p += 4;
        }
        
        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;

        // 更新状态
        prve->cs_id = h->cs_id;
        prve->msg_len = h->msg_len;
        prve->msg_sid = h->msg_sid;
        prve->msg_type = h->msg_type;
        if(fmt == kRtmpFmt0){
            prve->timestamp = timestamp; // fmt0 timestamp 是绝对值
        }else{
            prve->timestamp += timestamp; // 其他是增量           
        }

        // 写入 Body (分块)
        const char * body = packet->Data();
        int32_t bytes_parsed = 0;
        while(true){
            const char * chunk = body + bytes_parsed;
            int32_t size = h->msg_len - bytes_parsed;
            size = std::min(size, out_chunk_size_);
            
            BufferNodePtr nbody = std::make_shared<BufferNode>((void*)chunk, size);   
            sending_bufs_.emplace_back(std::move(nbody));   
            bytes_parsed += size; 
            
            if(bytes_parsed < h->msg_len){
                // 需要发送下一个块，先检查缓冲区空间
                if(out_current_ - out_buffer_ > 4000){ // 留点余量
                    // 实际生产环境这里应该 flush 或重置 buffer，这里仅打印错误防止越界
                    RTMP_ERROR << "out buffer overflow risk";
                    break;
                }
                char * p = out_current_;
                // 下一个块使用 fmt=3 (只有头)
                if(h->cs_id < 64){
                    *p++ = (char)((0xc3) | (h->cs_id)); // fmt=3 (0x3 << 6) | csid
                }else if(h->cs_id < (64 + 256)){
                    *p++ = (char)((0xc3) | 0);
                    *p++ = (char)(h->cs_id - 64);
                }else{
                    *p++ = (char)((0xc3) | 1);
                    uint16_t cs = h->cs_id - 64;
                    memcpy(p, &cs, sizeof(cs));
                    p += sizeof(uint16_t);
                }

                if(ts == 0xffffff){
                     BytesWriter::WriteUint32T(p, timestamp);
                     p += 4;
                }
                
                BufferNodePtr nnext_header = std::make_shared<BufferNode>(out_current_, p - out_current_);
                sending_bufs_.emplace_back(std::move(nnext_header));
                out_current_ = p;                
            }else{
                break;
            }
        }
        return true;
    }
    return false;
}

void RtmpContext::SendSetChunkSize() {
    PacketPtr packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    if (header) {
        header->cs_id = kRtmpCSIDCommand;
        header->msg_len = 0;
        header->msg_sid = kRtmpMsID0;
        header->msg_type = kRtmpMsgTypeChunkSize;
        header->timestamp = 0;
        packet->SetExt(header);
    }
    char * body = packet->Data();
    header->msg_len = BytesWriter::WriteUint32T(body, out_chunk_size_);
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send chunk size:" << header->msg_len << std::endl;
    PushOutPacket(std::move(packet));
}

void RtmpContext::SendAckWindowSize() {
    PacketPtr packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    if (header) {
        header->cs_id = kRtmpCSIDCommand;
        header->msg_len = 0;
        header->msg_sid = kRtmpMsID0;
        header->msg_type = kRtmpMsgTypeWindowACKSize;
        header->timestamp = 0;
        packet->SetExt(header);
    }
    char * body = packet->Data();
    header->msg_len = BytesWriter::WriteUint32T(body, ack_size_);
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send ack size:" << header->msg_len << " to host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
    PushOutPacket(std::move(packet));
}

void RtmpContext::SendPeerBandWidth() {
    PacketPtr packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    if (header) {
        header->cs_id = kRtmpCSIDCommand;
        header->msg_len = 0;
        header->msg_sid = kRtmpMsID0;
        header->msg_type = kRtmpMsgTypeSetPeerBW;
        header->timestamp = 0;
        packet->SetExt(header);
    }
    char * body = packet->Data();
    body += BytesWriter::WriteUint32T(body, ack_size_);
    *body++ = 0x02; // Limit Type: Dynamic
    header->msg_len = 5;
    packet->SetPacketSize(5);
    RTMP_TRACE << "send band width:" << header->msg_len << std::endl;
    PushOutPacket(std::move(packet));
}

void RtmpContext::SetBytesRevc() {
    if (in_bytes >= ack_size_) {
        PacketPtr packet = Packet::NewPacket(64);
        RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
        if (header) {
            header->cs_id = kRtmpCSIDCommand;
            header->msg_len = 0;
            header->msg_sid = kRtmpMsID0;
            header->msg_type = kRtmpMsgTypeBytesRead;
            header->timestamp = 0;
            packet->SetExt(header);
        }
        char * body = packet->Data();
        header->msg_len = BytesWriter::WriteUint32T(body, in_bytes);
        packet->SetPacketSize(header->msg_len);
        PushOutPacket(std::move(packet));
    }
}

void RtmpContext::SetUserCtlMessage(short nType, int32_t value_1, int32_t value_2) {
    PacketPtr packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    if (header) {
        header->cs_id = kRtmpCSIDCommand;
        header->msg_len = 0;
        header->msg_sid = kRtmpMsID0;
        header->msg_type = kRtmpMsgTypeUserControl;
        header->timestamp = 0;
        packet->SetExt(header);
    }
    char * body = packet->Data();
    char * p = body;
    p += BytesWriter::WriteUint16T(body, nType);
    p += BytesWriter::WriteUint32T(body, value_1);
    if (nType == kRtmpEventTypeSetBufferLength) {
        p += BytesWriter::WriteUint32T(body, value_2);
    }
    packet->SetPacketSize(p - body);
    RTMP_TRACE << "send user ctl type:" << nType << " value1:" << value_1 << " value2:" << value_2 << std::endl;
    PushOutPacket(std::move(packet));
}

void RtmpContext::HandleChunkSize(PacketPtr &packet) {
    if (packet->PacketSize() >= 4) {
        auto size = BytesReader::ReadUint32T(packet->Data());
        RTMP_TRACE << "read ChunkSize:" << size << std::endl;
        in_chunk_size_ = size;
    }
}

void RtmpContext::HandleAckWindowSize(PacketPtr &packet) {
    // 【修复】之前错误地赋值给了 in_chunk_size_
    if (packet->PacketSize() >= 4) {
        auto size = BytesReader::ReadUint32T(packet->Data());
        RTMP_TRACE << "read AckWindowSize:" << size << " host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
        ack_size_ = size; // 修正点：更新 ack 窗口，而不是 chunk size
    } else {
        RTMP_TRACE << "invalid AckWindowSize:" << packet->PacketSize() << " host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
    }
}

void RtmpContext::HandleUserMessage(PacketPtr &packet) {
    auto msg_len = packet->PacketSize();
    char * body = packet->Data();
    if(msg_len < 6) return; // 简单防护

    auto type = BytesReader::ReadUint16T(body);
    auto value = BytesReader::ReadUint32T(body + 2);
    RTMP_TRACE << "recv user ctl type:" << type << " value:" << value << " host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
    
    switch (type) {
    case kRtmpEventTypePingRequest:
    {
        SetUserCtlMessage(kRtmpEventTypePingResponse, value, 0);
        break;
    }
    default:
        break;
    }
}

void RtmpContext::HandleAMFCommeand(PacketPtr &packet, bool afm3) {
    const char * body = packet->Data();
    int32_t msg_len = packet->PacketSize();
    if (afm3) {
        body += 1;
        msg_len -= 1;
    }
    AMFObject obj;
    if (obj.Decode(body, msg_len) < 0) {
        RTMP_TRACE << "afm decode failed" << " to host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
    }
    AMFAnyPtr method_any = obj.Property(0);
    if (!method_any) {
        return;
    }
    std::string method = method_any->String();
    std::transform(method.begin(), method.end(), method.begin(), ::tolower);
    
    auto iter = commands_.find(method);
    if (iter == commands_.end()) {
        RTMP_TRACE << "no support command:" << method_any->String() << " to host:" << connection_->GetPeerAddr().ToIpPort() << std::endl;
        return;
    }
    iter->second(obj);
}

void RtmpContext::SendConnect() {
    SendSetChunkSize();
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 0;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "connect");
    p += AMFAny::EncodeNumber(p, 1.0);
    *p++ = kAMFObject;

    p += AMFAny::EncodeNamedString(p, "app", app_);
    p += AMFAny::EncodeNamedString(p, "tcUrl", tc_url_);
    // 补齐常见字段，兼容 nginx/flash 期望
    p += AMFAny::EncodeNamedString(p, "flashVer", "LNX 9,0,124,2");
    p += AMFAny::EncodeNamedString(p, "swfUrl", "");
    p += AMFAny::EncodeNamedString(p, "pageUrl", "");
    p += AMFAny::EncodeNamedBoolean(p, "fpad", false);
    p += AMFAny::EncodeNamedNumber(p, "capabilities", 31.0);
    p += AMFAny::EncodeNamedNumber(p, "audioCodecs", 1639.0);
    p += AMFAny::EncodeNamedNumber(p, "videoCodecs", 252.0);
    p += AMFAny::EncodeNamedNumber(p, "videoFunction", 1.0);
    p += AMFAny::EncodeNamedNumber(p, "objectEncoding", 0.0);

    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send connect msg_len:" << header->msg_len << " to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::HandleConnect(AMFObject &obj) {
    auto amf3 = false;

    tc_url_ = obj.Property("tcUrl")->String();
    AMFObjectPtr sub_obj = obj.Property(2)->Object();
    AMFAnyPtr enc = sub_obj->Property("objectEncoding");
        if(enc){  // 只有当属性存在时才访问
            amf3 = enc->Number() == 3.0;
        } else {
            amf3 = false;
        }    
    if (sub_obj) {
        app_ = sub_obj->Property("app")->String();
    }

    RTMP_TRACE << "recv tc_url_:" << tc_url_ << " app:" << app_ <<" amf3"<<amf3;
    SendSetChunkSize();
    SendAckWindowSize();
    SendPeerBandWidth();
    
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 0;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "_result");
    p += AMFAny::EncodeNumber(p, 1.0);
    *p++ = kAMFObject;

    p += AMFAny::EncodeNamedString(p, "fmsVer", "FMS/3,0,1,123");
    p += AMFAny::EncodeNamedNumber(p, "capabilities", 31);

    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    *p++ = kAMFObject;

    p += AMFAny::EncodeNamedString(p, "level", "status");
    p += AMFAny::EncodeNamedString(p, "code", "NetConnection.Connect.Success");
    p += AMFAny::EncodeNamedString(p, "description", "Connection succeeded.");
    p += AMFAny::EncodeNamedNumber(p, "objectEncoding", amf3 ? 3.0 : 0);

    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "connect result msg_len:" << header->msg_len << " to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::CreateStream() {
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 0;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "createStream");
    p += AMFAny::EncodeNumber(p, 4.0);
    *p++ = kAMFNull;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send CreateStream msg_len:" << header->msg_len << " to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::HandleCreateStream(AMFObject &obj) {
    auto tran_id = obj.Property(1)->Number();

    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 0;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "_result");
    p += AMFAny::EncodeNumber(p, tran_id);
    *p++ = kAMFNull;

    p += AMFAny::EncodeNumber(p, kRtmpMsID1);

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "create stream result msg_len:" << header->msg_len << " to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::SendStatus(const std::string & level, const std::string & code, const std::string & description) {
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 1;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "onStatus");
    p += AMFAny::EncodeNumber(p, 0);
    *p++ = kAMFNull;
    *p++ = kAMFObject;
    p += AMFAny::EncodeNamedString(p, "level", level);
    p += AMFAny::EncodeNamedString(p, "code", code);
    p += AMFAny::EncodeNamedString(p, "description", description);
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send status level:" << level << " code:" << code << " description" << description << " to host:" << connection_->GetPeerAddr().ToIpPort()<<std::endl;
    PushOutPacket(std::move(packet));
}

void RtmpContext::SendPlay() {
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 1;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "play");
    p += AMFAny::EncodeNumber(p, 0);
    *p++ = kAMFNull;
    p += AMFAny::EncodeString(p, name_);
    p += AMFAny::EncodeNumber(p, -1000.0);

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send Play name_:" << name_ << " to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::HandlePlay(AMFObject &obj) {
    auto tran_id = obj.Property(1)->Number();
    name_ = obj.Property(3)->String();
    ParseNameAndUrl();
    RTMP_TRACE << "recv Play session_name_:" << session_name_
        << " parma_:" << parma_
        << " host:" << connection_->GetPeerAddr().ToIpPort();
    SetUserCtlMessage(kRtmpEventTypeStreamBegin , 1, 0);
    isplaying = true;
    SendStatus("status", "NetStream.Play.Start", "Start Play");
    if (rtmp_handler_) {
        rtmp_handler_->OnPlay(connection_, session_name_, parma_);
    }
}

void RtmpContext::ParseNameAndUrl(){
    // tc_url_ 示例: rtmp://host[:port]/app/stream?query
    std::string url = tc_url_;
    const std::string prefix = "rtmp://";
    if(url.rfind(prefix, 0) == 0){
        url = url.substr(prefix.size());
    }
    // 去掉前导 '/'
    while(!url.empty() && url[0] == '/'){
        url.erase(0,1);
    }

    // 拆 host[:port] 与 path
    auto slash_pos = url.find('/');
    std::string hostport = (slash_pos == std::string::npos) ? url : url.substr(0, slash_pos);
    std::string path     = (slash_pos == std::string::npos) ? ""  : url.substr(slash_pos + 1);

    // 处理 query 参数
    auto qpos = path.find('?');
    if(qpos != std::string::npos){
        parma_ = path.substr(qpos + 1);
        path   = path.substr(0, qpos);
    }else{
        parma_.clear();
    }

    // 拆 app/name
    app_.clear();
    name_.clear();
    auto split_pos = path.find('/');
    if(split_pos != std::string::npos){
        app_  = path.substr(0, split_pos);
        name_ = path.substr(split_pos + 1);
    }else if(!path.empty()){
        app_ = path;
    }

    // 提取 domain（去掉端口）
    std::string domain = hostport;
    auto colon_pos = domain.find(':');
    if(colon_pos != std::string::npos){
        domain = domain.substr(0, colon_pos);
    }

    session_name_.clear();
    session_name_ += domain;
    if(!app_.empty()){
        session_name_ += "/";
        session_name_ += app_;
    }
    if(!name_.empty()){
        session_name_ += "/";
        session_name_ += name_;
    }

    RTMP_TRACE <<"session_name_:"<<session_name_<<" parma_:"<<parma_<<" host:"<<connection_->GetPeerAddr().ToIpPort();
}

void RtmpContext::SendPublish() {
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage;
    header->msg_sid = 1;
    packet->SetExt(header);

    char * body = packet->Data();
    char * p = body;

    p += AMFAny::EncodeString(p, "publish");
    p += AMFAny::EncodeNumber(p, 5);
    *p++ = kAMFNull;
    p += AMFAny::EncodeString(p, name_);
    p += AMFAny::EncodeString(p, "live");

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send Publish name_:" << name_ << " msg_len:" << header->msg_len << "to host:" << connection_->GetPeerAddr().ToIpPort();
    PushOutPacket(std::move(packet));
}

void RtmpContext::HandlePublish(AMFObject &obj) {
    auto tran_id = obj.Property(1)->Number();
    name_ = obj.Property(3)->String();
    isplaying = false;

    SendStatus("status", "NetStream.Publish.Start", "Start Publishing");
    
    if (rtmp_handler_) {
        rtmp_handler_->OnPublish(connection_, session_name_, parma_);
    }
}

void RtmpContext::HandleResult(AMFObject &obj) {
    auto id = obj.Property(1)->Number();
    RTMP_TRACE << "HandleResult id:" << id << " host:" << connection_->GetPeerAddr().ToIpPort();
    if (id == 1) {
        CreateStream();
    } else if (id == 4) {
        if (isplaying) {
            SendPlay();
        } else {
            SendPublish();
        }
    }
}

void RtmpContext::HandleError(AMFObject &obj) {
    std::string description;
    if (obj.Property(3) && obj.Property(3)->Object()) {
         description = obj.Property(3)->Object()->String();
    }
    RTMP_TRACE << "HandleError description:" << description << " host:" << connection_->GetPeerAddr().ToIpPort();
    connection_->ForceClose();
}

void RtmpContext::Play(const std::string &url){
    is_client = true;
    isplaying = true; 
    tc_url_ =  url;
    ParseNameAndUrl();
}

void RtmpContext::Publish(const std::string &url){
    is_client = true;
    isplaying = false; 
    tc_url_ =  url;   
    ParseNameAndUrl();
}