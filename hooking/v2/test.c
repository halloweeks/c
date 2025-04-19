#include <stdio.h>
#include <unistd.h>

void DecryptPak(unsigned char *data, unsigned int size, const unsigned char *key) {
	while (size--) {
		data[size] ^= key[0];
	}
	return;
}

int main() {
	// hold key and data
	unsigned char key[32];
	unsigned char data[16] = {
		0x11, 0x18, 0x15, 0x15, 0x16, 0x0e, 0x1c, 0x1c,
		0x12, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	// put some dummy 
	memset(key, 0x79, 32);
	// memset(data, 0x79, 16);
	sleep(1);
	// run function 
	DecryptPak(data, 10, key);
	
	
	DecryptPak(data, 14, key);
	sleep(1);
	
	return 0;
}

/*



#include <stdio.h>
#include <unistd.h>
#include "AES_128_ECB.h"

void DecryptPak(unsigned char *data, unsigned int size, const unsigned char *key) {
	sleep(5);
	AES_CTX ctx;
	AES_DecryptInit(&ctx, key);
	AES_Decrypt(&ctx, data, size, data);
	sleep(1);
	AES_CTX_Free(&ctx);
	return;
}

int main() {
	// hold key and data
	unsigned char key[16];
	
	unsigned char data[16] = {
		0xfb, 0xb6, 0x24, 0xf1, 0x50, 0x74, 0xcc, 0xf8,
		0x97, 0xb2, 0x56, 0x55, 0x77, 0x28, 0x89, 0xa2
	};
	
	// put key
	memset(key, 0x79, 16);
	
	printf("encrypted: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
	
	// run function
	DecryptPak(data, 16, key);
	
	printf("decrypted: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
	
	printf("string: %s\n", (char*)data);
	return 0;
}*/