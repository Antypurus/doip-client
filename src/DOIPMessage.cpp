#include "DOIPMessage.h"

#include <cstdint>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define XOR(A,B) A^B
#define BIT_INVERSE(X) XOR(X, 0xFF)

#define DOIP_HEADER_SIZE 8

DOIPMessage::DOIPMessage(std::uint32_t payload_size, DOIPPayloadType payload_type, DOIPVersion protocol_version)
{
    const std::uint8_t doip_protocol_header_size = DOIP_HEADER_SIZE;

    this->doip_message_length = doip_protocol_header_size + payload_size;
    this->doip_message = (std::uint8_t*)malloc(this->doip_message_length);
    memset(this->doip_message, 0, this->doip_message_length);

    const std::uint16_t network_order_payload_type = htons((std::uint16_t)payload_type);
    const std::uint32_t network_order_payload_size = htonl((std::uint32_t)payload_size);

    this->doip_message[0] = (std::uint8_t)protocol_version;
    this->doip_message[1] = BIT_INVERSE((std::uint8_t)protocol_version);
    memcpy(this->doip_message + 2, &network_order_payload_type, 2);
    memcpy(this->doip_message + 4, &network_order_payload_size, 4);
}

DOIPMessage::~DOIPMessage()
{
    if(this->doip_message != nullptr)
    {
        free(this->doip_message);
        this->doip_message_length = 0;
    }
}

std::uint8_t& DOIPMessage::operator[](std::uint32_t index) const
{
    return this->doip_message[DOIP_HEADER_SIZE + index];
}
