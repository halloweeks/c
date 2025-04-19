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

#include <stdint.h>

#include <sys/mman.h>

// read memory!
ssize_t read_memory(pid_t pid, void *address, void *value, size_t size) {
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = value;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return syscall(__NR_process_vm_readv, pid, local, 1, remote, 1, 0);
}

// get process id by process name 
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

// get module address 
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

void _output(const char *title, unsigned char *data, unsigned int size) {
	printf("%s", title);
	for (unsigned int i = 0; i < size; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
}

void _d() {
	
	printf("waiting: ");
		for (int i = 0; i < 5; i++) {
			printf("%d ", i);
			sleep(1);
		}
		printf("\n");
		
}

int main(int argc, const char *argv[]) {
	const char *process_name = "test"; // target process name 
	const char *module_name = "test"; // module name 
	const long offset = 0x4788; // function address 
	
	pid_t target_pid = -1;
	
	printf("[INFO] Waiting for opening '%s' process\n", process_name);
	
	while (target_pid == -1) {
		target_pid = find_pid(process_name);
	}
	
	printf("[INFO] Process '%s' is now open pid %d\n", process_name, target_pid);
	
	
	int status;
	
	// Attach to the target
	ptrace(PTRACE_ATTACH, target_pid, NULL, NULL);
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
	
	unsigned long long func_addr = base + offset;
	unsigned long long data_ptr = 0;
	unsigned long long length = 0;
	unsigned long long key_ptr = 0;
	unsigned long long ret_addr = 0;
	
	unsigned char data[1024];
	unsigned char key[1024];
	unsigned int read_size = 16;
	
	
	long orig;
	
	// 5. Now safe to read registers
	struct iovec iov;
	struct user_pt_regs regs;
	
	iov.iov_base = &regs;
	iov.iov_len = sizeof(regs);
	
	// save original instruction
	orig = ptrace(PTRACE_PEEKTEXT, target_pid, func_addr, 0);
		
	// write brk instruction
	ptrace(PTRACE_POKETEXT, target_pid, func_addr, (void*)0xd4200000);
	
	// Resume the target process
	ptrace(PTRACE_CONT, target_pid, NULL, NULL);
	
	
	while (1) {
		// write brk instruction
		ptrace(PTRACE_POKETEXT, target_pid, func_addr, (void*)0xd4200000);
		
		// Resume the target process
		ptrace(PTRACE_CONT, target_pid, NULL, NULL);
	
		// Wait for the process to hit the breakpoint (SIGTRAP)
		waitpid(target_pid, &status, 0);
		
		if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
			ptrace(PTRACE_GETREGSET, target_pid, NT_PRSTATUS, &iov);
			
			if (regs.pc == func_addr) {
				printf("[onEnter]\n");
			}
			
			/*
			printf("pc addr: 0x%llx\n", regs.pc);
			printf("func addr: 0x%llx\n", func_addr);
			*/
			
			data_ptr = regs.regs[0]; // save data pointer 
			length = regs.regs[1]; // save data length 
			key_ptr = regs.regs[2]; // save key pointer
			ret_addr = regs.regs[30]; // save return address 
			
			/*
			printf("\nx0 (data ptr): 0x%llx\n", data_ptr);
			printf("x1 (size):     %llu\n",  length);
			printf("x2 (key ptr):  0x%llx\n", key_ptr);
			printf("x30 (ret addr): 0x%llx\n\n", ret_addr);
			*/
			read_memory(target_pid, (void*)data_ptr, data, read_size);
			read_memory(target_pid, (void*)key_ptr, key, read_size);
			
			_output("data: ", data, read_size);
			_output("key: ", key, read_size);
			
			// write original instruction for continue execution of functions 
			ptrace(PTRACE_POKETEXT, target_pid, func_addr, (void*)orig);
			// program counter
			regs.pc = func_addr;
			// set
			ptrace(PTRACE_SETREGSET, target_pid, NT_PRSTATUS, &iov);
			
			// save original return address
			long orig_ret_instr = ptrace(PTRACE_PEEKTEXT, target_pid, (void*)ret_addr, 0);
			// Inject brk at return address
			ptrace(PTRACE_POKETEXT, target_pid, (void*)ret_addr, (void*)0xd4200000);
			
			// Resume to let function finish until ret
			ptrace(PTRACE_CONT, target_pid, NULL, NULL);
			
			// On hit the return address! OnLeave
			waitpid(target_pid, &status, 0);
			
			ptrace(PTRACE_GETREGSET, target_pid, NT_PRSTATUS, &iov);
			
			if (regs.pc == ret_addr) {
				printf("[onLeave]\n");
			}
			
			// OnEnter
			if (read_memory(target_pid, (void*)data_ptr, data, 16) != 16) {
				printf("Can't read memory! region: data\n");
			}
			
			if (read_memory(target_pid, (void*)key_ptr, key, 16) != 16) {
				printf("Can't read memory! region: key\n");
			}
			
			_output("data: ", data, read_size);
			_output("key: ", key, read_size);
			printf("Str: %s\n", (char*)data);
			
			// write original instruction (ret)
			ptrace(PTRACE_POKETEXT, target_pid, (void*)ret_addr, (void*)orig_ret_instr);
			
			// ptrace(PTRACE_POKETEXT, target_pid, func_addr, (void*)0xd4200000);
			
			regs.pc = ret_addr;
			ptrace(PTRACE_SETREGSET, target_pid, NT_PRSTATUS, &iov);
			
			
			
			// If needed, resume execution from here
			// ptrace(PTRACE_CONT, target_pid, NULL, NULL);
			// waitpid(target_pid, &status, 0);
			
		}
		
		if (WIFEXITED(status)/* || WIFSIGNALED(status)*/) {
			printf("Process exited (normally or by crash)\n");
			break;
		}
		
		printf("\n\n");
	}
	
	ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
	return EXIT_SUCCESS;
}