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

    const unsigned short diagnostic_address = htons(0x0032);
    DOIPMessage routing_activation_request(11, DOIPPayloadType::RoutingActivationRequest);
    memcpy(&routing_activation_request[0], &diagnostic_address, sizeof(diagnostic_address));
    routing_activation_request[2] = 0x00;

    status = send(socket_fd, routing_activation_request.doip_message, routing_activation_request.doip_message_length, 0);
    if(status != routing_activation_request.doip_message_length)
    {
        printf("failed to send routing activation request");
        return 1;
    }

    return 0;
}
