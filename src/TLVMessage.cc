#include "TLVMessage.h"
#include <cstdint>
#include <string.h>

TLV::TLVMessage::TLVMessage()
{

}

TLV::TLVMessage::TLVMessage(uint8_t type)
: _type(type)
{
    Init(0, 0);
}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length)
: _type(type)
{
    Init(length, 0);
}

void TLV::TLVMessage::encodeMessage()
{

}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length, const void * value)
{
    _type = type;
    _length = length;
    _value =  new unsigned char[length]();
    memcpy(_value, value, length);
}

void TLV::TLVMessage::Init(uint16_t length, unsigned char * value)
{
    _length = length;
    _value = value;
}

void TLV::TLVMessage::decodeMessage(const string& message)
{
    size_t off = 0;
    if (off + 3 <= message.size())
    {
        _type = message[off];
        memcpy(&_length, &message[off + 1], 2);
        off += 3;
        memcpy(_value, &message[off], _length);                
    }
}

uint8_t TLV::TLVMessage::getType()
{
    return _type;
}

TLV::TLVMessage::TLVMessage(const TLVMessage& message)
: _type(message._type)
{
    Init(message._length, message._value);
}

TLV::TLVMessage& TLV::TLVMessage::operator=(const TLVMessage& message)
{
    _type = message._type;
    _length = message._length;
    _value = message._value;
    return *this;
}

TLV::TLVMessage::TLVMessage(TLVMessage&& message)
: _type(message._type)
{
    Init(message._length, message._value);
}

TLV::TLVMessage& TLV::TLVMessage::operator=(TLVMessage&& message)
{
    if (this != &message)
    {
        _type = message._type;
        _length = message._length;
        _value = message._value;
    }
    return *this;
}

TLV::TLVMessage::~TLVMessage()
{
    if (_value)
    {
        delete [] _value;
        _value = nullptr;
    }
}

