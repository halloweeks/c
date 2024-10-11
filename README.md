## typedef data type 
```c
typedef unsigned long size_t;
typedef unsigned char bool;

```

## strcpy

```c
void __strcpy(char *dst, const char *src) {
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}
```

## usage 
```
__strcpy(dest, str);
```

## strcat
```c
void __strcat(char *dst, const char *src) {
	while (*dst) {
		dst++;
	}
	
	while (*src) {
		*dst++ = *src++;
	}
	
	*dst = '\0';
}
```

## strlen

```c
unsigned long __strlen(const char *str) {
	const char *s = str;
	
	while (*s) {
		s++;
	}
	
	return s - str;
}
```

## memcpy
```c
void __memcpy(void *dst, void *src, unsigned long size) {
	while (size--) {
		*(unsigned char*)dst++ = *(unsigned char*)src++;
	}
}
```

## memset
```c
void __memset(void *src, int ch, unsigned long size) {
	while (size--) {
		*(unsigned char*)src++ = (unsigned char)ch;
	}
}
```

## strcmp
```c
unsigned char __strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (*str1 == *str2);
}
```

## Usage 
```c
__strcmp(str1, str2);
```

## strline
```c
unsigned long __strline(const char *str) {
	const char *s = str;
	while (*s != '\n') {
		s++;
	}
	return s - str;
}
```
## usage

```c
__strline(str);
```

## packet 
```c
typedef struct {
	uint32_t src_addr;
	uint32_t dst_addr;
	uint32_t sequence;
	uint32_t timestamp;
	uint16_t checksum;
	uint16_t datasize;
	uint8_t raw_data[65536];
} __attribute__((packed)) packet;

unsigned int _get_packet_size(packet *pkt) {
	return offsetof(packet, raw_data) + pkt->datasize;
	
}

void _payload(packet *pkt, void *data, unsigned short size) {
	pkt->datasize = size;
	
	for (unsigned short i = 0; i < size; i++) {
		pkt->raw_data[i] = ((unsigned char *)data)[i]; // *((unsigned char *)data + i);
	}
}
```

## network order address 
```c
const char *to_ip(unsigned int num) {
	static char ip[16];
	snprintf(ip, sizeof(ip), "%d.%d.%d.%d", (num >> 0) & 0xFF, (num >> 8) & 0xFF, (num >> 16) & 0xFF, (num >> 24) & 0xFF);
	return ip;
}

unsigned int ip_to(const char *addr) {
    unsigned int ip = 0;
    unsigned int octet = 0;

    while (*addr) {
        if (*addr == '.') {
            ip = (ip << 8) | octet; // Shift left and add the current octet
            octet = 0;              // Reset for the next octet
        } else {
            octet = octet * 10 + (*addr - '0'); // Build the current octet
        }
        addr++;
    }
    
    ip = (ip << 8) | octet;
    
    return (ip >> 24) | ((ip >> 8) & 0x0000FF00) | ((ip << 8) & 0x00FF0000) | (ip << 24);
}

```

```c
const char *ip = "127.0.0.1";

unsigned int out = ip_to(ip);

printf("network order: %u\n", out);

printf("ip address: %s\n", to_ip(out));
```

