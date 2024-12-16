# Part 1: Advanced Synchronization of File Access with Naive Methods

## **Objective**
This program demonstrates the challenges of synchronizing file access between parent and child processes using naive methods such as `wait` or `sleep`, with a more complex scenario involving multiple processes and varied writing patterns. The program should also handle input parameters for testing purposes.

## **Instructions**
The program forks two child processes and the parent process, each writing specified messages to a file (`output.txt`) a specified number of times. Naive synchronization ensures that writes do not overlap using simple timing mechanisms.

## **Example Usage**
```bash
./part1 "Parent message\n" "Child1 message\n" "Child2 message\n" 3
```
This command runs the program with the following parameters:
- **Parent message**: The message that the parent process writes to the file.
- **Child1 message**: The message that the first child process writes to the file.
- **Child2 message**: The message that the second child process writes to the file.
- **3**: The number of times each process writes its respective message to the file.

The output will be written to `output.txt`, with each message appearing the specified number of times, ordered according to the naive synchronization logic to prevent interleaving.

# Part 2: Assignment: Implementing a Synchronization Lock for File Access

## **Objective**
This program demonstrates the implementation of a synchronization lock for file access, ensuring that only one process writes to the file at a time while the others wait. This assignment involves creating a dynamic number of child processes and using a separate file as a lock to control access.

## **Instructions**
The program forks a dynamic number of child processes along with the parent process, each writing specified messages to a file (`output.txt`) a specified number of times. A synchronization lock ensures that only one process writes to the file at a time while others wait for access, preventing interleaved writes.

## **Example Usage**
```bash
./part2 "Parent message\n" "Child1 message\n" "Child2 message\n" 3
```
This command runs the program with the following parameters:
- **Parent message**: The message that the parent process writes to the file.
- **Child1 message**: The message that the first child process writes to the file.
- **Child2 message**: The message that the second child process writes to the file.
- **3**: The number of times each process writes its respective message to the file.

The output will be written to `output.txt`, with each message appearing the specified number of times, ordered according to the synchronization lock logic to prevent interleaving.

# Part 3: Buffered File I/O with O_PREAPPEND Flag

## **Objective**
This part expands the project by introducing buffered file I/O with enhanced functionality for the `O_PREAPPEND` flag. The goal is to create custom wrappers for standard file operations that enable efficient buffered reading and writing. The program ensures that new data can be prepended to files without overriding existing content, providing a flexible and efficient approach to file manipulation.

## **Example Usage**
```c
#include "buffered_open.h"
#include <stdio.h>
#include <string.h>

int main() {
    buffered_file_t *bf = buffered_open("example.txt", O_WRONLY | O_CREAT | O_PREAPPEND, 0644);
    if (!bf) {
        perror("buffered_open");
        return 1;
    }

    const char *text = "Hello, World!";
    if (buffered_write(bf, text, strlen(text)) == -1) {
        perror("buffered_write");
        buffered_close(bf);
        return 1;
    }

    if (buffered_close(bf) == -1) {
        perror("buffered_close");
        return 1;
    }

    return 0;
}
```
This program demonstrates how to create and use a buffered file with support for the `O_PREAPPEND` flag. It showcases how to open a file, write data to it, and close it using custom buffered file functions, ensuring that new data is prepended to the existing content of the file.

