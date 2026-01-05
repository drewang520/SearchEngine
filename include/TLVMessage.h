#ifndef _TLV_MESSAGE_H_
#define _TLV_MESSAGE_H_

#include <cstdint>
#include <string>

using std::string;
namespace  TLV
{

enum MessageType
{
    KEY_COMMANDER = 0x01,
    PAGE_SEARCHER = 0x02
};

class TLVMessage
{
public:
    TLVMessage();
    TLVMessage(uint8_t type);
    TLVMessage(uint8_t type, uint16_t length);
    TLVMessage(uint8_t type, uint16_t length, const void * value);

    TLVMessage(const TLVMessage& message);
    TLVMessage& operator=(const TLVMessage& message);
    TLVMessage(TLVMessage&& message);
    TLVMessage& operator=(TLVMessage&& message);
    ~TLVMessage();


    uint8_t getType();
    void encodeMessage();
    void decodeMessage(const string& message);
private:
    void Init(uint16_t length, unsigned char * value);
private:
    uint8_t _type; // 1字节
    uint16_t _length; // 2字节
    unsigned char * _value;
};

};

#endif
