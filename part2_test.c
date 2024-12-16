// to run the test: 
// gcc part2_test.c -o part2_test
// gcc part2.c -o part2
// ./part2_test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define OUTPUT_FILE "output2.txt"
#define LOCK_FILE "lockfile.lock"

void clean_up(const char *filename) {
    if (remove(filename) != 0) {
        perror("Failed to clean up file");
    }
}

int verify_output(const char *filename, const char **messages, int message_count, int repeats) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open the output file for verification");
        return 0;
    }

    char buffer[256];
    int counts[message_count];
    memset(counts, 0, sizeof(counts));

    while (fgets(buffer, sizeof(buffer), file)) {
        for (int i = 0; i < message_count; i++) {
            if (strncmp(buffer, messages[i], strlen(messages[i])) == 0) {
                counts[i]++;
                break;
            }
        }
    }
    fclose(file);

    for (int i = 0; i < message_count; i++) {
        if (counts[i] != repeats) {
            fprintf(stderr, "Error: Message '%s' was written %d times (expected %d).\n",
                    messages[i], counts[i], repeats);
            return 0;
        }
    }
    return 1;
}

int main() {
    const char *messages[] = {"First message", "Second message", "Third message"};
    int repeats = 3;
    int message_count = sizeof(messages) / sizeof(messages[0]);

    // Construct the command-line arguments
    char repeats_str[10];
    snprintf(repeats_str, sizeof(repeats_str), "%d", repeats);

    // Build the command to execute part2
    char command[512] = "./part2";
    for (int i = 0; i < message_count; i++) {
        strcat(command, " \"");
        strcat(command, messages[i]);
        strcat(command, "\"");
    }
    strcat(command, " ");
    strcat(command, repeats_str);
    strcat(command, " > ");
    strcat(command, OUTPUT_FILE);

    // Run the program
    int ret = system(command);
    if (ret != 0) {
        fprintf(stderr, "Error: part2 program failed to execute correctly.\n");
        return 1;
    }

    // Verify the output file
    if (verify_output(OUTPUT_FILE, messages, message_count, repeats)) {
        printf("Test passed: Output is as expected.\n");
    } else {
        printf("Test failed: Output does not match the expected content.\n");
    }

    // Clean up
    clean_up(OUTPUT_FILE);
    clean_up(LOCK_FILE);

    return 0;
}