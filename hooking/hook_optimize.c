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
	const long offset = 0x60b8; // function address 
	
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
	
	// save original instruction
	long orig = ptrace(PTRACE_PEEKTEXT, target_pid, base + offset, 0);
	
	// write brk instruction
	if (ptrace(PTRACE_POKETEXT, target_pid, base + offset, (void*)0xd4200000) == -1) {
		perror("ptrace_write failed");
	}
	
	// Resume the target process
	ptrace(PTRACE_CONT, target_pid, NULL, NULL);
	
	unsigned long long data_ptr = 0;
	unsigned long long length = 0;
	unsigned long long key_ptr = 0;
	unsigned long long ret_addr = 0;
	
	unsigned char data[1024];
	unsigned char key[1024];
	unsigned int read_size = 16;
	
	// 5. Now safe to read registers
	struct iovec iov;
	struct user_pt_regs regs;
	
	iov.iov_base = &regs;
	iov.iov_len = sizeof(regs);
	
	// Wait for the process to hit the breakpoint (SIGTRAP)
	waitpid(target_pid, &status, 0);
	
	// OnEnter
	if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
		if (ptrace(PTRACE_GETREGSET, target_pid, NT_PRSTATUS, &iov) == -1) {
			perror("ptrace(PTRACE_GETREGSET)");
			return 0;
		}
		
		data_ptr = regs.regs[0]; // save data pointer 
		length = regs.regs[1]; // save data length 
		key_ptr = regs.regs[2]; // save key pointer 
		ret_addr = regs.regs[30]; // save return address 
		
		printf("\nx0 (data ptr): 0x%llx\n", data_ptr);
		printf("x1 (size):     %llu\n",  length);
		printf("x2 (key ptr):  0x%llx\n", key_ptr);
		printf("x30 (ret addr): 0x%llx\n\n", ret_addr);
		
		read_memory(target_pid, (void*)data_ptr, data, read_size);
		read_memory(target_pid, (void*)key_ptr, key, read_size);
		
		_output("data: ", data, read_size);
		_output("key: ", key, read_size);
		
		// write original instruction
		ptrace(PTRACE_POKETEXT, target_pid, base + offset, (void*)orig);
		// program counter
		regs.pc = (uint64_t)base + offset;
		// set
		ptrace(PTRACE_SETREGSET, target_pid, NT_PRSTATUS, &iov);
		
		
		// inject brk #0 at ret addr
		// Step 1: Save return address and original instruction
		// ret_addr = regs.regs[30];
		
		
		
		// save original return address
		long orig_ret_instr = ptrace(PTRACE_PEEKTEXT, target_pid, (void*)ret_addr, 0);
		
		// Inject brk at return address
		ptrace(PTRACE_POKETEXT, target_pid, (void*)ret_addr, (void*)0xd4200000);
		
		// Resume to let function finish until ret
		ptrace(PTRACE_CONT, target_pid, NULL, NULL);
		
		
		
		
		// On hit the return address! OnLeave
		waitpid(target_pid, &status, 0);
		
		// write original instruction (ret)
		ptrace(PTRACE_POKETEXT, target_pid, (void*)ret_addr, (void*)orig_ret_instr);
		
		if (read_memory(target_pid, (void*)data_ptr, data, 16) != 16) {
			printf("Can't read memory! region: data\n");
		}
		
		if (read_memory(target_pid, (void*)key_ptr, key, 16) != 16) {
			printf("Can't read memory! region: key\n");
			
		}
		
		printf("Data: ");
		for (int i = 0; i < read_size; i++) {
			printf("%02x ", data[i]);
		}
		printf("\n");
		
		printf("Key: ");
		for (int i = 0; i < read_size; i++) {
			printf("%02x ", key[i]);
		}
		printf("\n");
		
		printf("Str: %s\n", (char*)data);
		
		// If needed, resume execution from here
		ptrace(PTRACE_CONT, target_pid, NULL, NULL);
		waitpid(target_pid, &status, 0);
		
		
		/*
		if (ptrace(PTRACE_GETREGSET, target_pid, NT_PRSTATUS, &iov) == -1) {
			perror("ptrace(PTRACE_GETREGSET)");
			return 0;
		}
		
		data_ptr = regs.regs[0];
		length = regs.regs[1];
		key_ptr = regs.regs[2];
		ret_addr = regs.regs[30];
		
		printf("\nx0 (data ptr): 0x%llx\n", data_ptr);
		printf("x1 (size):     %llu\n",  length);
		printf("x2 (key ptr):  0x%llx\n", key_ptr);
		printf("x30 (ret addr): 0x%llx\n\n", ret_addr);
		
		*/
		
		/*
		ptrace(PTRACE_SINGLESTEP, target_pid, NULL, NULL);
		waitpid(target_pid, &status, 0);
		*/
		// Step 6: NOW read decrypted data
		// unsigned char data[16];
		// if target function has return value that mean regs2.regs[1] is first parameter of the target function 
		
		
		
	}
	
	ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
	return EXIT_SUCCESS;
}