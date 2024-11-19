#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


int main(int argc, const char *argv[]) {
	int file = open("/sdcard/pubgmobile/log.txt", O_RDONLY);
	
	if (file == -1) {
		fprintf(stderr, "Can't open file!\n");
		return EXIT_FAILURE;
	}
	
	char data[100];
	
	int len = read(file, data, 100);
	
	if (len == -1) {
		printf("Can't read file!\n");
		return EXIT_FAILURE;
	}
	
	data[len] = '\0';
	
	printf("data: %s\n", data);
	
	// while (1) {}
	
	close(file);
	return 0;
}