#include "TLVMessage.h"
#include "arpa/inet.h"
#include <cstdint>
#include <string.h>

TLV::TLVMessage::TLVMessage()
: m_type(0)
, m_length(0)
, m_value(nullptr)
{

}

TLV::TLVMessage::TLVMessage(uint8_t type)
: m_type(type)
{
    Init(0, nullptr);
}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length)
: m_type(type)
{
    Init(length, nullptr);
}

TLV::TLVMessage::TLVMessage(uint8_t type, uint16_t length, const void * value)
{
    m_type = type;
    m_length = length;
    m_value =  new char[length + 1]();
    memcpy(m_value, value, length);
}

void TLV::TLVMessage::Init(uint16_t length, char * value)
{
    m_length = length;
    m_value = value;
}


TLV::TLVMessage::TLVMessage(const TLVMessage& message)
: m_type(message.m_type)
, m_length(message.m_length)
, m_value(new char[m_length + 1]())
{
    strcpy(m_value, message.m_value);
}

TLV::TLVMessage& TLV::TLVMessage::operator=(const TLVMessage& message)
{
    if (this != &message)
    {
        m_type = message.m_type;
        m_length = message.m_length;
        delete [] m_value;
        m_value = new char[m_length + 1]();
        strcpy(m_value, message.m_value);
    }
    return *this;
}

TLV::TLVMessage::TLVMessage(TLVMessage&& message)
: m_type(message.m_type)
, m_length(message.m_length)
, m_value(message.m_value)
{
    message.m_type = 0;
    message.m_length = 0;
    message.m_value = nullptr;
}

TLV::TLVMessage& TLV::TLVMessage::operator=(TLVMessage&& message)
{
    if (this != &message)
    {
        m_type = message.m_type;
        m_length = message.m_length;
        delete [] m_value;
        m_value = nullptr;
        m_value = message.m_value;

        message.m_type = 0;
        message.m_length = 0;
        message.m_value = nullptr;
    }
    return *this;
}

TLV::TLVMessage::~TLVMessage()
{
    if (m_value)
    {
        delete [] m_value;
        m_value = nullptr;
    }
}

void TLV::TLVMessage::encodeMessage(char * buf)
{
    size_t offset = 0;
    buf[offset++] = m_type;
    uint16_t value_length = htons(m_length); 
    memcpy(buf + offset, &value_length, 2);
    offset += 2;
    memcpy(buf + offset, m_value, m_length);
}

void TLV::TLVMessage::decodeMessage(const char * buf)
{
    if (3 <= strlen(buf))
    {
        m_type = buf[0];
        uint16_t nlen;
        memcpy(&nlen, buf+ 1, 2);
        m_length = ntohs(nlen);
        m_value = new char[m_length + 1]();
        memcpy(m_value, buf+ 3, m_length);                
    }
}

uint8_t TLV::TLVMessage::getType()
{
    return m_type;
}
uint16_t TLV::TLVMessage::getLength()
{
    return m_length;
}

const char * TLV::TLVMessage::getValue()
{
    return m_value;
}
