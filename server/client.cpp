#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define ADDRESS "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

#include "packet_map.h"

int main(int argc, char const *argv[]) {
	int master_socket;
	uint8_t buffer[BUFFER_SIZE];
	size_t recv_len = 0;
	
	
	struct sockaddr_in serv_addr;
	
	memset(&serv_addr, 0, sizeof(serv_addr));
		
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("[ERROR] CAN'T CREATE SOCKET\n");
		return 1;
	}
	
	// assign IP, PORT
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr) == -1) {
		printf("[ERROR] INVALID ADDRESS/ ADDRESS NOT SUPPORTED\n");
		return 1;
	}
	
	// connect
	if (connect(master_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		printf("[ERROR] CAN'T CONNECT TO THE HOST %s:%d\n", ADDRESS, PORT);
		return 1;
	}
	
	uint16_t packet_size = 0;
	uint16_t packet_type = 0;
	int n;
	size_t offset = 0;
	
	while (1) {
        n = recv(master_socket, buffer + recv_len, BUFFER_SIZE - recv_len, 0);
        
        if (n == 0) {
            printf("disconnected!\n");
            break;
        }
        
        if (n == -1) {
            printf("Can't read data!\n");
            break;
        }

        recv_len += n;
        printf("data received: %d (buffer now %zu)\n", n, recv_len);

        offset = 0;
        while (recv_len - offset >= 4) {
            
            memcpy(&packet_size, buffer + offset, 2);

            if (recv_len - offset < packet_size) {
                // not a full packet yet
                break;
            }
            
            if (packet_size < 4) {
            	printf("Invalid packet size: %u\n", packet_size);
                break;
            }
            
            memcpy(&packet_type, buffer + offset + 2, 2);

            printf("PACKET TYPE: %s\n", get_packet_name(packet_type));
            printf("PACKET SIZE: %u\n", packet_size);

            // process payload here...
            // dump_data("log.bin", "", buffer + offset + 2, packet_size - 2);

            offset += packet_size;
        }

        if (offset > 0) {
            memmove(buffer, buffer + offset, recv_len - offset);
            recv_len -= offset;
        }
    }
    
    
	close(master_socket);
	
	return 0;
}