
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
    metadata = malloc(metadatalength*sizeof(Filemetadata));
    for(int i = 0; i<numfiles;i++){
        ftw(listoffilesandfolders[i],list,1);
    }

    fwrite(&lengthoffetadata, sizeof(lengthoffetadata), 1, filefp);
    for(int i = 0 ; i<metadatalength;i++){
        fwrite(&metadata[i], sizeof(metadata[i]), 1, filefp);
    }
    fseek(filefp, 0, SEEK_SET);
    fwrite(&totalfilesize, sizeof(totalfilesize), 1, filefp);
    fclose(filefp);

}


void ftw(char *path, char **list, int depth)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int i;
    if (lstat(path, &statbuf) < 0)
    {
        printf("Error: Could not open file %s for archiving\n", path);
        exit(1);
    }
    if (S_ISDIR(statbuf.st_mode) == 0)
    {
        // Write file contents to archive
        unsigned int file_size = statbuf.st_size;
        char *buffer = malloc(file_size);
        fread(buffer, file_size, 1, filefp);
        fwrite(buffer, file_size, 1, filefp);
        free(buffer);

        // Add file header to archive header array
        strncpy(archive_file_headers[archive_file_headers_index].file_name, file_path, 100);
        archive_file_headers[archive_file_headers_index].file_size = file_size;
        archive_file_headers[archive_file_headers_index].file_offset = offset;
        archive_file_headers_index++;

        // Update offset for next file
        offset += file_size;

        fclose(file_to_archive);
    }
    else if (S_ISDIR(statbuf.st_mode))
    {
        if ((dp = opendir(path)) == NULL)
        {
            printf("Error: Could not open directory %s for archiving\n", path);
            exit(1);
        }
        while ((dirp = readdir(dp)) != NULL)
        {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            {
                continue;
            }
            char *newpath = malloc(strlen(path) + strlen(dirp->d_name) + 2);
            strcpy(newpath, path);
            strcat(newpath, "/");
            strcat(newpath, dirp->d_name);
            ftw(newpath, list, depth + 1);
            free(newpath);
        }
        closedir(dp);
    }
}