#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "net_rw.h"
#include "packet_enum.h"
#include "des.h"

typedef struct {
	int sock;
	uint32_t seq_id;
	uint8_t recv_buff[4096];
	int recv_len;
	uint8_t send_buff[1024];
	int send_len;
} Connection;

static uint8_t ENCRYPTION_KEY[9] = "L*#)@!&8";

void Login(Connection *conn, const char *username, const char *password) {
	int username_len = strlen(username);
	int password_len = strlen(password);
	
	conn->send_len = 2;
	
	// write packet type 
	write_u16(conn->send_buff    + conn->send_len, _MSG_NEWLOGIN_LOGINTOL); conn->send_len += 2;
	
	// write sequence id
	write_u32(conn->send_buff    + conn->send_len, ++conn->seq_id);        conn->send_len += 4;
	
	// write username length 
	write_u8(conn->send_buff     + conn->send_len, username_len);          conn->send_len += 1;
	
	// write username 
	write_bytes(conn->send_buff  + conn->send_len, username, username_len); conn->send_len += username_len;
	
	// write password length 
	write_u8(conn->send_buff     + conn->send_len, password_len); conn->send_len += 1;
	
	// write password 
	write_bytes(conn->send_buff  + conn->send_len, password, password_len); conn->send_len += password_len;
	
	// update packet size 
	write_u16(conn->send_buff,     conn->send_len);
	
	// EncryptData(conn->send_buff + 4, conn->send_len - 4, conn->send_buff + 4, ENCRYPTION_KEY);
	
	// e.g write file for testing not socket
	write(conn->sock, conn->send_buff, conn->send_len);
}


int main() {
	Connection conn;
	memset(&conn, 0, sizeof(conn));
	
	int fd = open("data.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	conn.sock = fd;
	
	Login(&conn, "admin", "admin@123");
	
	close(fd);
	return 0;
}