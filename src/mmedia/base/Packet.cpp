#include"mmedia/base/Packet.h"
using namespace tmms::mm;
#include <new> // 必须包含这个头文件以使用 placement new
PacketPtr Packet::NewPacket(int32_t size){
    // 1. 计算总大小
    int32_t block_size = size + sizeof(Packet);
    
    // 2. 分配原始字节流 (申请足够大的连续空间)
    char* base_mem = new char[block_size];
    
    // 3. 使用 Placement New 在这块内存上构造 Packet 对象
    // 这会自动初始化 ext_ 等智能指针，不需要 memset
    Packet * packet = new (base_mem) Packet(size); 
    
    // 4. 初始化成员变量
    // 注意：不要使用 memset 清零整个对象！
    packet->index_ = -1;
    packet->type_ = kPacketTypeUnknow;
    packet->capacity_ = size;
    // packet->ext_ 已经被构造函数初始化为空了，不需要 reset
    
    // 5. 设置自定义删除器
    // 因为是我们手动构造的，所以析构时需要：
    // a. 手动调用析构函数 (释放 ext_ 等资源)
    // b. 释放原始内存
    return PacketPtr(packet, [](Packet * p){
        if(p){
            p->~Packet();        // 显式调用析构函数
            delete[] (char*)p;   // 释放原始内存
        }
    });
}