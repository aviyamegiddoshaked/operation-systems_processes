#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <sys/wait.h>   
#include <fcntl.h>      
#include <string.h>     

#define ARG_COUNT 5 

// Function: write_to_file - Writes a message to a file multiple times
void write_to_file(const char *message, int count) {
    int fd = open("output.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    
    if (fd == -1) {
        perror("Failed to open file"); 
        exit(EXIT_FAILURE); 
    }

    for (int i = 0; i < count; i++) {
        char buffer[1024]; 
        snprintf(buffer, sizeof(buffer), "%s\n", message); 

        if (write(fd, buffer, strlen(buffer)) == -1) {
            perror("Failed to write to file"); 
            close(fd); 
            exit(EXIT_FAILURE); 
        }

        fsync(fd); 
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != ARG_COUNT) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>\n", argv[0]);
        return 1; 
    }

    int fd = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to clear file"); 
        return 1; 
    }
    close(fd); 

    const char *parent_message = argv[1]; 
    const char *child1_message = argv[2]; 
    const char *child2_message = argv[3]; 
    int count = atoi(argv[4]); 

    pid_t pid1, pid2; 

    // Create the first child process
    if ((pid1 = fork()) == 0) { 
        write_to_file(child1_message, count); 
        exit(0); 
    } // **CLOSE THE IF BLOCK**

    // Parent waits for the first child to finish
    waitpid(pid1, NULL, 0); 

    // Create the second child process
    if ((pid2 = fork()) == 0) { 
        write_to_file(child2_message, count); 
        exit(0); 
    } // **CLOSE THE IF BLOCK**

    // Parent waits for the second child to finish
    waitpid(pid2, NULL, 0); 

    // Parent writes its own message to the file after both child processes are complete
    write_to_file(parent_message, count); 

    return 0; 
} // **CLOSE THE MAIN FUNCTION**
