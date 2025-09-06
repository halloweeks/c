#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addr_len;
} UDPServer;

// Create and bind UDP server with IP and port
int udp_server(UDPServer *server, const char *ip, int port) {
    server->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&server->addr, 0, sizeof(server->addr));
    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server->addr.sin_addr) <= 0) {
        perror("invalid IP address");
        close(server->sockfd);
        return -1;
    }

    server->addr_len = sizeof(server->addr);

    if (bind(server->sockfd, (struct sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
        perror("bind failed");
        close(server->sockfd);
        return -1;
    }

    return 0;
}

// Receive message from any client
ssize_t udp_recv(UDPServer *server, char *buffer, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(struct sockaddr_in);
    ssize_t bytes = recvfrom(server->sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr*)client_addr, &client_len);
    if (bytes >= 0) buffer[bytes] = '\0';
    return bytes;
}

// Send message to a client
ssize_t udp_send(UDPServer *server, const char *message, struct sockaddr_in *client_addr) {
    return sendto(server->sockfd, message, strlen(message), 0,
                  (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
}

// Close server socket
void udp_close(UDPServer *server) {
    close(server->sockfd);
}

// Example usage
int main() {
    UDPServer server;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;

    if (udp_server(&server, "127.0.0.1", 8888) != 0) {
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on 127.0.0.1:8888...\n");

    while (1) {
        ssize_t len = udp_recv(&server, buffer, &client_addr);
        if (len > 0) {
            printf("Received from client: %s\n", buffer);
            udp_send(&server, "Message received", &client_addr);
        }
    }

    udp_close(&server);
    return 0;
}