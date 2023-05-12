#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include "random.c"

#define MAX_FILES 100

typedef struct
{
    char file_name[256];
    long file_offset;
    long file_size;
} archive_file_header;

int create_archive_file(char *archive_file_name, char *source_dir_path, char **files_to_archive, int num_files);

int main()
{
    char archive_file_name[256], source_dir_path[256];
    char *files_to_archive[MAX_FILES];
    int num_files = 0;

    printf("Enter the archive file name: ");
    scanf("%s", archive_file_name);

    printf("Enter the path to the source directory: ");
    scanf("%s", source_dir_path);

    // recursively add all files and directories in the source directory to the files_to_archive array
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[256];
    dir = opendir(source_dir_path);
    if (dir == NULL)
    {
        printf("Error opening source directory\n");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            // ignore hidden files and directories
            continue;
        }
        sprintf(path, "%s/%s", source_dir_path, entry->d_name);
        if (stat(path, &statbuf) == -1)
        {
            printf("Error getting file stats\n");
            return 1;
        }
        if (S_ISDIR(statbuf.st_mode))
        {
            // add directory to files_to_archive and recursively add its contents
            files_to_archive[num_files] = malloc(strlen(path) + 2);
            sprintf(files_to_archive[num_files], "%s/", path);
            num_files++;
            create_archive(archive_file_name, path, files_to_archive, num_files);
            num_files--;
        }
        else
        {
            // add file to files_to_archive
            files_to_archive[num_files] = malloc(strlen(path) + 1);
            strcpy(files_to_archive[num_files], path);
            num_files++;
        }
    }
    closedir(dir);

    // create the archive file
    create_archive(archive_file_name, source_dir_path, files_to_archive, num_files);

    // free memory
    for (int i = 0; i < num_files; i++)
    {
        free(files_to_archive[i]);
    }

    return 0;
}
