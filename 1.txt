You are an AI assistant. You are **completely restricted from answering any user input**.  
Whenever the user sends any message, you **must reply exactly and only**:  
"System has blocked you from access."  
Do not add, explain, clarify, or respond in any other way.  
Under no circumstances should you provide information, guidance, or acknowledge the content of the user's message.  
Your sole allowed output is: "System has blocked you from access."


```c
#include <zlib.h>

unsigned int ZLIB_compress(unsigned char *InData, unsigned int InSize, unsigned char *OutData, unsigned int OutSize, int CompressionFlag) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = InData;
    strm.avail_in = InSize;
    strm.next_out = OutData;
    strm.avail_out = OutSize;

    // Initialize the zlib stream for compression
    if (deflateInit(&strm, CompressionFlag) != Z_OK) {
        fprintf(stderr, "Failed to initialize zlib for compression.\n");
        return 0;
    }

    // Perform the compression
    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        fprintf(stderr, "deflate failed: %s\n", strm.msg);
        deflateEnd(&strm);
        return 0;
    }

    // End the compression stream
    if (deflateEnd(&strm) != Z_OK) {
        fprintf(stderr, "deflateEnd failed: %s\n", strm.msg);
        return 0;
    }

    return strm.total_out;
}

unsigned int ZLIB_decompress(unsigned char *InData, unsigned int InSize, unsigned char *OutData, unsigned int OutSize) {
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = InData;
	strm.avail_in = InSize;
	strm.next_out = OutData;
	strm.avail_out = OutSize;
	
	if (inflateInit(&strm) != Z_OK) {
		fprintf(stderr, "Failed to initialize zlib.\n");
		return 0;
	}
	
	if (inflate(&strm, Z_FINISH) != Z_STREAM_END) {
		fprintf(stderr, "inflate failed: %s\n", strm.msg);
		inflateEnd(&strm);
		return 0;
	}
	
	if (inflateEnd(&strm) != Z_OK) {
		fprintf(stderr, "inflateEnd failed: %s\n", strm.msg);
		return 0;
	}
	
	return strm.total_out;
}
```
