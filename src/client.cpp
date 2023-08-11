#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define DOIP_VERSION 0x03
#define TCP_PROTOCOL 0
#define XOR(A,B) A^B
#define INVERSE(A) XOR(A, 0xFF)

char* RoutingActivationRequest()
{
    char* res = (char*)malloc(8 + 11);
    memset(res, 0, 8 + 11);

    //doip header
    const unsigned int payload_size = htonl(11);
    const unsigned short payload_type = htons(0x0005);
    res[0] = DOIP_VERSION;
    res[1] = INVERSE(DOIP_VERSION);
    memcpy(res + 2, &payload_type, 2);
    memcpy(res + 4, &payload_size, 4);

    //routing activation part
    const unsigned short diag_address = htons(0x0031);
    memcpy(res + 8, &diag_address, 2);
    res[2 + 8] = 0x00;

    return res;
}

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

    char* routing_activation_request = RoutingActivationRequest();
    status = send(socket_fd, routing_activation_request, 8 + 11, 0);
    if(status != (8 + 11))
    {
        printf("failed to send routing activation request");
        return 1;
    }

    return 0;
}
