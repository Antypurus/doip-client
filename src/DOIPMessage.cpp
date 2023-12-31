#include "DOIPMessage.h"

#include <cstdint>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>

#define XOR(A,B) (A^B)
#define BIT_INVERSE(X) XOR(X, 0xFF)

#define DOIP_HEADER_SIZE 8
#define DOIP_PROTOCOL_VERSION_OFFSET 0
#define DOIP_INVERSE_PROTOCOL_VERSION_OFFSET 1
#define DOIP_PAYLOAD_TYPE_OFFSET 2
#define DOIP_PAYLOAD_LENGTH_OFFSET 4
#define DOIP_HEADER_PAYLOAD_TYPE_SIZE 2
#define DOIP_HEADER_PAYLOAD_LENGTH_SIZE 4

#define DOIP_NACK_PAYLOAD_SIZE 1
#define DOIP_NACK_CODE_OFFSET (DOIP_HEADER_SIZE + DOIP_NACK_PAYLOAD_SIZE - 1)

#define DOIP_ROUTING_ACTIVATION_REQUEST_PAYLOAD_SIZE 11
#define DOIP_ROUTING_ACTIVATION_REQUEST_DIAGNOSTIC_ADDRESS_RELATIVE_OFFSET 0
#define DOIP_ROUTING_ACTIVATION_REQUEST_ACTIVATION_TYPE_RELATIVE_OFFSET 2
#define DOIP_ROUTING_ACTIVATION_REQUEST_DIAGNOSTIC_ADDRESS_SIZE 2

#define DOIP_ROUTING_ACTIVATION_RESPONSE_PAYLOAD_SIZE 13
#define DOIP_ROUTING_ACTIVATION_RESPONSE_CLIENT_ADDRESS_RELATIVE_OFFSET 0
#define DOIP_ROUTING_ACTIVATION_RESPONSE_SERVER_ADDRESS_RELATIVE_OFFSET 2
#define DOIP_ROUTING_ACTIVATION_RESPONSE_ACTIVATION_RESPONSE_OFFSET 4
#define DOIP_ROUTING_ACTIVATION_RESPONSE_CLIENT_ADDRESS_SIZE 2
#define DOIP_ROUTING_ACTIVATION_RESPONSE_SERVER_ADDRESS_SIZE 2

#define DOIP_DIAGNOSTIC_MESSAGE_BASE_PAYLOAD_SIZE 4
#define DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_RELATIVE_OFFSET 0
#define DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_RELATIVE_OFFSET 2
#define DOIP_DIAGNOSTIC_MESSAGE_DATA_RELATIVE_OFFSET 4
#define DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_SIZE 2
#define DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_SIZE 2

DOIPMessage::DOIPMessage(std::uint32_t payload_size, DOIPPayloadType payload_type, DOIPVersion protocol_version)
{
    this->doip_message_length = DOIP_HEADER_SIZE + payload_size;
    this->doip_message = (std::uint8_t*)malloc(this->doip_message_length);
    memset(this->doip_message, 0, this->doip_message_length);

    const std::uint16_t network_order_payload_type = htons((std::uint16_t)payload_type);
    const std::uint32_t network_order_payload_size = htonl((std::uint32_t)payload_size);

    this->doip_message[0] = (std::uint8_t)protocol_version;
    this->doip_message[1] = BIT_INVERSE((std::uint8_t)protocol_version);
    memcpy(this->doip_message + DOIP_PAYLOAD_TYPE_OFFSET, &network_order_payload_type, DOIP_HEADER_PAYLOAD_TYPE_SIZE);
    memcpy(this->doip_message + DOIP_PAYLOAD_LENGTH_OFFSET, &network_order_payload_size, DOIP_HEADER_PAYLOAD_LENGTH_SIZE);
}

DOIPMessage::DOIPMessage(std::uint8_t* message, std::uint32_t message_length)
{
    this->doip_message = message;
    this->doip_message_length = message_length;
}

