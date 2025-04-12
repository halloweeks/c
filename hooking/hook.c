#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <linux/ptrace.h>
#include <linux/elf.h>
#include <string.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include <stdbool.h>

#include <sys/mman.h>

ssize_t read_memory(pid_t pid, void *address, void *value, size_t size) {
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = value;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return syscall(__NR_process_vm_readv, pid, local, 1, remote, 1, 0);
}

pid_t find_pid(const char *process_name) {
	DIR *dir = opendir("/proc");
	struct dirent *entry = NULL;
	char cmdline_path[256];
	char cmdline[256];
	int fd;
	
	if (dir == NULL) {
		return -1;
	}
	
	while ((entry = readdir(dir)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) || (entry->d_type != DT_DIR) || (strspn(entry->d_name, "0123456789") != strlen(entry->d_name))) {
			continue;
		}
		snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);
		fd = open(cmdline_path, O_RDONLY);
		read(fd, cmdline, 256);
		close(fd);
		
		
		if (strstr(cmdline, process_name) != NULL) {
			closedir(dir);
			return atoi(entry->d_name);
		}
		/*
		if (strncmp(cmdline, process_name, strlen(process_name)) == 0) {
			closedir(dir);
			return atoi(entry->d_name);
		}*/
	}
	closedir(dir);
	return -1;
}

bool ptrace_write(pid_t pid, long addr, void *value, size_t size) {
    long *data = (long*) value;
    
    for (size_t offset = 0; offset < size; offset += sizeof(long)) {
        // Write the next long-sized value to the target process's memory
        if (ptrace(PTRACE_POKEDATA, pid, addr + offset, data[offset / sizeof(long)]) == -1) {
            return false;  // Return false on failure
        }
    }
    
    return true;
}

bool ptrace_write2(pid_t pid, long addr, void *value, size_t size) {
    size_t i = 0;
    long word;

    // Write word-aligned chunks
    for (; i + sizeof(long) <= size; i += sizeof(long)) {
        memcpy(&word, (char*)value + i, sizeof(long));
        if (ptrace(PTRACE_POKEDATA, pid, addr + i, word) == -1) {
            return false;
        }
    }

    // Handle trailing bytes (if any)
    if (i < size) {
        // Read the current word at the destination
        word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (word == -1 && errno) return false;

        // Modify only the needed bytes
        memcpy((char*)&word, (char*)value + i, size - i);

        // Write it back
        if (ptrace(PTRACE_POKEDATA, pid, addr + i, word) == -1) {
            return false;
        }
    }

    return true;
}

long long get_module_address(pid_t process_id, const char *module_name) {
	char filename[256];
	char line[1024];
	FILE *fp = NULL;
	long long start = -1, end = -1;
	char permissions[5];
	
	snprintf(filename, sizeof(filename), "/proc/%d/maps", process_id);
	
	if (!(fp = fopen(filename, "r"))) {
		return -1;
	}
	
	while (fgets(line, sizeof(line), fp)) {
		if (sscanf(line, "%llx-%llx %4s", &start, &end, permissions) == 3) {
			if (strstr(line, module_name)/*&& permissions[2] == 'x'*/) {
				fclose(fp);
				return start;
			}
		}
	}
	
	fclose(fp);
	return -1;
}


// cc hook.c -o hook -static -ffunction-sections -fdata-sections -Wl,--gc-sections

// breakpoint instruction
uint8_t brk_shellcode[] = {
    0x00, 0x00, 0x20, 0xD4  // brk #0
};

int main(int argc, const char *argv[]) {
	/*
	if (argc != 2) {
		printf("Usage: %s <process_name>\n", argv[0]);
		return 0;
	}*/
	
	const char *process_name = "test"; // target process name 
	const char *module_name = "test"; // module name 
	const long offset = 0x4740; // target function address 
	
	pid_t target_pid = -1;
	
	printf("[INFO] Waiting for opening '%s' process\n", process_name);
	
	while (target_pid == -1) {
		target_pid = find_pid(process_name);
	}
	
	printf("[INFO] Process '%s' is now open pid %d\n", process_name, target_pid);
	
	
	int status;
	
	// 1. Attach to the target
	ptrace(PTRACE_ATTACH, target_pid, NULL, NULL);
	// Wait for the target process to stop
	waitpid(target_pid, &status, 0);
	
	printf("Tracing process %d\n", target_pid);
	
	// Get base address 
	long long base = get_module_address(target_pid, module_name);
	
	if (base == -1) {
		fprintf(stderr, "Can't find module!\n");
		// Detach from the target process
		ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
		return 1;
	}
	
	// ...
	printf("base: 0x%llx\n", base);
	
	printf("addr: 0x%llx\n", base + offset);
	
	
	long orig = ptrace(PTRACE_PEEKTEXT, target_pid, base + offset, 0);
	
	if (ptrace(PTRACE_POKETEXT, target_pid, base + offset, (void*)0xd4200000) == -1) {
		perror("ptrace_write failed");
	}
	
	/*
	// 2. Inject brk #0 shellcode
	if (!ptrace_write2(target_pid, base + offset, brk_shellcode, sizeof(brk_shellcode))) {
		perror("ptrace_write failed");
	}*/
	
	// 3. Resume the process
	ptrace(PTRACE_CONT, target_pid, NULL, NULL);
	
	// 4. Wait for the process to hit the breakpoint (SIGTRAP)
	waitpid(target_pid, &status, 0);
	
	if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
		// 5. Now safe to read registers
		struct iovec iov;
		struct user_pt_regs regs;
		
		iov.iov_base = &regs;
		iov.iov_len = sizeof(regs);
		
		if (ptrace(PTRACE_GETREGSET, target_pid, NT_PRSTATUS, &iov) == -1) {
			perror("ptrace(PTRACE_GETREGSET)");
			return 0;
		}
		
		unsigned long long data_ptr = regs.regs[0];
		unsigned long long length = regs.regs[1];
		unsigned long long key_ptr = regs.regs[2];
		
		printf("x0 (data ptr): 0x%llx\n", data_ptr);
		printf("x1 (size):     %llu\n",  length);
		printf("x2 (key ptr):  0x%llx\n", key_ptr);
		
		unsigned char data[16];
		unsigned char key[32];
		
		ssize_t read_size = 16;
		
		if (read_memory(target_pid, (void*)data_ptr, data, read_size) == -1) {
			perror("Error reading data");
		} else {
			printf("Data: ");
			for (int i = 0; i < read_size; i++) {
				printf("%02x ", data[i]);
			}
			printf("\n");
		}
		
		read_size = 32;
		
		if (read_memory(target_pid, (void*)regs.regs[2], key, read_size) == -1) {
			perror("Error reading key");
		} else {
			printf("Key: ");
			for (int i = 0; i < read_size; i++) {
				printf("%02x ", key[i]);
			}
			printf("\n");
		}
		
		ptrace(PTRACE_POKETEXT, target_pid, base + offset, (void*)/*(uintptr_t)*/orig);
		
		// ptrace_write2(target_pid, base + offset, &orig, sizeof(orig));
		
		
		regs.pc = (uint64_t)base + offset;
		ptrace(PTRACE_SETREGSET, target_pid, NT_PRSTATUS, &iov);
		
		
		// If needed, resume execution from here
		ptrace(PTRACE_CONT, target_pid, NULL, NULL);
	}
	
	ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
	return EXIT_SUCCESS;
}