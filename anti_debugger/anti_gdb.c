#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

int main() {
    char line[1024];
    int pid = 0;

    FILE* file = fopen("/proc/self/status", "r");
    
    if (file == NULL) {
        perror("Failed to open /proc/self/status");
        return 1;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "TracerPid", 9) == 0) {
            sscanf(line, "TracerPid: %d", &pid);

            if (pid != 0) {
                printf("Being traced! Killing tracer PID %d\n", pid);
                fclose(file);
                int ret = kill(pid, SIGKILL);
                if (ret != 0) {
                    perror("kill failed");
                }
                return 1;
            }
            
            rewind(file);
            // break;
        }
    }

    fclose(file);
    return 0;
}