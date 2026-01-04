#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define ADDRESS "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 4096

#include "packet_map.h"
#include "packet_enum.h"


// Function pointer type for packet handlers
typedef void (*PacketHandler)(int sock, const uint8_t *data);

// O(1) dispatcher table
PacketHandler dispatcher[65536];


void handle_chat(int sock, const uint8_t *data) {
    uint16_t length = 0;
    char message[65535];
    memcpy(&length, data, 2);
	memcpy(message, data + 2, length);
	printf("[CHAT]: %s\n", message);
}

void setup_dispatcher() {
    memset(dispatcher, 0, sizeof(dispatcher));
    dispatcher[_MSG_RESP_CHATMESSAGE] = handle_chat;
    // dispatcher[MSG_RESP_LOGIN]       = handle_login;
    // dispatcher[MSG_RESP_ACTION]      = handle_action;
}


/*
void Dispatcher(uint16_t size, uint16_t type, const uint8_t *data) {
	if (type == _MSG_RESP_CHATMESSAGE) {
		char message[65535];
		uint16_t length = 0;
		
	} else if 
	return;
}*/

int main(int argc, char const *argv[]) {
	setup_dispatcher();
	
	int master_socket;
	
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
	uint8_t buffer[BUFFER_SIZE];
	size_t read_pos = 0;
	size_t parse_pos = 0;
	
	while (1) {
		int n = recv(master_socket, buffer + read_pos, BUFFER_SIZE - read_pos, 0);
		
		if (n == 0) printf("Disconnected\n");
		if (n == -1) printf("Recv error\n");
		if (n <= 0) break;
		
		read_pos += n;
		
		while (read_pos - parse_pos >= 4) {
			memcpy(&packet_size, buffer + parse_pos, 2);
			memcpy(&packet_type, buffer + parse_pos + 2, 2);
			
			if (packet_size < 4 || packet_size > BUFFER_SIZE) {
				parse_pos += 4;
			} else {
				if (read_pos - parse_pos < packet_size) break;
				
				// printf("PACKET TYPE: %s\n", get_packet_name(packet_type));
				// printf("PACKET SIZE: %u\n", packet_size);
				
				// process payload here...
				// dump_data("log.bin", "", buffer + parse_pos + 2, packet_size - 2);
				// Dispatcher(packet_size - 4, packet_type, buffer + parse_pos + 4);
				
				// O(1) dispatcher
				if (dispatcher[packet_type]) {
					dispatcher[packet_type](master_socket, buffer + parse_pos + 4);
					printf("[DEBUG] Handling packet type=%u size=%u\n", packet_type, packet_size);
				} else {
					printf("[WARNING] Unknown packet type %u\n", packet_type);
					close(master_socket);
				}
				
				parse_pos += packet_size;
			}
		}
		
		if (parse_pos > 0) {
			memmove(buffer, buffer + parse_pos, read_pos - parse_pos);
			read_pos -= parse_pos;
		}
		
		parse_pos = 0;
	}
	
	close(master_socket);
	
	return 0;
}