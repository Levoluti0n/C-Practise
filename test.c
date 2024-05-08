#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

void printProgressBar(double progress) {
    int barWidth = 70;
    printf(" %.2f%%", progress * 100);
    printf("[");
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            printf("=");
        else if (i == pos)
            printf(">");
        else
            printf(" ");
    }

    printf("]\r");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source_path = argv[1];
    const char *dest_path = argv[2];

    int source_fd = open(source_path, O_RDONLY);
    if (source_fd == -1) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    struct stat st; 
    if (fstat(source_fd, &st) == -1) {
        perror("Error getting source file size");
        close(source_fd);
        exit(EXIT_FAILURE);
    }

    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(source_fd);
        exit(EXIT_FAILURE);
    }

    off_t total_size = st.st_size;
    off_t bytes_copied = 0;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read); 
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            close(source_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
        bytes_copied += bytes_written;
        double progress = (double)bytes_copied / total_size;
        printProgressBar(progress);
    }

    if (bytes_read == -1) {
        perror("Error reading from source file");
        close(source_fd);
        close(dest_fd);
        exit(EXIT_FAILURE);
    }

    if (fsync(dest_fd) == -1) {
        perror("Error flushing data to destination file");
        close(source_fd);
        close(dest_fd);
        exit(EXIT_FAILURE);
    }

    close(source_fd);
    close(dest_fd);

    printf("\nFile copied successfully!\n");

    return 0;
}
