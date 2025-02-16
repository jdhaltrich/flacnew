#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024

// Function to clone directory structure
static int clone_directory(const char *src_path, const char *dst_path) {
    DIR *src_dir;
    struct dirent *entry;
    char src_entry_path[MAX_PATH_LENGTH];
    char dst_entry_path[MAX_PATH_LENGTH];

    // Open source directory
    src_dir = opendir(src_path);
    if (src_dir == NULL) {
        perror("opendir");
        return -1;
    }

    // Create destination directory
    mkdir(dst_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // Iterate over source directory entries
    while ((entry = readdir(src_dir)) != NULL) {
        // Skip current and parent directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct source and destination entry paths
        sprintf(src_entry_path, "%s/%s", src_path, entry->d_name);
        sprintf(dst_entry_path, "%s/%s", dst_path, entry->d_name);

        // Check if entry is a directory
        struct stat statbuf;
        lstat(src_entry_path, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively clone directory
            clone_directory(src_entry_path, dst_entry_path);
        } else {
            // Copy file
            FILE *src_file, *dst_file;
            src_file = fopen(src_entry_path, "r");
            if (src_file == NULL) {
                perror("fopen");
                continue;
            }

            dst_file = fopen(dst_entry_path, "w");
            if (dst_file == NULL) {
                perror("fopen");
                fclose(src_file);
                continue;
            }

            char buffer[1024];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, 1024, src_file)) > 0) {
                fwrite(buffer, 1, bytes_read, dst_file);
            }

            fclose(src_file);
            fclose(dst_file);
        }
    }

    // Close source directory
    closedir(src_dir);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <target_dir>\n", argv[0]);
        return 1;
    }

    if (clone_directory(argv[1], argv[2]) != 0) {
        fprintf(stderr, "Failed to clone directory structure\n");
        return 1;
    }

    return 0;
}
