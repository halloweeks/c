#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>
#include <stdbool.h>

int ZLIB_compress(unsigned char *InData, unsigned int InSize, unsigned char *OutData, unsigned int OutSize, int CompressionFlag) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = InData;
    strm.avail_in = InSize;
    strm.next_out = OutData;
    strm.avail_out = OutSize;

    if (deflateInit(&strm, CompressionFlag) != Z_OK) {
        return Z_ERRNO;  // Return error code if initialization fails
    }

    if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&strm);
        return Z_DATA_ERROR;  // Return error code if deflate fails
    }

    if (deflateEnd(&strm) != Z_OK) {
        return Z_ERRNO;  // Return error code if ending the stream fails
    }

    return strm.total_out;  // Return compressed size if successful
}

int ZLIB_decompress(unsigned char *InData, unsigned int InSize, unsigned char *OutData, unsigned int OutSize) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = InData;
    strm.avail_in = InSize;
    strm.next_out = OutData;
    strm.avail_out = OutSize;

    if (inflateInit(&strm) != Z_OK) {
        return Z_ERRNO;  // Return error code if initialization fails
    }

    if (inflate(&strm, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&strm);
        return Z_DATA_ERROR;  // Return error code if inflate fails
    }

    if (inflateEnd(&strm) != Z_OK) {
        return Z_ERRNO;  // Return error code if ending the stream fails
    }

    return strm.total_out;  // Return decompressed size if successful
}

bool ZLIB_isError(int len) {
    return len < 0;  // Return true if the return value is negative (error code)
}

char *ZLIB_getErrorName(int code) {
    switch (code) {
        case Z_ERRNO:
            return "Z_ERRNO: Error due to I/O or system-related issues.";
        case Z_MEM_ERROR:
            return "Z_MEM_ERROR: Memory allocation failure.";
        case Z_BUF_ERROR:
            return "Z_BUF_ERROR: Input or output buffer issue.";
        case Z_DATA_ERROR:
            return "Z_DATA_ERROR: Corrupted or invalid data.";
        case Z_STREAM_ERROR:
            return "Z_STREAM_ERROR: Stream configuration issue.";
        case Z_VERSION_ERROR:
            return "Z_VERSION_ERROR: Incompatible zlib version.";
        case Z_OK:
            return "Z_OK: No error.";
        default:
            return "Unknown zlib error code.";
    }
}

int main() {
    uint8_t data[1024];
    uint8_t com[1024];
    uint8_t decom[1024];

    memcpy(data, "halloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweekshalloweeks", 241);

    // Compress data
    int len = ZLIB_compress(data, 241, com, 1024, Z_DEFAULT_COMPRESSION);
    
    if (ZLIB_isError(len)) {
        fprintf(stderr, "Error during compression: %s\n", ZLIB_getErrorName(len));
        return 1;
    }

    printf("Compression successful. Compressed size: %d bytes.\n", len);

    // Decompress data (optional for testing)
    int decompressed_len = ZLIB_decompress(com, len, decom, 1024);
    
    if (ZLIB_isError(decompressed_len)) {
        fprintf(stderr, "Error during decompression: %s\n", ZLIB_getErrorName(decompressed_len));
        return 1;
    }

    printf("Decompression successful. Decompressed size: %d bytes.\n", decompressed_len);
    
    return 0;
}