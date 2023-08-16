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
    Unsupported = 0xFF,  //NOTE(Tiago): used to signal a payload type
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

enum class DOIPRoutingActivationNegativeAck: std::uint8_t
{
    None = 0xFF,
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
    DOIPMessage(std::uint8_t* message, std::uint32_t message_length);
    DOIPMessage(std::uint32_t payload_size, DOIPPayloadType payload_type, DOIPVersion protocol_version = DOIPVersion::V3);
    ~DOIPMessage();

    std::uint32_t GetPayloadLength() const;

    static bool IsValidDOIPMesasage(const std::uint8_t* data, std::uint32_t data_length);
    static DOIPPayloadType DetermineDOIPMessageType(const std::uint8_t* data, std::uint32_t data_length);
    static DOIPRoutingActivationNegativeAck GetNegativeAcknoledgement(const std::uint8_t* data, std::uint32_t data_length);
};

enum class DOIPActivationType: std::uint8_t
{
    Default = 0x00,
    ODB = 0x01,
    CentralSecurity = 0xE0
};

enum class RoutingActivationResponse: std::uint8_t
{
    None = 0xFF,//NOTE(Tiago): used to denote errors with input data
    Reserved = 0xFE,
    DeniedUnknownSourceAddress = 0x00,
    DeniedConcurrentConnectionLimit = 0x01,
    DeniedMismatchWithConnecitonTable = 0x02,
    DeniedExistingConnection = 0x03,
    DeniedMissingAuthentication = 0x04,
    DeniedRejectedConfirmation = 0x05,
    DeniedUnsupportedActivationType = 0x06,
    DeniedTLSRequired = 0x07,
    SuccessfullActivation = 0x10,
    ActivationPendingConfirmation = 0x11
};

class RoutingActivationRequestMessage: public DOIPMessage
{
public:
    RoutingActivationRequestMessage() = default;
    RoutingActivationRequestMessage(std::uint16_t diagnostic_address, DOIPActivationType activation_type = DOIPActivationType::Default);

    static RoutingActivationResponse ParseActivationResponse(const std::uint8_t* data, std::uint32_t data_length);
};

class DoIPDiagnosticMessage: public DOIPMessage
{
public:
    DoIPDiagnosticMessage() = default;
    DoIPDiagnosticMessage(std::uint8_t* doip_diagnostic_message, std::uint32_t message_length);
    DoIPDiagnosticMessage(std::uint16_t source_diagnostic_address, std::uint16_t target_diagnostic_address, const std::uint8_t* diagnostic_message, std::uint32_t diagnostic_message_length);

    std::uint8_t& operator[](std::uint32_t index) const;
    std::uint8_t* GetDiagnosticMessage() const;
    std::uint32_t GetDiagnosticMessageLenght() const;
    std::uint16_t GetSourceAddress() const;
    std::uint16_t GetTargetAddress() const;

};

