#pragma once

#include <cstddef>
#include <cstdint>

enum class DOIPVersion: std::uint8_t
{
    V1 = 0x01,
    V2 = 0x02,
    V3 = 0x03
};

enum class DOIPPayloadType: std::uint16_t
{
    GenericNegativeAck = 0x0000,
    RoutingActivationRequest = 0x0005,
    RoutingActivationResponse = 0x0006
};

class DOIPMessage
{
public:
    std::uint8_t* doip_message = nullptr;
    std::uint32_t doip_message_length = 0;

public:
    DOIPMessage() = default;
    DOIPMessage(std::uint32_t payload_size, DOIPPayloadType payload_type, DOIPVersion protocol_version = DOIPVersion::V3);
    ~DOIPMessage();

    std::uint8_t& operator[](std::uint32_t index) const;

};
