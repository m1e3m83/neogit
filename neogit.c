#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define GLOBAL 'g'
#define LOCAL 'l'
#define EMAIL 1
#define USER 0
#define DATASTR_LEN 50
#define DIRNAME_LEN 100

#define INVCMD puts("Invalid cmd :/")

void config(char, int, char *);
void neogitReplocation(char *);
void init();

int main(int argc, char *argv[])
{
    // build a func to check for diff in staged files
    if (strcmp(argv[1], "config") == 0 && (argc == 4 || argc == 5))
    {
        char mode = LOCAL;
        if (strcmp(argv[2], "-global") == 0)
        {
            mode = GLOBAL;
            if (strcmp(argv[3], "user.name") == 0)
                config(GLOBAL, USER, argv[4]);
            else if (strcmp(argv[3], "user.email") == 0)
                config(GLOBAL, EMAIL, argv[4]);
            else
                INVCMD;
        }
        else if (strcmp(argv[2], "user.name") == 0)
            config(LOCAL, USER, argv[3]);
        else if (strcmp(argv[2], "user.email") == 0)
            config(LOCAL, EMAIL, argv[3]);
        else
            INVCMD;
    }
    else if (strcmp(argv[1], "init") == 0 && argc == 2)
    {
        init();
    }
    else
        INVCMD;

    return 0;
}

void config(char mode, int type, char *data)
{
    if (mode == GLOBAL)
    {
        // updates configFile.txt in main dir
        char exefileDir[DIRNAME_LEN];

        GetModuleFileName(NULL, exefileDir, DIRNAME_LEN);
        char *lastbs = strrchr(exefileDir, '\\');
        if (lastbs != NULL)
            *lastbs = '\0';
        strcat(exefileDir, "\\configfile.neogit");

        FILE *configFile = fopen(exefileDir, "r+");
        fseek(configFile, type * DATASTR_LEN, SEEK_SET);
        fwrite(data, sizeof(char), DATASTR_LEN, configFile);
        fclose(configFile);

        char repLoc[DIRNAME_LEN];
        neogitReplocation(repLoc);
        if (*repLoc != '\0')
        {
            strcat(repLoc, "\\localconfigs.neogit");
            FILE *localconfigs = fopen(repLoc, "r+");
            if (localconfigs == NULL)
            {
                fprintf(stderr, "ERORR: LOCALCONFIGS NOT FOUND.");
                return;
            }
            fprintf(localconfigs, "G");
            fclose(localconfigs);
        }
        return;
    }
    if (mode == LOCAL)
    {
        char reploc[DIRNAME_LEN];
        neogitReplocation(reploc);
        if (*reploc == '\0')
        {
            puts("ERROR: NOT IN A REPO FOLDER OR SUBFOLDER!");
            return;
        }
        strcat(reploc, "\\localconfigs.neogit");
        FILE *localconfigs = fopen(reploc, "r+");
        fprintf(localconfigs, "L");
        fseek(localconfigs, type * DATASTR_LEN, SEEK_CUR);
        fwrite(data, sizeof(char), DATASTR_LEN, localconfigs);
        fclose(localconfigs);

        return;
    }
}

void neogitReplocation(char *curLoc)
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

void init()
{
    char reploc[DIRNAME_LEN];
    neogitReplocation(reploc);
    if (*reploc != '\0')
    {
        puts("ERROR: A NEOGIT REPO ALREADY EXISTS IN THIS PAHT.");
        return;
    }

    CreateDirectory(".neogit", NULL);
    SetFileAttributes(".neogit", FILE_ATTRIBUTE_HIDDEN);

    FILE *localconfigs = fopen(".neogit\\localconfigs.neogit", "w");
    fprintf(localconfigs, "G");
    fclose(localconfigs);
    puts("Initializing neogit repo in this directory.");

    FILE *status = fopen(".neogit\\status.neogit", "w");
    char branch[DATASTR_LEN] = "\\main";
    fwrite(branch, 1, DATASTR_LEN, status);
    fclose(status);

    CreateDirectory(".neogit\\main", NULL);
    CreateDirectory(".neogit\\main\\staged", NULL);
    FILE *staged = fopen(".neogit\\main\\staged\\stagedfiles.neogit", "w");
    int a = 0;
    fwrite(&a, sizeof(int), 1, staged);
    fclose(staged);
}

void add(char *fileName)
{
    char neogitLoc[DIRNAME_LEN];
    neogitReplocation(neogitLoc);
    if (*neogitLoc == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }

    if (GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES)
    {
        puts("ERROR: INVALID FILE OR DIRECTORY PATH:");
        puts(fileName);
    }
    else if (GetFileAttributes(fileName) == FILE_ATTRIBUTE_DIRECTORY)
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
            strcat(path, "\\");
            strcat(path, findFileData.cFileName);
            add(path);
        }
        FindClose(hFind);
    }
    else
    {
        // reading current branch
        strcat(neogitLoc, "\\status");
        FILE *status = fopen(neogitLoc, "r");
        char branch[DATASTR_LEN];
        fread(branch, 1, DATASTR_LEN, status);
        fclose(status);
        // reseting neogitloc to original loc
        char *lastbs = strrchr(neogitLoc, '\\');
        *lastbs = '\0';
        // moving to the branch staged folder
        strcat(neogitLoc, branch);
        strcat(neogitLoc, "\\staged\\stagedfiles.neogit");

        // reading staged files num
        FILE *staged = fopen(neogitLoc, "r+");
        int stagedCount;
        fread(stagedCount, sizeof(int), 1, staged);
        stagedCount++;
        rewind(staged);
        fwrite(&stagedCount, sizeof(int), 1, staged);
        fclose(staged);
        char stagedCountStr[DATASTR_LEN];
        itoa(stagedCount, stagedCountStr, 10);
        // going back to staged dir
        lastbs = strrchr(neogitLoc, '\\');
        *lastbs = '\0';

        strcat(neogitLoc, "\\");
        strcat(neogitLoc, stagedCountStr);
        CreateDirectory(neogitLoc, NULL);
        strcat(neogitLoc, "\\filedata.neogit");
        FILE *filedata = fopen(neogitLoc, "w");
        fwrite(fileName, 1, DIRNAME_LEN, filedata);
        fclose(filedata);

        lastbs = strrchr(neogitLoc, '\\');
        *lastbs = '\0';

        // adding the full address to the file name
        char curdir[DIRNAME_LEN];
        GetCurrentDirectory(DIRNAME_LEN, curdir);
        strcat(curdir, "\\");
        strcat(curdir, fileName);
        strcpy(fileName, curdir);
    }
}