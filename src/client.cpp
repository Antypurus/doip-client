#include <cstdint>
#include <cstring>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "DOIPMessage.h"

#define DOIP_VERSION 0x03
#define TCP_PROTOCOL 0
#define XOR(A,B) A^B
#define INVERSE(A) XOR(A, 0xFF)

int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, TCP_PROTOCOL);
    if(socket_fd == -1)
    {
        printf("failed to create socket\n");
        return 1;
    }

    sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(13400)
    };
    inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);

    int status = connect(socket_fd, (sockaddr*)&server_address, sizeof(server_address));
    if(status < 0)
    {
        printf("failed to connect");
        return 1;
    }

    RoutingActivationRequestMessage routing_activation_request(0x32);
    status = send(socket_fd, routing_activation_request.doip_message, routing_activation_request.doip_message_length, 0);
    if(status != routing_activation_request.doip_message_length)
    {
        printf("failed to send routing activation request");
        return 1;
    }

    std::uint8_t read_buffer[1024];
    status = read(socket_fd, read_buffer, 1024);
    if(!DOIPMessage::IsValidDOIPMesasage(read_buffer, status))
    {
        printf("Invalid DOIP Message Received\n");
        return 1;
    }

    DOIPPayloadType type = DOIPMessage::DetermineDOIPMessageType(read_buffer, status);
    if(type == DOIPPayloadType::GenericNegativeAck)
    {
        printf("Negative ACK Code:%d\n", (int)DOIPMessage::GetNegativeAcknoledgement(read_buffer, status));
        return 1;
    }
    else if(type == DOIPPayloadType::RoutingActivationResponse)
    {
        RoutingActivationResponse activation_response = RoutingActivationRequestMessage::ParseActivationResponse(read_buffer, status);
        if(activation_response == RoutingActivationResponse::SuccessfullActivation || activation_response == RoutingActivationResponse::ActivationPendingConfirmation)
        {
            printf("DOIP Connection Established\n");
        }
        else
        {
            printf("Activation Failed: %d\n", (int)activation_response);
            return 1;
        }
    }

    std::uint8_t read_data[3];
    read_data[0] = 0x22;
    read_data[1] = 0x32;
    read_data[2] = 0x50;

    DoIPDiagnosticMessage diag(0x32, 0x3301, read_data, 3);
    send(socket_fd, diag.doip_message, diag.doip_message_length, 0);

    status = read(socket_fd, read_buffer, 1024);
    std::cout << status << std::endl;

    status = read(socket_fd, read_buffer, 1024);
    std::cout << status << std::endl;

    close(socket_fd);

    return 0;
}
