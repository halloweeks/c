#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

void write_reset(uint8_t *buffer, uint16_t *size) {
	// No need to set buffer zero because new data overwrite it!
	(*size) = 2; // 2 byte space reserved for packet size that later overwrite!
}

void write_u8(uint8_t *data, uint16_t *size, uint8_t value) {
	data[*size] = value;
	(*size) ++;
}

void write_u16(uint8_t *data, uint16_t *size, uint16_t value) {
	data[*size]     = (uint8_t)(value & 0xFF);       // low byte
    data[*size + 1] = (uint8_t)((value >> 8) & 0xFF); // high byte
    (*size) += 2;
}

void write_u32(uint8_t *data, uint16_t *size, uint32_t value) {
    data[*size]     = (uint8_t)(value & 0xFF);         // byte 0 (LSB)
    data[*size + 1] = (uint8_t)((value >> 8) & 0xFF);  // byte 1
    data[*size + 2] = (uint8_t)((value >> 16) & 0xFF); // byte 2
    data[*size + 3] = (uint8_t)((value >> 24) & 0xFF); // byte 3 (MSB)
    (*size) += 4;  // advance the offset by 4 bytes
}


void write_u64(uint8_t *data, uint16_t *size, uint64_t value) {
    data[*size]     = (uint8_t)(value & 0xFF);          // byte 0 (LSB)
    data[*size + 1] = (uint8_t)((value >> 8) & 0xFF);   // byte 1
    data[*size + 2] = (uint8_t)((value >> 16) & 0xFF);  // byte 2
    data[*size + 3] = (uint8_t)((value >> 24) & 0xFF);  // byte 3
    data[*size + 4] = (uint8_t)((value >> 32) & 0xFF);  // byte 4
    data[*size + 5] = (uint8_t)((value >> 40) & 0xFF);  // byte 5
    data[*size + 6] = (uint8_t)((value >> 48) & 0xFF);  // byte 6
    data[*size + 7] = (uint8_t)((value >> 56) & 0xFF);  // byte 7 (MSB)
    (*size) += 8;  // advance the offset by 8 bytes
}

void write_bytes(uint8_t *data, uint16_t *size, const void *src, uint32_t len) {
    memcpy(data + *size, src, len); // copy raw bytes
    *size += len;                    // advance the offset
}

void write_string(uint8_t *data, uint16_t *size, const char *str) {
	size_t len = strlen(str);
	
	memcpy(data + *size, str, len); // copy raw bytes
    *size += len;                    // advance the offset
}

void write_done(uint8_t *data, uint16_t size) {
	data[0] = (uint8_t)(size & 0xFF);       // low byte
    data[1] = (uint8_t)((size >> 8) & 0xFF); // high byte
}

int main(int argc, char const *argv[]) {
	uint8_t buffer[512];
	uint16_t size = 0;
	
	write_reset(buffer, &size);
	
	write_u16(buffer, &size, 1024); // packet type
	write_u32(buffer, &size, 11); // sequence id
	write_u8(buffer, &size, 1); // type 
	write_string(buffer, &size, "hello");
	write_done(buffer, size);
	
	int fd = open("packet.bin", O_WRONLY | O_CREAT | O_TRUNC,  0644);
	write(fd,  buffer, size);
	close(fd);
	
	return 0;
}