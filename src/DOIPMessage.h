#pragma once

#include <optional>
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
    Unsupported, //NOTE(Tiago): used to signal a payload type
                 //that we currently dont support
    GenericNegativeAck = 0x0000,
    RoutingActivationRequest = 0x0005,
    RoutingActivationResponse = 0x0006,
    AliveCheckRequest = 0x0007,
    AlivecheckResponse = 0x0008,
    DiagnosticMessage = 0x8001,
    DiagnosticMessagePositiveResponse = 0x8002,
    DiagnosticMessageNegativeResponse = 0x8003
};

enum class DOIPNegativeAckCode: std::uint8_t
{
    None,
    IncorrectPattern = 0x00,
    UnknownPayloadType = 0x01,
    MessageTooLarge = 0x02,
    OutOfMemory = 0x03,
    InvalidPayloadLength = 0x04
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

    static bool IsValidDOIPMesasage(const std::uint8_t* data, std::uint32_t data_length);
    static DOIPPayloadType DetermineDOIPMessageType(const std::uint8_t* data, std::uint32_t data_length);
    static DOIPNegativeAckCode GetNegativeAcknoledgement(const std::uint8_t* data, std::uint32_t data_length);
};

enum class DOIPActivationType: std::uint8_t
{
    Default = 0x00,
    ODB = 0x01,
    CentralSecurity = 0xE0
};

class RoutingActivationRequestMessage: public DOIPMessage
{
public:
    RoutingActivationRequestMessage() = default;
    RoutingActivationRequestMessage(std::uint16_t diagnostic_address, DOIPActivationType activation_type = DOIPActivationType::Default);
};