DOIPMessage::~DOIPMessage()
{
    if(this->doip_message != nullptr)
    {
        free(this->doip_message);
        this->doip_message_length = 0;
    }
}

std::uint32_t DOIPMessage::GetPayloadLength() const
{
    return this->doip_message[DOIP_PAYLOAD_LENGTH_OFFSET];
}

bool DOIPMessage::IsValidDOIPMesasage(const std::uint8_t* data, std::uint32_t data_length)
{
    if(data_length < DOIP_HEADER_SIZE) return false;

    //NOTE(Tiago): at the moment no protocol version validation is done, as it seems that new protocol
    //versions do not change the core format of the protocol.
    const std::uint8_t protocol_version = data[DOIP_PROTOCOL_VERSION_OFFSET];
    const std::uint8_t inverse_protocol_version = data[DOIP_INVERSE_PROTOCOL_VERSION_OFFSET];
    if(BIT_INVERSE(protocol_version) != inverse_protocol_version) return false;

    std::uint32_t payload_length;
    memcpy(&payload_length, data + DOIP_PAYLOAD_LENGTH_OFFSET, DOIP_HEADER_PAYLOAD_LENGTH_SIZE);
    payload_length = ntohl(payload_length);
    if(data_length != (payload_length + DOIP_HEADER_SIZE)) return false;

    return true;
}

DOIPPayloadType DOIPMessage::DetermineDOIPMessageType(const std::uint8_t* data, std::uint32_t data_length)
{
    //fetch payload type and reorder bytes as needed
    std::uint16_t payload_type;
    memcpy(&payload_type, data + DOIP_PAYLOAD_TYPE_OFFSET, DOIP_HEADER_PAYLOAD_TYPE_SIZE);
    payload_type = ntohs(payload_type);

    switch(payload_type)
    {
        case((std::uint16_t)DOIPPayloadType::GenericNegativeAck):
        {
            return DOIPPayloadType::GenericNegativeAck;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::RoutingActivationRequest):
        {
            return DOIPPayloadType::RoutingActivationRequest;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::RoutingActivationResponse):
        {
            return DOIPPayloadType::RoutingActivationResponse;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::AliveCheckRequest):
        {
            return DOIPPayloadType::AliveCheckRequest;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::AlivecheckResponse):
        {
            return DOIPPayloadType::AlivecheckResponse;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::DiagnosticMessage):
        {
            return DOIPPayloadType::DiagnosticMessage;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::DiagnosticMessagePositiveResponse):
        {
            return DOIPPayloadType::DiagnosticMessagePositiveResponse;
            break;
        }
        case((std::uint16_t)DOIPPayloadType::DiagnosticMessageNegativeResponse):
        {
            return DOIPPayloadType::DiagnosticMessageNegativeResponse;
            break;
        }
        default:
        {
            return DOIPPayloadType::Unsupported;
            break;
        }
    }
}

DOIPRoutingActivationNegativeAck DOIPMessage::GetNegativeAcknoledgement(const std::uint8_t* data, std::uint32_t data_length)
{
    if(data_length != (DOIP_HEADER_SIZE + DOIP_NACK_PAYLOAD_SIZE)) return DOIPRoutingActivationNegativeAck::None;

    const std::uint8_t negatic_ack_code = data[DOIP_NACK_CODE_OFFSET];
    if(negatic_ack_code >= 0x00 && negatic_ack_code <= 0x04)
    {
        return (DOIPRoutingActivationNegativeAck)negatic_ack_code;
    }

    return DOIPRoutingActivationNegativeAck::None;
}

