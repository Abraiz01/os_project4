#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <libgen.h>


#define MAX_FILES 1000

// Define struct header and metadata here
struct header {
    unsigned int meta_data_offset;
    int metadata_length;
};

struct metadata{
    char file_name[100];
    unsigned int file_size;
    unsigned int file_offset;
};

    struct node {
        char *name;
        struct node *parent;
        struct node *children[MAX_FILES];
        int num_children;
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
void read_archive_metadata(char *archiveFileName);
void display_hierarchy(char *archiveFileName);
void display_node(struct node *node, int depth);
void append_file(char *archiveFileName, char *filename);

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
    //display_hierarchy(archiveFileName);
    append_file(archiveFileName, "7.txt");
    read_archive_metadata(archiveFileName);
    extract_archive(archiveFileName);

    return 0;
}

// Function to create archive file
void create_archive(char *archiveFileName, char **listoffilesandfolders,int numfiles){
    metadatalength = 0;
    totalfilesize = 0;
    struct header archiveHeader;

    filefp = fopen(archiveFileName, "wb");
    if (filefp == NULL)
    {
        printf("Error: Could not create archive file\n");
        exit(1);
    }

    // Write archive header
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    fwrite(&archiveHeader, sizeof(archiveHeader), 1, filefp);

    // Recursively add files/directories to metadata array
    for(int i = 0; i<numfiles;i++){
        ftw(listoffilesandfolders[i],list,1);
    }

    // Write metadata to archive file
    for(int i = 0 ; i<metadatalength;i++){
        fwrite(&metadata[i], sizeof(metadata[i]), 1, filefp);
    }

    // Update header with metadata offset
    archiveHeader.meta_data_offset = totalfilesize + sizeof(totalfilesize)+ sizeof(archiveHeader);
    archiveHeader.metadata_length = metadatalength;
    fseek(filefp, 0, SEEK_SET);
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    printf("CREATE Total file size: %d\n", totalfilesize);
    fwrite(&archiveHeader, sizeof(archiveHeader), 1, filefp);
    printf("CREATE Metadata offset: %d\n", archiveHeader.meta_data_offset);

    fclose(filefp);
}

