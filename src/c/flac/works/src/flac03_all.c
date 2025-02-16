#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024
#define OUTPUT_FILE "01sourcefiles.rst"
#define SECOND_OUTPUT_FILE "02targetfiles.rst"

// Define the file extensions to look for
#define FILE_EXTENSIONS ".flac,.wav,.aiff,.aif"

#define COMMAND_FORMAT "/usr/bin/flac -l 0 --disable-constant-subframes --disable-fixed-subframes --no-preserve-modtime -V -o %s %s"

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
        }
    }

    // Close source directory
    closedir(src_dir);

    return 0;
}

void scan_directory(const char *dir_path, FILE *output_file, const char *file_extensions) {
    struct dirent **entries;
    int num_entries;

    // Open directory
    num_entries = scandir(dir_path, &entries, NULL, alphasort);
    if (num_entries == -1) {
        fprintf(stderr, "Error opening directory '%s': %s\n", dir_path, strerror(errno));
        return;
    }

    // Iterate over directory entries
    for (int i = 0; i < num_entries; i++) {
        // Skip current and parent directories
        if (strcmp(entries[i]->d_name, ".") == 0 || strcmp(entries[i]->d_name, "..") == 0) {
            continue;
        }

        // Construct entry path
        char entry_path[MAX_PATH_LENGTH];
        sprintf(entry_path, "%s/%s", dir_path, entries[i]->d_name);

        // Check if entry is a file
        struct stat statbuf;
        if (stat(entry_path, &statbuf) == 0) {
            if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
                // Check if file has one of the specified extensions
                char *extensions = strdup(file_extensions);
                char *extension = strtok(extensions, ",");
                while (extension != NULL) {
                    if (strstr(entries[i]->d_name, extension) != NULL) {
                        // Write file path to output file
                        fprintf(output_file, "%s\n", entry_path);
                        break;
                    }
                    extension = strtok(NULL, ",");
                }
                free(extensions);
            } else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
                // Recursively scan subdirectory
                scan_directory(entry_path, output_file, file_extensions);
            }
        } else {
            fprintf(stderr, "Error scanning file: %s\n", entry_path);
        }

        free(entries[i]);
    }

    free(entries);
}

void replace_directory_path(const char *original_dir, const char *replacement_dir, const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    if (input == NULL) {
        fprintf(stderr, "Error opening input file '%s': %s\n", input_file, strerror(errno));
        return;
    }

    FILE *output = fopen(output_file, "w");
    if (output == NULL) {
        fprintf(stderr, "Error opening output file '%s': %s\n", output_file, strerror(errno));
        fclose(input);
        return;
    }

    char line[MAX_PATH_LENGTH];
    while (fgets(line, MAX_PATH_LENGTH, input) != NULL) {
        // Remove trailing newline character
        line[strcspn(line, "\n")] = 0;

        // Replace original directory path with replacement directory path
        char modified_line[MAX_PATH_LENGTH];
        sprintf(modified_line, "%s%s", replacement_dir, line + strlen(original_dir));

        // Write modified line to output file
        fprintf(output, "%s\n", modified_line);
    }

    fclose(input);
    fclose(output);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <replacement_dir>\n", argv[0]);
        return 1;
    }
    
    if (clone_directory(argv[1], argv[2]) != 0) {
        fprintf(stderr, "Failed to clone directory structure\n");
        return 1;
    }

    // Open output file
    FILE *output_file = fopen(OUTPUT_FILE, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output file '%s': %s\n", OUTPUT_FILE, strerror(errno));
        return 1;
    }

    printf("Scanning directory '%s'...\n", argv[1]);

    // Scan directory
    scan_directory(argv[1], output_file, ".flac,.wav,.aiff,.aif");

    printf("Scan complete.\n");

    // Close output file
    fclose(output_file);

    // Replace directory path in output file
    replace_directory_path(argv[1], argv[2], OUTPUT_FILE, SECOND_OUTPUT_FILE);

    printf("Modified file list written to '%s'.\n", SECOND_OUTPUT_FILE);

    // section to convert to flac

    // Open output files
    FILE *output_file02 = fopen(OUTPUT_FILE, "r");
    if (output_file02 == NULL) {
        fprintf(stderr, "Error opening output file '%s': %s\n", OUTPUT_FILE, strerror(errno));
        return 1;
    }

    FILE *second_output_file02 = fopen(SECOND_OUTPUT_FILE, "r");
    if (second_output_file02 == NULL) {
        fprintf(stderr, "Error opening second output file '%s': %s\n", SECOND_OUTPUT_FILE, strerror(errno));
        fclose(output_file02);
        return 1;
    }

    char source_file_path[MAX_PATH_LENGTH];
    char target_file_path[MAX_PATH_LENGTH];
    char command[MAX_PATH_LENGTH];

    // Read files from output files
    while (fgets(source_file_path, MAX_PATH_LENGTH, output_file02) != NULL &&
           fgets(target_file_path, MAX_PATH_LENGTH, second_output_file02) != NULL) {
        // Remove trailing newline characters
        source_file_path[strcspn(source_file_path, "\n")] = 0;
        target_file_path[strcspn(target_file_path, "\n")] = 0;

        // Construct the command
        sprintf(command, COMMAND_FORMAT, target_file_path, source_file_path);

        // Call the command using system()
        printf("Executing command: %s\n", command);
        system(command);
    }

    // Close output files
    fclose(output_file02);
    fclose(second_output_file02);

    return 0;
}
