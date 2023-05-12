#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>

#define MAX_FILES 1000

// Define struct header and metadata here
struct header {
    unsigned int meta_data_offset;
};

struct metadata{
    char file_name[100];
    unsigned int file_size;
    unsigned int file_offset;
};

// Global variables
struct metadata *metadata = NULL;
int metadatalength = 0;
unsigned int totalfilesize = 0;
FILE *filefp = NULL;

// Define create_archive() function here
void create_archive(char *archiveFileName, char **listoffilesandfolders,int numfiles);
int add_metadata(const char *fpath, const struct stat *sb, int typeflag);
int list(const char *name, const struct stat *status, int type);
void extract_archive(char *archiveFileName);

int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        printf("Usage: adzip -c <archive-file> <file/directory list>\n");
        exit(1);
    }

    if(strcmp(argv[1], "-c") != 0)
    {
        printf("Error: Invalid flag specified. Only -c flag is supported.\n");
        exit(1);
    }

    char *archiveFileName = argv[2];
    char **listoffilesandfolders = argv+3;
    int numfiles = argc-3;

    create_archive(archiveFileName, listoffilesandfolders, numfiles);
    extract_archive(archiveFileName);

    return 0;
}

// Function to create archive file
void create_archive(char *archiveFileName, char **listoffilesandfolders,int numfiles){
    metadatalength = 0;
    totalfilesize = 0;

    filefp = fopen(archiveFileName, "wb");
    if (filefp == NULL)
    {
        printf("Error: Could not create archive file\n");
        exit(1);
    }

    // Write archive header
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);

    // Recursively add files/directories to metadata array
    for(int i = 0; i<numfiles;i++){
        ftw(listoffilesandfolders[i],list,1);
    }

    // Write metadata to archive file
    fwrite(&metadatalength, sizeof(metadatalength), 1, filefp);
    for(int i = 0 ; i<metadatalength;i++){
        fwrite(&metadata[i], sizeof(metadata[i]), 1, filefp);
    }

    // Update header with metadata offset
    struct header archiveHeader;
    archiveHeader.meta_data_offset = sizeof(totalfilesize) + sizeof(metadatalength) + (metadatalength * sizeof(struct metadata));
    fseek(filefp, 0, SEEK_SET);
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    fwrite(&archiveHeader, sizeof(archiveHeader), 1, filefp);

    fclose(filefp);
}

// Function to add file metadata to metadata array and write file content to archive file
int list(const char *fpath, const struct stat *sb, int typeflag) {
    if (typeflag == FTW_F) {
        // If file, add metadata to metadata array
        metadata = realloc(metadata, (metadatalength+1)*sizeof(struct metadata));
        strcpy(metadata[metadatalength].file_name, fpath);
        metadata[metadatalength].file_size = sb->st_size;
        metadata[metadatalength].file_offset = totalfilesize;
        metadatalength++;
        
        // Open file for reading
        FILE *file = fopen(fpath, "rb");
        if (file == NULL) {
            printf("Error: Could not open file %s\n", fpath);
            exit(1);
        }
        
        // Read file content and write it to archive file
        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            fwrite(buffer, 1, bytes_read, filefp);
            totalfilesize += bytes_read;
        }
        
        // Close file
        fclose(file);
    }
    return 0;
}

void extract_archive(char *archiveFileName) {
    FILE *filefp = fopen(archiveFileName, "rb");
    if (filefp == NULL)
    {
        printf("Error: Could not open archive file\n");
        exit(1);
    }

    // Read archive header to get metadata offset
    unsigned int metadata_offset;
    fread(&metadata_offset, sizeof(metadata_offset), 1, filefp);

    // Read number of metadata entries
    int num_entries;
    fread(&num_entries, sizeof(num_entries), 1, filefp);

    // Allocate memory for metadata array
    struct metadata *metadata = malloc(num_entries * sizeof(struct metadata));

    // Read metadata from archive file
    fread(metadata, sizeof(struct metadata), num_entries, filefp);

    // Extract files from archive
    for (int i = 0; i < num_entries; i++) {
        // Seek to the offset of the file content in the archive
        fseek(filefp, metadata[i].file_offset + metadata_offset, SEEK_SET);

        // Open file for writing
        FILE *outfile = fopen(metadata[i].file_name, "wb");
        if (outfile == NULL) {
            printf("Error: Could not create file %s\n", metadata[i].file_name);
            exit(1);
        }

        // Read file content from archive and write it to the output file
        char buffer[1024];
        size_t bytes_read;
        unsigned int bytes_remaining = metadata[i].file_size;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), filefp)) > 0 && bytes_remaining > 0) {
            if (bytes_read > bytes_remaining) {
                bytes_read = bytes_remaining;
            }
            fwrite(buffer, 1, bytes_read, outfile);
            bytes_remaining -= bytes_read;
        }

        // Close output file
        fclose(outfile);
    }

    // Free metadata array
    free(metadata);

    // Close archive file
    fclose(filefp);
}

// // Function to add file metadata to metadata array
// int list(const char *fpath, const struct stat *sb, int typeflag) {
//     if (typeflag == FTW_F) {
//         // If file, add metadata to metadata array
//         metadata = realloc(metadata, (metadatalength+1)*sizeof(struct metadata));
//         strcpy(metadata[metadatalength].file_name, fpath);
//         metadata[metadatalength].file_size = sb->st_size;
//         metadata[metadatalength].file_offset = totalfilesize;
//         metadatalength++;
//         totalfilesize += sb->st_size;
//     }
//     return 0;
// }


