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
	uint32_t checksum;
	uint32_t timestamp;
	uint16_t datasize;
	uint8_t flags; // bits for encryption, compression, method 
	uint8_t *data;
} packet;
```

## unsigned int to ip
```c
void to_ip(unsigned int num) {
    // Extract each byte using bitwise shifts and masks
    unsigned char byte1 = (num >> 24) & 0xFF;
    unsigned char byte2 = (num >> 16) & 0xFF;
    unsigned char byte3 = (num >> 8) & 0xFF;
    unsigned char byte4 = num & 0xFF;
    
    // Print in dotted decimal format
    printf("IPv4 Address: %d.%d.%d.%d\n", byte1, byte2, byte3, byte4);
}

to_ip(3232235777);
```


