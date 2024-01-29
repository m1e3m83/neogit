#include "neogit.h"

void add(char *fileName, char mode)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);

    char filedir[DIRNAME_LEN];
    GetCurrentDirectory(DIRNAME_LEN, filedir);
    dirChange(filedir, fileName, 0);

    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }
    if (GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES && checkInFilelog(filedir) == -1)
    {
        puts("ERROR: INVALID FILE OR DIRECTORY PATH:");
        puts(fileName);
    }
    else if ((GetFileAttributes(fileName) & FILE_ATTRIBUTE_DIRECTORY) && checkInFilelog(filedir) == -1)
    {
        dirChange(fileName, "*", 0);

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(fileName, &findFileData);
        FindNextFile(hFind, &findFileData);
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            char path[DIRNAME_LEN];
            strcpy(path, fileName);
            dirChange(path, findFileData.cFileName, 1);

            add(path, mode);
        }
        FindClose(hFind);
    }
    else
    {
        dirChange(dir, "stagedfiles.neogit", 0);

        if (mode == 'n')
        {
            printf(" %-30s : ", fileName);
            if (checkPathInFIle(filedir, "stagedfiles.neogit") == -1)
            {
                puts("not staged");
            }
            else
                puts("staged");
        }
        else if (checkPathInFIle(filedir, "stagedfiles.neogit") == -1)
        {
            FILE *stagedfiles = fopen(dir, "a");
            fwrite(filedir, 1, DIRNAME_LEN, stagedfiles);
            fclose(stagedfiles);

            int d = checkPathInFIle(filedir, "resetfiles.neogit");
            if (d != -1)
            {
                dirChange(dir, "resetfiles.neogit", 1);

                FILE *resetfiles = fopen(dir, "r+");
                fseek(resetfiles, d * DIRNAME_LEN, SEEK_SET);
                fwrite(EMPTY_STRING, 1, DIRNAME_LEN, resetfiles);
                fclose(resetfiles);
            }
        }
    }
}

int checkPathInFIle(char *filedir, char *file) // check existance of a path in a file in current branch
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);

    dirChange(dir, file, 0);

    FILE *stagedfiles = fopen(dir, "r");
    int d = 0;
    char stagedfiledir[DIRNAME_LEN];
    while (fread(stagedfiledir, 1, DIRNAME_LEN, stagedfiles))
    {
        if (strcmp(filedir, stagedfiledir) == 0)
            return d;
        d++;
        fseek(stagedfiles, d * DIRNAME_LEN, SEEK_SET);
    }
    return -1;
}

void reset(char *fileName, char nth)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }

    if (GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES)
    {
        puts("ERROR: INVALID FILE OR DIRECTORY PATH:");
        puts(fileName);
    }
    else if (GetFileAttributes(fileName) & FILE_ATTRIBUTE_DIRECTORY)
    {
        strcat(fileName, "\\*");
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(fileName, &findFileData);
        FindNextFile(hFind, &findFileData);
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            if (hFind == NULL)
            {
                puts("ERROR: INVALID FILE OR DIRECTORY PATH:");
                puts(fileName);
                continue;
            }
            char path[DIRNAME_LEN];
            strcpy(path, fileName);
            dirChange(path, findFileData.cFileName, 1);
            reset(path, 'a');
        }
        FindClose(hFind);
    }
    else
    {
        char filedir[DIRNAME_LEN];
        GetCurrentDirectory(DIRNAME_LEN, filedir);
        strcat(filedir, "\\");
        strcat(filedir, fileName);
        int d = checkPathInFIle(filedir, "stagedfiles.neogit");

        if (d == -1)
        {
            puts("ERROR: NOT AN STAGED FILE");
            return;
        }

        dirChange(dir, "stagedfiles.neogit", 0);

        FILE *stagedfiles = fopen(dir, "r+");
        fseek(stagedfiles, d * DIRNAME_LEN, SEEK_SET);
        fwrite(EMPTY_STRING, 1, DIRNAME_LEN, stagedfiles);
        fclose(stagedfiles);

        dirChange(dir, "resetfiles.neogit", 0);
        FILE *resetfiles = fopen(dir, "a");
        fwrite(filedir, 1, DIRNAME_LEN, resetfiles);
        fclose(resetfiles);
    }
}

void fileSep()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
        return;
    strcat(dir, "\\stagedfiles.neogit");
    FILE *resetfiles = fopen(dir, "a");
    fwrite(EMPTY_STRING, 1, DIRNAME_LEN, resetfiles);
    fclose(resetfiles);
}

void redo()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }
    char resetdir[DIRNAME_LEN];
    strcpy(resetdir, dir);

    dirChange(dir, "stagedfiles.neogit", 0);
    dirChange(resetdir, "resetfiles.neogit", 0);

    FILE *stagedfiles = fopen(dir, "a");
    FILE *resetfiles = fopen(resetdir, "r+");

    char path[DIRNAME_LEN];
    int d = 0;
    while (fread(path, 1, DIRNAME_LEN, resetfiles))
    {
        if (strcmp(path, EMPTY_STRING) != 0)
        {
            fseek(resetfiles, d * DIRNAME_LEN, SEEK_SET);
            fwrite(EMPTY_STRING, 1, DIRNAME_LEN, resetfiles);
            fwrite(path, 1, DIRNAME_LEN, stagedfiles);
        }
        d++;
        fseek(resetfiles, d * DIRNAME_LEN, SEEK_SET);
    }
}

void undo()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }
    char resetdir[DIRNAME_LEN];
    strcpy(resetdir, dir);

    dirChange(dir, "stagedfiles.neogit", 0);
    dirChange(resetdir, "resetfiles.neogit", 0);

    FILE *stagedfiles = fopen(dir, "r+");
    FILE *resetfiles = fopen(resetdir, "a");
    fseek(stagedfiles, 0, SEEK_END);

    char path[DIRNAME_LEN];
    int d = ftell(stagedfiles);

    while (1)
    {
        if (d == 0)
            return;
        fseek(stagedfiles, -DIRNAME_LEN, SEEK_CUR);
        fread(path, 1, DIRNAME_LEN, stagedfiles);
        if (strcmp(path, EMPTY_STRING) != 0)
            break;
        fseek(stagedfiles, -DIRNAME_LEN, SEEK_CUR);
        d -= DIRNAME_LEN;
    }

    while (1)
    {
        if (strcmp(path, EMPTY_STRING) == 0)
            break;
        fwrite(path, 1, DIRNAME_LEN, resetfiles);
        fseek(stagedfiles, -DIRNAME_LEN, SEEK_CUR);
        fwrite(EMPTY_STRING, 1, DIRNAME_LEN, stagedfiles);
        fseek(stagedfiles, -DIRNAME_LEN, SEEK_CUR);
        d -= DIRNAME_LEN;
        if (d == 0)
            return;
        fseek(stagedfiles, -DIRNAME_LEN, SEEK_CUR);
        fread(path, 1, DIRNAME_LEN, stagedfiles);
    }
    fclose(stagedfiles);
    fclose(resetfiles);
    return;
}
