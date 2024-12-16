// to run the test: 
// gcc part1_test.c -o part1_test
// gcc part1.c -o part1
// ./part1_test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define OUTPUT_FILE "output.txt"

void clean_up(const char* filename) {
    if (remove(filename) != 0) {
        perror("Failed to clean up the output file");
    }
}

int verify_output(const char* filename, const char* expected_output) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the output file for verification");
        return 0;
    }

    char buffer[1024];
    size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);

    buffer[read_size] = '\0';  // Null-terminate the buffer
    return strcmp(buffer, expected_output) == 0;
}

int main() {
    const char* parent_message = "Parent pim";
    const char* child1_message = "Child1 pam";
    const char* child2_message = "Child2 pom";
    int count = 3;

    char count_str[10];
    snprintf(count_str, sizeof(count_str), "%d", count);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process: Execute part1 program
        execl("./part1", "./part1", parent_message, child1_message, child2_message, count_str, NULL);
        perror("execl failed");
        exit(1);
    }
    

    // Parent process: Wait for the child to finish
    wait(NULL);

    // Expected output
    char expected_output[1024];
    snprintf(expected_output, sizeof(expected_output),
             "%s\n%s\n%s\n"
             "%s\n%s\n%s\n"
             "%s\n%s\n%s\n",
             child1_message, child1_message, child1_message,
             child2_message, child2_message, child2_message,
             parent_message, parent_message, parent_message);

    // Verify the output
    if (verify_output(OUTPUT_FILE, expected_output)) {
        printf("Test passed: Output is as expected.\n");
    } else {
        printf("Test failed: Output does not match the expected content.\n");
    }

    // Clean up
    clean_up(OUTPUT_FILE);

    return 0;
}