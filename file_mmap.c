#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char *argv[]) {
	int file = open(argv[1], O_RDONLY);
	
	if (file == -1) {
		perror("Error opening file");
		return 1;
	}
	
	// Get the size of the file
	struct stat st;
	
	if (fstat(file, &st) == -1) {
		perror("Error getting file size");
        close(file);
        return 1;
    }

    // Map the file into memory
    uint8_t *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file, 0);
    
    if (data == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(file);
        return 1;
    }
    
    write(1, data, 10);
    
    // Unmap the file when done
    munmap(data, st.st_size);
    close(file);
    return 0;
}
