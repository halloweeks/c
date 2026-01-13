#ifndef NET_RW_H
#define NET_RW_H

#include <stdint.h>

// Check for known endian macros
#if !defined(__BYTE_ORDER__) || !defined(__ORDER_BIG_ENDIAN__) || !defined(__ORDER_LITTLE_ENDIAN__)
    #error "Cannot determine endianness. __BYTE_ORDER__ or __ORDER_BIG_ENDIAN__/__ORDER_LITTLE_ENDIAN__ not defined."
#endif

/* Type size safety */
_Static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");
_Static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes");

static inline void write_u8(uint8_t *p, uint8_t v)
{
	p[0] = v;
}

static inline void write_u16(uint8_t *p, uint16_t v)
{
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap16(v);
	#endif
	
    __builtin_memcpy(p, &v, sizeof(v));
}


static inline void write_u32(uint8_t *p, uint32_t v)
{
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap32(v);
	#endif
	
	__builtin_memcpy(p, &v, sizeof(v));
}


static inline void write_u64(uint8_t *p, uint64_t v)
{
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap64(v);
	#endif
	
	__builtin_memcpy(p, &v, sizeof(v));
}


static inline uint8_t read_u8(const uint8_t *p) {
	return p[0];
}


static inline uint16_t read_u16(const uint8_t *p) {
	uint16_t v;
	
	__builtin_memcpy(&v, p, sizeof(v));
	
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap16(v);
	#endif
	
	return v;
}

static inline uint32_t read_u32(const uint8_t *p) {
	uint32_t v;
	__builtin_memcpy(&v, p, sizeof(v));
	
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap32(v);
	#endif
	
	return v;
}


static inline uint64_t read_u64(const uint8_t *p) {
	uint64_t v;
	__builtin_memcpy(&v, p, sizeof(v));
	
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		v = __builtin_bswap64(v);
	#endif
	
	return v;
}


#endif /* NET_RW_H */ 