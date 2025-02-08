#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_PROCESSES 10

void copy_file(const char *src, const char *dest) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        perror("open source file");
        exit(EXIT_FAILURE);
    }

    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("open destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    size_t total_bytes = 0;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("write error");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
        total_bytes += bytes_written;
    }

    printf("Process %d copied %s (%zu bytes)\n", getpid(), src, total_bytes);
    close(src_fd);
    close(dest_fd);
    exit(EXIT_SUCCESS);
}

void sync_directories(const char *dir1, const char *dir2, int max_processes) {
    DIR *d = opendir(dir1);
    if (!d) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    struct stat st;
    int active_processes = 0;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_REG) {
            char src_path[1024], dest_path[1024];
            snprintf(src_path, sizeof(src_path), "%s/%s", dir1, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", dir2, entry->d_name);

            if (stat(dest_path, &st) == -1 && errno == ENOENT) {
                pid_t pid = fork();
                if (pid == 0) {
                    copy_file(src_path, dest_path);
                } else if (pid > 0) {
                    active_processes++;
                    if (active_processes >= max_processes) {
                        wait(NULL);
                        active_processes--;
                    }
                } else {
                    perror("fork");
                }
            }
        }
    }

    while (active_processes > 0) {
        wait(NULL);
        active_processes--;
    }

    closedir(d);
}

int main() {
    char dir1[256], dir2[256];
    int max_processes;

    printf("Enter source directory: ");
    scanf("%255s", dir1);
    printf("Enter destination directory: ");
    scanf("%255s", dir2);
    printf("Enter max number of concurrent processes: ");
    scanf("%d", &max_processes);

    if (max_processes <= 0 || max_processes > MAX_PROCESSES) {
        fprintf(stderr, "Invalid number of processes. Using default: %d\n", MAX_PROCESSES);
        max_processes = MAX_PROCESSES;
    }

    sync_directories(dir1, dir2, max_processes);
    return 0;
}
