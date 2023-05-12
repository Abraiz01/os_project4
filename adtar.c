#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "adtar.helper.c"
#define MAX_FILES 100

typedef struct {
    char file_name[256];
    long file_offset;
    long file_size;
} archive_file_header;

// make the file structure proper. Make the header include meta data address, make the meta data part. 

int main()
{
    char option[3], archive_file[100], file_dir_list[100];

    printf("Enter parameters in the format {-c|-a|-x|-m|-p} <archive-file> <file/directory list>\n");
    scanf("%s %s %s", option, archive_file, file_dir_list);

    printf("option: %s\n", option);
    printf("archive_file: %s\n", archive_file);
    printf("file_dir_list: %s\n", file_dir_list);

    char *files_to_archive[MAX_FILES];
    int num_files = 0;

    if (strcmp(option, "-c") == 0)
    {
        // Handle -c option

      DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[256];
    dir = opendir(file_dir_list);
    if (dir == NULL) {
        printf("Error opening source directory\n");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            // ignore hidden files and directories
            continue;
        }
        sprintf(path, "%s/%s", file_dir_list, entry->d_name);
        if (stat(path, &statbuf) == -1) {
            printf("Error getting file stats\n");
            return 1;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            // add directory to files_to_archive and recursively add its contents
            files_to_archive[num_files] = malloc(strlen(path) + 2);
            sprintf(files_to_archive[num_files], "%s/", path);
            num_files++;
            create_archive(archive_file, path, files_to_archive, num_files);
            num_files--;
        } else {
            // add file to files_to_archive
            files_to_archive[num_files] = malloc(strlen(path) + 1);
            strcpy(files_to_archive[num_files], path);
            num_files++;
        }
    }
    closedir(dir);

    // create the archive file
    create_archive(archive_file, file_dir_list, files_to_archive, num_files);

    // free memory
    for (int i = 0; i < num_files; i++) {
        free(files_to_archive[i]);
    }

    }
    else if (strcmp(option, "-a") == 0)
    {
        // Handle -a option
    }
    else if (strcmp(option, "-x") == 0)
    {
        // Handle -x option
    }
    else if (strcmp(option, "-m") == 0)
    {
        // Handle -m option
    }
    else if (strcmp(option, "-p") == 0)
    {
        // Handle -p option
    }
    else
    {
        printf("Invalid option selected.\n");
        return 1;
    }

    // Do something with the archive_file and file_dir_list variables
    // based on the selected option

    return 0;
}

