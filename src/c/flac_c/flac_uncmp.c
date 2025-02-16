#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024

// Function to clone directory structure
static int clone_directory(const char *src_path, const char *dst_path) {
    int ret = 0;

    // Create destination directory
    ret = mkdir(dst_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    // Clone directory structure
    ret = nftw(src_path, NULL, 0, FTW_PHYS);
    if (ret != 0) {
        perror("nftw");
        return -1;
    }

    return 0;
}

// Callback function for nftw
static int callback(const char *path, const struct stat *sb, int flag, struct FTW *ftwbuf) {
    char dst_path[MAX_PATH_LENGTH];
    char *src_dir, *dst_dir;

    // Get source and destination directory paths
    src_dir = strdup(path);
    dst_dir = strdup(ftwbuf->base);

    // Construct destination path
    sprintf(dst_path, "%s/%s", dst_dir, src_dir + ftwbuf->base);

    // Create destination directory
    if (flag == FTW_D) {
        mkdir(dst_path, sb->st_mode);
    }

    free(src_dir);
    free(dst_dir);

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
