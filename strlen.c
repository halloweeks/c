/*
output:
Average execution time of code1: 0.183837 seconds
Average execution time of code2: 0.070792 seconds
Code2 is 61.49% efficient Code1
code1 length: 104857600
code2 length: 104857600
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define size 104857600

// calculate strlen 1
unsigned int _code1(const char *str) {
	const char *s = str;
	while (*s++);
	return (s - str) - 1;
}

// calculate strlen 2
unsigned int _code2(const char *str) {
	const char *s = str;
	while (*(s + 0) && *(s + 1) && *(s + 2) && *(s + 3) && *(s + 4) && *(s + 5) && *(s + 6) && *(s + 7)) {
		s += 8;
	}
	
	while (*s++);
	
	return (s - str) - 1;
}

int main(){
	clock_t start, end;
    double cpu_time_used;
    
	char *data = malloc(size);
	
	if (data == NULL) {
		fprintf(stderr, "memory allocation failed!\n");
		return 1;
	}
	
	memset(data, 0x79, size);
	data[size] = '\0';
	
	
	int iterations = 20;
    double total_time_code1 = 0;
    double total_time_code2 = 0;

    // Measuring time for code1
    for (int i = 0; i < iterations; i++) {
        start = clock();
        _code1(data);
        end = clock();
        total_time_code1 += ((double) (end - start)) / CLOCKS_PER_SEC;
    }

    double avg_time_code1 = total_time_code1 / iterations;
    printf("Average execution time of code1: %f seconds\n", avg_time_code1);

    // Measuring time for code2
    for (int i = 0; i < iterations; i++) {
        start = clock();
        _code2(data);
        end = clock();
        total_time_code2 += ((double) (end - start)) / CLOCKS_PER_SEC;
    }

    double avg_time_code2 = total_time_code2 / iterations;
    printf("Average execution time of code2: %f seconds\n", avg_time_code2);

    // Calculating percentage difference
    double percentage_difference = ((avg_time_code1 - avg_time_code2) / avg_time_code1) * 100;
    printf("Code2 is %.2f%% efficient Code1\n", percentage_difference);
    
    unsigned int a = _code1(data);
    unsigned int b = _code2(data);
    
    printf("code1 length: %u\n", a);
    printf("code2 length: %u\n", b);
    
    
    free(data); // Free the allocated memory
	
	return 0;
}
