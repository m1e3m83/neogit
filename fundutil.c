#include "neogit.h"

void dirChange(char *dir, char *dirApp, int bsnum)
{
    if (bsnum == 0)
    {
        strcat(dir, "\\");
        strcat(dir, dirApp);
        return;
    }

    char *lastbs;
    for (int i = 0; i < bsnum; i++)
    {
        lastbs = strrchr(dir, '\\');
        *lastbs = '\0';
    }
    strcat(dir, "\\");
    strcat(dir, dirApp);
}

void findNeogitRep(char *curLoc)
{
    GetCurrentDirectory(DIRNAME_LEN, curLoc);
    while (1)
    {
        char *lastbs = strrchr(curLoc, '\\');
        strcat(curLoc, "\\.neogit");
        if (GetFileAttributes(curLoc) != INVALID_FILE_ATTRIBUTES && (GetFileAttributes(curLoc) & FILE_ATTRIBUTE_DIRECTORY) && (GetFileAttributes(curLoc) & FILE_ATTRIBUTE_HIDDEN))
            return;
        if (lastbs == NULL)
            break;
        *lastbs = '\0';
    }
    *curLoc = '\0';
    return;
}

int copydir(char *src, char *dest, char mode)
{
    if (GetFileAttributes(src) == INVALID_FILE_ATTRIBUTES)
    {
        fputs("ERROR: INVALILD FILENAME OR PATH", stderr);
        return 0;
    }
    else if (GetFileAttributes(src) & FILE_ATTRIBUTE_DIRECTORY)
    {
        // finding the name of the folder and creating it
        char *lastbs = strrchr(src, '\\');
        char foldername[DIRNAME_LEN];
        strcpy(foldername, dest);
        strcat(foldername, lastbs);
        if (GetFileAttributes(foldername) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(foldername) & FILE_ATTRIBUTE_DIRECTORY))
            CreateDirectory(foldername, NULL);
        // coping src folder path and searching its contents
        char subfilepath[DIRNAME_LEN];
        strcpy(subfilepath, src);

        dirChange(subfilepath, "*", 0);
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(subfilepath, &findFileData);
        FindNextFile(hFind, &findFileData);

        int fileCopied = 0;
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            strcpy(subfilepath, src);
            dirChange(subfilepath, findFileData.cFileName, 0);
            fileCopied += copydir(subfilepath, foldername, mode);
        }
        CloseHandle(hFind);
        return fileCopied;
    }
    else
    {
        if (checkPathInFIle(src, "stagedfiles.neogit") == -1 && mode == COPY_STAGED)
        {
            return 0;
        }
        char *lastbs = strrchr(src, '\\');
        char destfilepath[DATASTR_LEN];
        strcpy(destfilepath, dest);
        strcat(destfilepath, lastbs);

        CopyFile(src, destfilepath, 0);
        return 1;
    }
}

void wildcard(char *fileName, void(addset)(char *, char), char addmode)
{
    strtok(fileName, "$");

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileName, &findFileData);

    if (hFind == NULL)
        puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
    else
    {
        do
        {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
            {
                if (strrchr(fileName, '\\') != NULL)
                {
                    dirChange(fileName, findFileData.cFileName, 1);
                    addset(fileName, addmode);
                }
                else
                    addset(findFileData.cFileName, addmode);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
    }
}

int isNum(char *str)
{
    while (*str != '\0')
    {
        if (*str > '9' || *str < '0')
            return 0;
        str++;
    }
    return 1;
}
