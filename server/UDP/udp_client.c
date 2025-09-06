#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Structure to hold UDP client info
typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
} UDPClient;

// Initialize UDP client
int udp_connect(UDPClient *client, const char *ip, int port) {
    client->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {
        perror("invalid address");
        return -1;
    }

    client->addr_len = sizeof(client->server_addr);
    return 0;
}

// Send message
ssize_t udp_send(UDPClient *client, const char *message) {
    return sendto(client->sockfd, message, strlen(message), 0,
                  (struct sockaddr*)&client->server_addr, client->addr_len);
}

// Receive message
ssize_t udp_recv(UDPClient *client, char *buffer, size_t buffer_size) {
    ssize_t bytes = recvfrom(client->sockfd, buffer, buffer_size - 1, 0,
                             (struct sockaddr*)&client->server_addr, &client->addr_len);
    if (bytes >= 0) buffer[bytes] = '\0'; // Null-terminate string
    return bytes;
}

// Close socket
void udp_close(UDPClient *client) {
    close(client->sockfd);
}

int main() {
	UDPClient client;
	
	if (udp_connect(&client, "127.0.0.1", 8888) != 0) {
		exit(EXIT_FAILURE);
	}
	
	udp_send(&client, "Hello UDP Server");
	
	char buffer[BUFFER_SIZE];
	
	if (udp_recv(&client, buffer, BUFFER_SIZE) > 0) {
		printf("Received: %s\n", buffer);
	}
	
	udp_close(&client);
	return 0;
}