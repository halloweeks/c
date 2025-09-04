#ifndef PACKET_H
#define PACKET_H

#include "des.h"

class Packet {
	private:
		uint8_t ENCRYPTION_KEY[9] = "L*#)@!&8";
		uint32_t seq_id;
		// int sock;
	public: 
		uint16_t zoneID;
		uint8_t pointID;
		
		int sock;
		uint8_t rdata[65535];
		uint8_t wdata[65535];
		
		uint16_t rsize;
		uint16_t roffset;
		
		uint16_t wsize;
		uint16_t woffset;
		
		Packet() : rsize(0), roffset(0), seq_id(0), sock(-1) {}
		
		void set_socket(int);
		
		void write_reset(void); // reset pointer 
		
		void write_u8(uint8_t); 
		void write_u16(uint16_t);
		void write_u32(uint32_t);
		void write_u64(uint64_t);
		
		void write_i8(int8_t);
		void write_i16(int16_t);
		void write_i32(int32_t);
		void write_i64(int64_t);
		void write_byte(const void *, size_t);
		
		void write_seq_id(void);
		void reset_seq(void);
		
		int recv_data(void);
		
		void read_reset(void);
		uint8_t read_u8();
		uint16_t read_u16();
		uint32_t read_u32();
		uint64_t read_u64();
		
		// int16_t read_i16();
		
		uint16_t read_u16_be();
		uint32_t read_u32_be();
		uint64_t read_u64_be();
		
		int8_t read_i8();
		int16_t read_i16(void);
		int32_t read_i32();
		int64_t read_i64();
		
		void read_byte(void *, size_t);
		
		bool send(bool);
		
		void reset(void); // clear read data
		
		void rewind(void);
		
		void wclear(void); // clear write data
};

void Packet::set_socket(int fd) {
	sock = fd;
}

void Packet::wclear() {
	woffset = 0;
	wsize = 0;
}

void Packet::write_u8(uint8_t value) {
	memcpy(data + woffset, &value, sizeof(value));
	woffset += sizeof(value);
	wsize += sizeof(value);
}

void Packet::write_u16(uint16_t value) {
	memcpy(data + woffset, &value, sizeof(value));
	woffset += sizeof(value);
	wsize += sizeof(value);
}

void Packet::write_u32(uint32_t value) {
	memcpy(data + woffset, &value, sizeof(value));
	woffset += sizeof(value);
	wsize += sizeof(value);
}

void Packet::write_u64(uint64_t value) {
	memcpy(data + woffset, &value, sizeof(value));
	woffset += sizeof(value);
	wsize += sizeof(value);
	
}

void Packet::write_byte(const void *value, size_t len) {
	memcpy(data + woffset, value, len);
	woffset += len;
	wsize += len;
}


void Packet::write_seq_id() {
	seq_id += 1;
	write_u32(seq_id);
}

int recv_data() {
	int len = recv(conn, rdata, sizeof(packet->rdata), MSG_DONTWAIT);
	
	// return 0 for disconnect 
	if (len == 0) {
		return 0;
	}
	
	if (len < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -2; // No data yet
		} else {
			return -1; // Recv error 
		}
	}
	
	rsize = len;
	roffset = 0;
	return len;
}

void Packet::read_reset() {
	
	roffset = 0;
	rsize = 0;
}

uint8_t Packet::read_u8() {
	uint8_t value = 0;
	memcpy(&value, rdata + roffset, sizeof(value));
	roffset += sizeof(value);
	return value;
}

uint16_t Packet::read_u16() {
	uint16_t value = 0;
	memcpy(&value, rdata + roffset, sizeof(value));
	roffset += sizeof(value);
	return value;
}

uint32_t Packet::read_u32() {
	uint32_t value = 0;
	memcpy(&value, rdata + offset, sizeof(value));
	roffset += sizeof(value);
	return value;
}

uint64_t Packet::read_u64() {
	uint64_t value = 0;
	memcpy(&value, rdata + roffset, sizeof(value));
	roffset += sizeof(value);
	return value;
}

void Packet::read_byte(void *value, size_t len) {
	memcpy(value, rdata + roffset, len);
	roffset += len;
}

void Packet::read_i16() {
	int16_t value = 0;
	memcpy(&value, rdata + roffset, sizeof(value));
	roffset += sizeof(value);
	return value;
}


bool Packet::send(bool encrypt) {
	memcpy(wdata, &wsize, 2);
	
	if (encrypt) {
		EncryptData(wdata + 4, wsize - 4, wdata + 4, ENCRYPTION_KEY);
	}
	
	return::send(sock, wdata, wsize, MSG_DONTWAIT) == wsize;
}

void Packet::reset_write(void) {
	memset(wdata, 0, sizeof(data));
	woffset = 2;
	wsize = 0;
}

void Packet::reset_seq() {
    seq_id = 0;
}

void Packet::rewind() {
	roffset = 0;
}

#endif // PACKET_H