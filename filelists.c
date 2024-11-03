#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    unsigned int current;   // Current number of names
    unsigned int capacity;  // Capacity of the names array
    char **name;           // Pointer to array of names
} names;

// Function to add a name to the list
void add_name(names *n, const char *uname) {
    // Check if the current capacity is reached
    if (n->current >= n->capacity) {
        // Increase capacity (double the current capacity or set to 1 if zero)
        n->capacity = (n->capacity == 0) ? 1 : n->capacity * 2;

        // Reallocate memory for the names array
        char **temp = realloc(n->name, n->capacity * sizeof(char*));
        
        if (temp == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1); // Exit if reallocation fails
        }
        
        n->name = temp; // Update pointer to new memory
    }

    // Allocate memory for the new name and copy it
    n->name[n->current] = malloc((strlen(uname) + 1) * sizeof(char));
    if (n->name[n->current] == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1); // Exit if allocation fails
    }
    strcpy(n->name[n->current], uname); // Copy the name
    n->current++; // Increment the count of names
}

// Function to free all allocated memory
void free_names(names *n) {
    for (unsigned int i = 0; i < n->current; i++) {
        free(n->name[i]); // Free each individual name
    }
    free(n->name); // Free the array of pointers
}

const char* clean_path(const char* path) {
	if (*path != '.' && *path != '/') {
		return path;
	}
	
    while (*path == '.' || *path == '/') {
        path++;  // Move the pointer forward
    }
    return path;
}

void get_all_files(names *n, const char *basePath) {
    struct dirent *entry;
    DIR *dir = opendir(basePath);

    if (dir == NULL) {
        perror("opendir failed");
        return;
    }
    
    char path[4096];
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip the "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            
            snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
            
            if (entry->d_type == DT_REG) {
            	add_name(n, clean_path(path));
            } else if (entry->d_type == DT_DIR) {
                get_all_files(n, path);
            }
        }
    }

    closedir(dir);
}

const char *base_name(const char *path) {
	const char *last_slash = strrchr(path, '/');
	
	return last_slash != NULL ? last_slash + 1 : path;
}

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file_directory> <...>\n", base_name(argv[0]));
		return 1;
	}
	
	names name_list = {0, 0, NULL};
	struct stat st;
	
	for (uint8_t i = 1; i < argc; i++) {
		if (stat(argv[i], &st) != 0) {
			printf("%s does not exist.\n", argv[i]);
			return 1;
		}
		
		if (S_ISDIR(st.st_mode)) {
			get_all_files(&name_list, argv[i]);
		} else if (S_ISREG(st.st_mode)) {
			add_name(&name_list, clean_path(argv[i]));
		}
	}
	
	
	printf("List of files:\n");
	for (unsigned int i = 0; i < name_list.current; i++) {
		printf("%s\n", name_list.name[i]);
	}
	
	// Free the allocated memory
	free_names(&name_list);
	return 0;
}