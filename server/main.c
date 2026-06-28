#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "net_rw.h"

typedef enum {
	_MSG_INVALID,
	_MSG_REQUEST_LOGIN = 1001,
	_MSG_RESP_LOGIN
} packet_type_t;

typedef struct {
	uint8_t  version_major;
	uint8_t  version_minor;
	uint16_t version_patch;
	int sock;
	uint32_t seq_id;
	uint8_t data[1024];
	uint16_t size;
} Connection;

int set_nonblocking(Connection *conn)
{
    int flags = fcntl(conn->sock, F_GETFL, 0);
    if (flags < 0) return -1;

    return fcntl(conn->sock, F_SETFL, flags | O_NONBLOCK);
}

int connect_server(const char *ip, unsigned short port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

bool send_packet(Connection *conn, bool enc)
{
	/*
	if (enc) {
		EncryptData(conn->data + 4, conn->size - 4, conn->data + 4, ENCRYPTION_KEY);
	}
	*/
    return send(conn->sock, conn->data, conn->size, 0) == conn->size;
}

void disconnect(Connection *conn)
{
    if (conn->sock >= 0)
        close(conn->sock);
}

void ServerLogin(Connection *c, const char *username, const char *password) {
	size_t user_len = strlen(username);
	size_t pass_len = strlen(password);
	
	if (user_len > UINT8_MAX || pass_len > UINT8_MAX) return;
	
	uint8_t user_len8 = (uint8_t)user_len;
	uint8_t pass_len8 = (uint8_t)pass_len;
	
	// reserve 2 byte for packet length 
	c->size = 2;
	
	// write packet type
	write_u16(c->data + c->size, _MSG_REQUEST_LOGIN); 
	c->size += 2;
	
	// write sequence id
	write_u32(c->data + c->size, ++c->seq_id);
	c->size += 4;
	
	// write version major
	write_u8 (c->data + c->size, c->version_major);
	c->size += 1;
	
	// write version minor
	write_u8 (c->data + c->size, c->version_minor);
	c->size += 1;
	
	// write version patch
	write_u16(c->data + c->size, c->version_patch); 
	c->size += 2;
	
	// write username length 
	write_u8 (c->data + c->size, user_len8);
	c->size += 1;
	
	// write username 
	write_raw(c->data + c->size, username, user_len8);
	c->size += user_len8;
	
	// write password length 
	write_u8 (c->data + c->size, pass_len8);
	c->size += 1;
	
	// write password 
	write_raw(c->data + c->size, password, pass_len8);
	c->size += pass_len8;
	
	// update packet size
	write_u16(c->data, c->size);
	
	send_packet(c, false);
}

int main(int argc, const char *argv[]) {
	Connection client = {0};
	
	client.version_major = 2;
	client.version_minor = 195;
	client.version_patch = 304;
	
	client.sock = connect_server("127.0.0.1", 8888);
	
	if (client.sock == -1) {
		printf("Failed to connect!\n");
		return 1;
	}
	
	ServerLogin(&client, "admin", "admin@123");
	
	return 0;
}