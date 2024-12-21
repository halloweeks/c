#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#include <sys/mman.h>
#include <time.h>

const char *base_name(const char *path) {
    const char *last_slash = strrchr(path, '/');
    return last_slash != NULL ? last_slash + 1 : path;
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pakfile>\n", base_name(argv[0]));
        return EXIT_FAILURE;
    }

    clock_t start_time = clock();
    
    const char *url_pattern = "(https?://[a-zA-Z0-9./?=_-]+\\.ifs)";
    
    regex_t regex;
    regmatch_t matches[1]; // Store matched groups
    
    // Compile the regex
    if (regcomp(&regex, url_pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        return 1;
    }
    
    struct stat st;

    if (stat(argv[1], &st) != 0) {
        fprintf(stderr, "'%s' does not exist.\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "'%s' is not a regular file!\n", argv[1]);
        return EXIT_FAILURE;
    }

    int file = open(argv[1], O_RDONLY);
    
    if (file == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char *data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, file, 0);
    
    if (data == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(file);
        return EXIT_FAILURE;
    }

    char *ptr = data;
    
    
    const char *cursor = data;
    
    while (regexec(&regex, cursor, 1, matches, 0) == 0) {
        // Get the start and end position of the match
        int start = matches[0].rm_so;
        int end = matches[0].rm_eo;

        // Print the matched URL
        // printf("Found URL: %.*s\n", end - start, cursor + start);
        printf("%.*s\n", end - start, cursor + start);
        
        // Move the cursor forward to search for more matches
        cursor += end;
    }

    // Free the compiled regex
    regfree(&regex);
    
    
    munmap(data, st.st_size);
    close(file);

    clock_t end_time = clock();
    double time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    const double MB = 1024.0 * 1024.0;
    printf("Processed %.2f MB, speed = %.2f MB/s, complete in %.2f seconds\n",
           st.st_size / MB, st.st_size / MB / time, time);

    return EXIT_SUCCESS;
}