// Function to add file metadata to metadata array and write file content to archive file
int list(const char *fpath, const struct stat *sb, int typeflag) {
    if (typeflag == FTW_F) {
        // If file, add metadata to metzsadata array
        metadata = realloc(metadata, (metadatalength+1)*sizeof(struct metadata));
        strcpy(metadata[metadatalength].file_name, fpath);
        metadata[metadatalength].file_size = sb->st_size;
        metadata[metadatalength].file_offset = totalfilesize + sizeof(totalfilesize) + sizeof(struct header);
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
   filefp = fopen(archiveFileName, "rb");
    if (filefp == NULL) {
        printf("Error: Could not open archive file\n");
        exit(1);
    }

    // Read archive header
    struct header reader_header;
    fread(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    printf("reader_header.totalfilesize: %u\n", totalfilesize);
    fread(&reader_header, sizeof(struct header), 1, filefp);

    // Read metadata from archive file
    fseek(filefp, reader_header.meta_data_offset, SEEK_SET);

    printf("reader_header.meta_data_offset: %u\n", reader_header.meta_data_offset);
    printf("reader_header.metadata_length: %u\n", reader_header.metadata_length);

    printf("metadatalength: %d\n", metadatalength);

    metadata = malloc(metadatalength * sizeof(struct metadata));
    fread(metadata, sizeof(struct metadata), metadatalength, filefp);

// Extract files and directories
    for (int i = 0; i < metadatalength; i++) {
        char *filename = metadata[i].file_name;
        unsigned int filesize = metadata[i].file_size;
        unsigned int fileoffset = metadata[i].file_offset;

        printf("EXTRACT fileoffset: %u\n", fileoffset);

        // Create parent directories if necessary
        char *parentdir = strdup(filename);
        char *dirpath = dirname(parentdir);
        if (strcmp(dirpath, ".") != 0) {
            char cmd[1000];
            sprintf(cmd, "mkdir -p \"%s\"", dirpath);
            system(cmd);
        }

        // Extract file
        FILE *outfile = fopen(filename, "wb");
        if (outfile == NULL) {
            printf("Error: Could not create file %s\n", filename);
            exit(1);
        }
        fseek(filefp, fileoffset, SEEK_SET);
        char buffer[1024];
        size_t bytes_read;
        unsigned int bytes_left = filesize;
        while (bytes_left > 0 && (bytes_read = fread(buffer, 1, bytes_left, filefp)) > 0) {
            fwrite(buffer, bytes_read, 1, outfile);
            bytes_left -= bytes_read;        
        }
        fclose(outfile);
    }

    close(filefp);
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


// Define read_archive_metadata() function here
void read_archive_metadata(char *archiveFileName) {
    filefp = fopen(archiveFileName, "rb");
    if (filefp == NULL) {
        printf("Error: Could not open archive file\n");
        exit(1);
    }

    // Read archive header
    struct header reader_header;
    fread(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    printf("reader_header.totalfilesize: %u\n", totalfilesize);
    fread(&reader_header, sizeof(struct header), 1, filefp);

    // Read metadata from archive file
    fseek(filefp, reader_header.meta_data_offset, SEEK_SET);

    printf("reader_header.meta_data_offset: %u\n", reader_header.meta_data_offset);
    printf("reader_header.metadata_length: %u\n", reader_header.metadata_length);

    printf("metadatalength: %d\n", metadatalength);

    metadata = malloc(metadatalength * sizeof(struct metadata));
    fread(metadata, sizeof(struct metadata), metadatalength, filefp);

    printf("Metadata:\n");
    for (int i = 0; i < metadatalength; i++) {
        printf("File name: %s\n", metadata[i].file_name);
        printf("File size: %u\n", metadata[i].file_size);
        printf("File offset: %u\n", metadata[i].file_offset);
    }

    fclose(filefp);
}

void display_hierarchy(char *archiveFileName) {
    filefp = fopen(archiveFileName, "rb");
    if (filefp == NULL) {
        printf("Error: Could not open archive file\n");
        exit(1);
    }

    // Read archive header
    struct header reader_header;
    fread(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    printf("reader_header.totalfilesize: %u\n", totalfilesize);
    fread(&reader_header, sizeof(struct header), 1, filefp);

    // Read metadata from archive file
    fseek(filefp, reader_header.meta_data_offset, SEEK_SET);

    printf("reader_header.meta_data_offset: %u\n", reader_header.meta_data_offset);
    printf("reader_header.metadata_length: %u\n", reader_header.metadata_length);

       // Build hierarchy
    char *root = "/";

    struct node *rootnode = (struct node*) malloc(sizeof(struct node));
    rootnode->name = root;
    rootnode->parent = NULL;
    rootnode->num_children = 0;

    for(int i = 0; i < metadatalength; i++) {
        char *filepath = metadata[i].file_name;
        char *path = strtok(filepath, "/");
        struct node *parent = rootnode;
        while (path != NULL) {
            int found_child = 0;
            for(int j = 0; j < parent->num_children; j++) {
                if (strcmp(parent->children[j]->name, path) == 0) {
                    parent = parent->children[j];
                    found_child = 1;
                    break;
                }
            }
            if (!found_child) {
                struct node *newnode = (struct node*) malloc(sizeof(struct node));
                newnode->name = strdup(path);
                newnode->parent = parent;
                newnode->num_children = 0;
                parent->children[parent->num_children] = newnode;
                parent->num_children++;
                parent = newnode;
            }
            path = strtok(NULL, "/");
        }
    }

    // Print hierarchy
    printf("%s\n", rootnode->name);
    for(int i = 0; i < rootnode->num_children; i++) {
        struct node *child = rootnode->children[i];
        display_node(child, 1);
    }

    // Free memory
    free(metadata);
    // free_node(rootnode);
    fclose(filefp);
}

void display_node(struct node *node, int depth) {
    for(int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%s\n", node->name);
    for(int i = 0; i < node->num_children; i++) {
        struct node *child = node->children[i];
        display_node(child, depth+1);
    }
}

void append_file(char *archiveFileName, char *filename){
    filefp = fopen(archiveFileName, "rb");
    if (filefp == NULL) {
        printf("Error: Could not open archive file\n");
        exit(1);
    }

    // Read archive header
    struct header reader_header;
    fread(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    printf("reader_header.totalfilesize: %u\n", totalfilesize);
    fread(&reader_header, sizeof(struct header), 1, filefp);

    // Read metadata from archive file
    fseek(filefp, reader_header.meta_data_offset, SEEK_SET);
    unsigned int old_meta_data_offset = reader_header.meta_data_offset;

    printf("reader_header.meta_data_offset: %u\n", reader_header.meta_data_offset);
    printf("reader_header.metadata_length: %u\n", reader_header.metadata_length);

    printf("metadatalength: %d\n", metadatalength);

    metadata = malloc(metadatalength * sizeof(struct metadata));
    fread(metadata, sizeof(struct metadata), metadatalength, filefp);

    printf("Metadata:\n");
    for (int i = 0; i < metadatalength; i++) {
        printf("File name: %s\n", metadata[i].file_name);
        printf("File size: %u\n", metadata[i].file_size);
        printf("File offset: %u\n", metadata[i].file_offset);
    }

    // fclose(filefp);

     printf("1\n");

    // Open file to append
    FILE *appendfp = fopen(filename, "rb");
    if (appendfp == NULL) {
        printf("Error: Could not open file to append\n");
        exit(1);
    }
    printf("2\n");

    // Get file size
    fseek(appendfp, 0, SEEK_END);
    unsigned int filesize = ftell(appendfp);
    fseek(appendfp, 0, SEEK_SET);
    printf("3\n");

    // Get file name
    char *filenameptr = strrchr(filename, '/');
    if (filenameptr == NULL) {
        filenameptr = filename;
    } else {
        filenameptr++;
    }
    printf("4\n");

    // Create new metadata entry
    metadata = realloc(metadata, (metadatalength+1)*sizeof(struct metadata));
    strcpy(metadata[metadatalength].file_name, filenameptr);
    metadata[metadatalength].file_size = filesize;
    metadata[metadatalength].file_offset = totalfilesize + sizeof(totalfilesize) + sizeof(struct header);
    metadatalength++;
    totalfilesize += filesize;
    printf("5\n");

    // Write new
    filefp = fopen(archiveFileName, "wb");
    if (filefp == NULL) {
        printf("Error: Could not open archive file\n");
        exit(1);
    }
    printf("6\n");


    // Write new header
    struct header writer_header;
    writer_header.meta_data_offset = totalfilesize + sizeof(totalfilesize) + sizeof(struct header);
    writer_header.metadata_length = metadatalength;
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    fwrite(&writer_header, sizeof(struct header), 1, filefp);

    fseek(filefp, old_meta_data_offset-1, SEEK_SET);
    printf("7\n");

    char buffer[1024];
    size_t bytes_read;
    unsigned int bytes_left = filesize;
    while (bytes_left > 0 && (bytes_read = fread(buffer, 1, bytes_left, appendfp)) > 0) {
        fwrite(buffer, bytes_read, 1, filefp);
        bytes_left -= bytes_read; 
    }
    printf("8\n");

    // Write new metadata
    fseek(filefp, writer_header.meta_data_offset, SEEK_SET);
    for(int i = 0 ; i<metadatalength;i++){
        fwrite(&metadata[i], sizeof(metadata[i]), 1, filefp);
    }
    printf("9\n");

    fclose(filefp);

    // Free memory  
    fclose(appendfp);

}

