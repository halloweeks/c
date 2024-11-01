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
const char *to_ip(unsigned int const char *to_ip(unsigned int num) {
	static char ip[16];
	// snprintf(ip, sizeof(ip), "%d.%d.%d.%d", (num >> 0) & 0xFF, (num >> 8) & 0xFF, (num >> 16) & 0xFF, (num >> 24) & 0xFF);
	snprintf(ip, sizeof(ip), "%d.%d.%d.%d", (num >> 24) & 0xFF, (num >> 16) & 0xFF, (num >> 8) & 0xFF, (num >> 0) & 0xFF);
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
    
    return ip;
    // return (ip >> 24) | ((ip >> 8) & 0x0000FF00) | ((ip << 8) & 0x00FF0000) | (ip << 24);
}

```

```c
const char *ip = "127.0.0.1";

unsigned int out = ip_to(ip);

printf("network order: %u\n", out);

printf("ip address: %s\n", to_ip(out));
```

## recvline
```c
int recvline(int socket, char *buffer, unsigned int size) {
	unsigned int i = 0;
	
	if (recv(socket, buffer, size, MSG_PEEK) < 0) {
		return -1;
	}
	
	for (int index = 0; index < size; index++) {
		if (buffer[index] == '\n') {
			i = index;
			break;
		}
	}
	
	if (i == 0) return -1;
	
	return recv(socket, buffer, i, 0);
}
```
## route 
```c
char route(const char *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL) return 0;
	
	return strncmp(s1 + 5, s2, strlen(s2)) == 0;
}

if (route(request, "/api/v1/login")) {
        printf("login request!\n");
}
```


## process vm read and write 
```c
#include <sys/uio.h>      // For struct iovec
#include <sys/syscall.h>  // For syscall numbers
#include <unistd.h>       // For syscall function
#include <stdio.h>        // For printf (optional)

ssize_t read_memory(pid_t pid, void *address, void *value, size_t size) {
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = value;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return syscall(__NR_process_vm_readv, pid, local, 1, remote, 1, 0);
}

ssize_t write_memory(pid_t pid, void *address, void *value, size_t size) {
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = value;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return syscall(__NR_process_vm_writev, pid, local, 1, remote, 1, 0);
}
```

## path clean 

#include <stdio.h>
#include <string.h>

char* clean_path(char* path) {
	if (*path != '.' && *path != '/') {
		return path;
	}
	
    while (*path == '.' || *path == '/') {
        path++;  // Move the pointer forward
    }
    return path;
}

int main() {
    char path[] = "../../../example/test.txt";
    printf("Original path: %s\n", path);
    printf("Cleaned path: %s\n", clean_path(path));
    return 0;
}
