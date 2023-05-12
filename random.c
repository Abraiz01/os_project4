#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define ARCHIVE_HEADER_SIZE 512

struct archive_file_header {
    char file_name[100];
    unsigned int file_size;
    unsigned int file_offset;
};

void create_archive_file(const char* archive_file_name, const char* source_dir_path, char** files_to_archive, int num_files) {
    FILE* archive_file = fopen(archive_file_name, "wb");
    if (archive_file == NULL) {
        printf("Error: Could not create archive file\n");
        exit(1);
    }

    // Write archive header
    char header[ARCHIVE_HEADER_SIZE];
    memset(header, 0, ARCHIVE_HEADER_SIZE);
    fwrite(header, ARCHIVE_HEADER_SIZE, 1, archive_file);

    // Write files to archive
    unsigned int offset = ARCHIVE_HEADER_SIZE;
    struct archive_file_header archive_file_headers[num_files];
    int archive_file_headers_index = 0;

    for (int i = 0; i < num_files; i++) {
        char* file_path = files_to_archive[i];
        struct stat stat_buf;
        stat(file_path, &stat_buf);

        if (S_ISREG(stat_buf.st_mode)) { // Regular file
            FILE* file_to_archive = fopen(file_path, "rb");
            if (file_to_archive == NULL) {
                printf("Error: Could not open file %s for archiving\n", file_path);
                exit(1);
            }

            // Write file contents to archive
            unsigned int file_size = stat_buf.st_size;
            char* buffer = malloc(file_size);
            fread(buffer, file_size, 1, file_to_archive);
            fwrite(buffer, file_size, 1, archive_file);
            free(buffer);

            // Add file header to archive header array
            strncpy(archive_file_headers[archive_file_headers_index].file_name, file_path, 100);
            archive_file_headers[archive_file_headers_index].file_size = file_size;
            archive_file_headers[archive_file_headers_index].file_offset = offset;
            archive_file_headers_index++;

            // Update offset for next file
            offset += file_size;

            fclose(file_to_archive);
        } else if (S_ISDIR(stat_buf.st_mode)) { // Directory
            DIR* dir = opendir(file_path);
            if (dir == NULL) {
                printf("Error: Could not open directory %s for archiving\n", file_path);
                exit(1);
            }

            // Recursively archive contents of directory
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char sub_path[1000];
                    snprintf(sub_path, sizeof(sub_path), "%s/%s", file_path, entry->d_name);
                    char* sub_files_to_archive[1];
                    sub_files_to_archive[0] = sub_path;
                    create_archive_file(archive_file_name, source_dir_path, sub_files_to_archive, 1);
                }
            }

            closedir(dir);
        }
    }

    // Write archive header dictionary
    fwrite(&archive_file_headers, sizeof(struct archive_file_header), num_files, archive_file);

    fclose(archive_file);
}
