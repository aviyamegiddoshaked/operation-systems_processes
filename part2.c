#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define LOCKFILE "lockfile.lock"

void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

void acquire_lock() {
    while (1) {
        int fd = open(LOCKFILE, O_CREAT | O_EXCL, 0666);
        if (fd != -1) { // Lock acquired
            close(fd);
            break;
        } else if (errno != EEXIST) {
            perror("Error creating lock file");
            exit(EXIT_FAILURE);
        }
        // Wait before retrying to avoid busy-waiting
        usleep(10000); // 10 milliseconds delay to reduce contention
    }
}

void release_lock() {
    if (unlink(LOCKFILE) == -1 && errno != ENOENT) { // Ignore "No such file" errors
        perror("Error releasing lock file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 4) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[argc - 1]); // The last argument is the number of times to write each message
    if (count <= 0) {
        fprintf(stderr, "Invalid count value. Must be a positive integer.\n");
        return 1;
    }

    int num_messages = argc - 2; // Exclude program name and count argument

    // Order in which child processes should be executed
    int execution_order[] = {0, 2, 1}; // This ensures first, third, second message order

    // Open file and redirect stdout to it before any forking
    FILE *output_file = fopen("output2.txt", "w");
    if (!output_file) {
        perror("Error opening output file");
        return 1;
    }

    // Redirect stdout to output2.txt
    if (dup2(fileno(output_file), STDOUT_FILENO) == -1) {
        perror("Error redirecting stdout");
        fclose(output_file);
        return 1;
    }
    fclose(output_file); // Close the FILE pointer but stdout is still redirected

    pid_t pids[num_messages];

    for (int i = 0; i < 3; i++) { // Loop only for first, third, second in this order
        int index = execution_order[i];
        pids[index] = fork();
        if (pids[index] < 0) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        } else if (pids[index] == 0) { // Child process
            srand(getpid() ^ time(NULL)); // Seed the random number generator with process ID and time
            acquire_lock();
            write_message(argv[index + 1], count);
            release_lock();
            exit(EXIT_SUCCESS);
        } else {
            if (waitpid(pids[index], NULL, 0) == -1) {
                perror("Error waiting for child process");
            }
        }
    }

    return 0;
}
