#include "TLVMessage.h"
#include "arpa/inet.h"
#include <cstdint>
#include <string.h>

TLV::TLVMessage::TLVMessage()
: _type(0)
, _length(0)
, _value(nullptr)
{

}

TLV::TLVMessage::TLVMessage(uint8_t type)
: _type(type)
{
    Init(0, nullptr);
}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length)
: _type(type)
{
    Init(length, nullptr);
}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length, const void * value)
{
    _type = type;
    _length = length;
    _value =  new char[length + 1]();
    memcpy(_value, value, length);
}

void TLV::TLVMessage::Init(uint16_t length, char * value)
{
    _length = length;
    _value = value;
}


TLV::TLVMessage::TLVMessage(const TLVMessage& message)
: _type(message._type)
, _length(message._length)
, _value(new char[_length + 1]())
{
    strcpy(_value, message._value);
}

TLV::TLVMessage& TLV::TLVMessage::operator=(const TLVMessage& message)
{
    if (this != &message)
    {
        _type = message._type;
        _length = message._length;
        delete [] _value;
        _value = new char[_length + 1]();
        strcpy(_value, message._value);
    }
    return *this;
}

TLV::TLVMessage::TLVMessage(TLVMessage&& message)
: _type(message._type)
, _length(message._length)
, _value(message._value)
{
    message._type = 0;
    message._length = 0;
    message._value = nullptr;
}

TLV::TLVMessage& TLV::TLVMessage::operator=(TLVMessage&& message)
{
    if (this != &message)
    {
        _type = message._type;
        _length = message._length;
        delete [] _value;
        _value = nullptr;
        _value = message._value;

        message._type = 0;
        message._length = 0;
        message._value = nullptr;
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

void TLV::TLVMessage::encodeMessage(char * buf)
{
    size_t offset = 0;
    buf[offset++] = _type;
    uint16_t value_length = htons(_length); 
    memcpy(buf + offset, &value_length, 2);
    offset += 2;
    memcpy(buf + offset, _value, _length);
}

void TLV::TLVMessage::decodeMessage(const char * buf)
{
    if (3 <= strlen(buf))
    {
        _type = buf[0];
        uint16_t nlen;
        memcpy(&nlen, buf+ 1, 2);
        _length = ntohs(nlen);
        _value = new char[_length + 1]();
        memcpy(_value, buf+ 3, _length);                
    }
}

uint8_t TLV::TLVMessage::getType()
{
    return _type;
}
uint16_t TLV::TLVMessage::getLength()
{
    return _length;
}

const char * TLV::TLVMessage::getValue()
{
    return _value;
}
