#include "buffered_open.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Macro for error handling and cleanup
#define ERROR_AND_CLEANUP(msg, bf) do { perror(msg); buffered_close(bf); return NULL; } while (0)

// Function to open a buffered file
buffered_file_t *buffered_open(const char *pathname, int flags, ...) {
    // Check if preappend flag is set and clear it from flags
    int preappend = (flags & O_PREAPPEND) ? 1 : 0;
    flags &= ~O_PREAPPEND;

    // Allocate memory for buffered file structure
    buffered_file_t *bf = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (!bf) {
        perror("Memory allocation error");
        return NULL;
    }

    // Initialize structure members
    bf->fd = -1;
    bf->read_buffer = (char *)malloc(BUFFER_SIZE);
    bf->write_buffer = (char *)malloc(BUFFER_SIZE);
    bf->read_buffer_size = 0;
    bf->write_buffer_size = BUFFER_SIZE;
    bf->read_buffer_pos = 0;
    bf->write_buffer_pos = 0;
    bf->flags = flags;
    bf->preappend = preappend;

    if (!bf->read_buffer || !bf->write_buffer) {
        ERROR_AND_CLEANUP("Memory allocation error", bf);
    }

    // Handle optional mode argument for file permissions
    mode_t mode = 0;  // Default mode in case not provided
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = (mode_t)va_arg(args, int);  // Cast the argument to mode_t after va_arg
        va_end(args);
    }

    // Open the file and store the file descriptor
    bf->fd = open(pathname, flags, mode);
    if (bf->fd == -1) {
        ERROR_AND_CLEANUP("Error opening file", bf);
    }

    return bf;
}

// Function to write to a buffered file
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    if (!bf || !buf) {
        errno = EINVAL;
        return -1;
    }

    const char *buf_ptr = (const char *)buf;
    size_t bytes_written = 0;

    if (bf->preappend) {
        // Handle preappend logic
        off_t file_size = lseek(bf->fd, 0, SEEK_END);
        if (file_size == -1) {
            perror("Error getting file size");
            return -1;
        }

        char *temp_buffer = (file_size > 0) ? (char *)malloc(file_size) : NULL;
        if (file_size > 0 && !temp_buffer) {
            perror("Memory allocation error");
            return -1;
        }

        if (file_size > 0 && pread(bf->fd, temp_buffer, file_size, 0) != file_size) {
            perror("Error reading existing file content");
            free(temp_buffer);
            return -1;
        }

        if (pwrite(bf->fd, buf, count, 0) != (ssize_t)count) {
            perror("Error writing new data");
            free(temp_buffer);
            return -1;
        }

        if (temp_buffer && pwrite(bf->fd, temp_buffer, file_size, count) != file_size) {
            perror("Error appending existing content");
            free(temp_buffer);
            return -1;
        }

        free(temp_buffer);
        return count;
    } else {
        // Standard buffered write logic
        while (bytes_written < count) {
            size_t space_left = bf->write_buffer_size - bf->write_buffer_pos;
            size_t to_write = (count - bytes_written < space_left) ? count - bytes_written : space_left;

            memcpy(bf->write_buffer + bf->write_buffer_pos, buf_ptr + bytes_written, to_write);
            bf->write_buffer_pos += to_write;
            bytes_written += to_write;

            if (bf->write_buffer_pos == bf->write_buffer_size) {
                if (buffered_flush(bf) == -1) {
                    return -1;
                }
            }
        }

        return bytes_written;
    }
}

// Function to read from a buffered file
ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    if (!bf || !buf) {
        errno = EINVAL;
        return -1;
    }

    char *buf_ptr = (char *)buf;
    size_t bytes_read = 0;

    while (bytes_read < count) {
        if (bf->read_buffer_pos >= bf->read_buffer_size) {
            ssize_t bytes_refilled = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (bytes_refilled <= 0) {
                return bytes_read > 0 ? bytes_read : bytes_refilled;
            }
            bf->read_buffer_size = bytes_refilled;
            bf->read_buffer_pos = 0;
        }

        size_t data_left = bf->read_buffer_size - bf->read_buffer_pos;
        size_t to_read = (count - bytes_read < data_left) ? count - bytes_read : data_left;
        memcpy(buf_ptr + bytes_read, bf->read_buffer + bf->read_buffer_pos, to_read);
        bf->read_buffer_pos += to_read;
        bytes_read += to_read;
    }

    return bytes_read;
}

// Function to flush the write buffer
int buffered_flush(buffered_file_t *bf) {
    if (!bf || bf->fd == -1) {
        errno = EINVAL;
        return -1;
    }

    if (bf->write_buffer_pos > 0) {
        ssize_t bytes_written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
        if (bytes_written == -1) {
            perror("Error flushing buffer");
            return -1;
        }

        bf->write_buffer_pos = 0;
    }

    return 0;
}

// Function to close a buffered file
int buffered_close(buffered_file_t *bf) {
    if (!bf) {
        errno = EINVAL;
        return -1;
    }

    if (buffered_flush(bf) == -1) {
        return -1;
    }

    int result = close(bf->fd);

    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);

    return result;
}