RoutingActivationRequestMessage::RoutingActivationRequestMessage(std::uint16_t diagnostic_address, DOIPActivationType activation_type)
    :DOIPMessage(DOIP_ROUTING_ACTIVATION_REQUEST_PAYLOAD_SIZE, DOIPPayloadType::RoutingActivationRequest)
{
    const std::uint16_t network_order_diagnostic_address = htons(diagnostic_address);

    memcpy(this->doip_message + DOIP_HEADER_SIZE + DOIP_ROUTING_ACTIVATION_REQUEST_DIAGNOSTIC_ADDRESS_RELATIVE_OFFSET,
            &network_order_diagnostic_address,
            DOIP_ROUTING_ACTIVATION_REQUEST_DIAGNOSTIC_ADDRESS_SIZE);
    this->doip_message[DOIP_HEADER_SIZE + DOIP_ROUTING_ACTIVATION_REQUEST_ACTIVATION_TYPE_RELATIVE_OFFSET] = (std::uint8_t)activation_type;
}

RoutingActivationResponse RoutingActivationRequestMessage::ParseActivationResponse(const std::uint8_t* data, std::uint32_t data_length)
{
    if(data_length != (DOIP_HEADER_SIZE + DOIP_ROUTING_ACTIVATION_RESPONSE_PAYLOAD_SIZE)) return RoutingActivationResponse::None;

    const std::uint8_t response = data[DOIP_HEADER_SIZE + DOIP_ROUTING_ACTIVATION_RESPONSE_ACTIVATION_RESPONSE_OFFSET];

    if(response >= 0x08 && response <= 0x0F) return RoutingActivationResponse::Reserved;
    if(response >= 0x12 && response <= 0xDF) return RoutingActivationResponse::Reserved;
    if(response >= 0xE0 && response <= 0xFE) return RoutingActivationResponse::Reserved;
    if(response == 0xFF) return RoutingActivationResponse::Reserved;

    return (RoutingActivationResponse)response;
}

DoIPDiagnosticMessage::DoIPDiagnosticMessage(
        std::uint16_t source_diagnostic_address,
        std::uint16_t target_diagnostic_address,
        const std::uint8_t* diagnostic_message,
        std::uint32_t diagnostic_message_length)
    :DOIPMessage(DOIP_DIAGNOSTIC_MESSAGE_BASE_PAYLOAD_SIZE + diagnostic_message_length, DOIPPayloadType::DiagnosticMessage)
{
    const std::uint16_t network_order_source_address = htons(source_diagnostic_address);
    const std::uint16_t network_order_target_address = htons(target_diagnostic_address);

    memcpy(this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_RELATIVE_OFFSET,
            &network_order_source_address,
            DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_SIZE);
    memcpy(this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_RELATIVE_OFFSET,
            &network_order_target_address,
            DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_SIZE);
    memcpy(this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_DATA_RELATIVE_OFFSET,
            diagnostic_message,
            diagnostic_message_length);
}

DoIPDiagnosticMessage::DoIPDiagnosticMessage(
        std::uint8_t* doip_diagnostic_message,
        std::uint32_t message_length)
    :DOIPMessage(doip_diagnostic_message, message_length) { }

std::uint8_t& DoIPDiagnosticMessage::operator[](std::uint32_t index) const
{
    return this->doip_message[DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_DATA_RELATIVE_OFFSET + index];
}

std::uint8_t* DoIPDiagnosticMessage::GetDiagnosticMessage() const
{
    return this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_DATA_RELATIVE_OFFSET;
}

std::uint32_t DoIPDiagnosticMessage::GetDiagnosticMessageLenght() const
{
    return this->GetPayloadLength() - DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_SIZE - DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_SIZE;
}

std::uint16_t DoIPDiagnosticMessage::GetSourceAddress() const
{
    std::uint16_t source_address;
    memcpy(&source_address,
            this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_RELATIVE_OFFSET,
            DOIP_DIAGNOSTIC_MESSAGE_SOURCE_ADDRESS_SIZE);
    return ntohs(source_address);
}

std::uint16_t DoIPDiagnosticMessage::GetTargetAddress() const
{
    std::uint16_t target_address;
    memcpy(&target_address,
            this->doip_message + DOIP_HEADER_SIZE + DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_RELATIVE_OFFSET,
            DOIP_DIAGNOSTIC_MESSAGE_TARGET_ADDRESS_SIZE);
    return ntohs(target_address);
}

