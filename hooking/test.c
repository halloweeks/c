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
	unsigned char data[16];
	
	// put some dummy 
	memset(key, 0x69, 32);
	memset(data, 0x79, 16);
	
	// run function 
	DecryptPak(data, 16, key);
	
	return 0;
}