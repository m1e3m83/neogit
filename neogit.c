#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define GLOBAL 'g'
#define LOCAL 'l'
#define EMAIL 1
#define USER 0
#define DATASTR_LEN 50
#define DIRNAME_LEN 128
#define EMPTY_STRING "THIS IS AN EMPTY STRING DESIGNED TO AND REWRITE THE ORIGINAL STRING THAT LAY IN HERE. RIP OLD STRING, ALL HAIL THE NEW STRING!"

#define INVCMD puts("Invalid cmd :/")

void dirChange(char *, char *, int);
void config(char, int, char *);
void findNeogitRep(char *);
void init();
void add(char *, char);
int checkPathInFIle(char *, char *);
void reset(char *);
void resetfs();

int main(int argc, char *argv[])
{
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
    else if (strcmp(argv[1], "add") == 0 && argc > 2)
    {
        if (strcmp(argv[2], "-f") == 0)
        {
            for (int i = 3; i < argc; i++)
            {
                strtok(argv[i], "\\");

                WIN32_FIND_DATA findFileData;
                HANDLE hFind = FindFirstFile(argv[i], &findFileData);

                if (hFind == NULL)
                    puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
                else
                {
                    do
                    {
                        add(findFileData.cFileName, '\0');
                    } while (FindNextFile(hFind, &findFileData) != 0);
                }
            }
        }
        else if (strcmp(argv[2], "-n") == 0)
        {
            strtok(argv[3], "\\");

            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(argv[3], &findFileData);
            if (hFind == NULL)
                puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
            else
            {
                do
                {
                    add(findFileData.cFileName, 'n');
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
        }
        else
        {
            strtok(argv[2], "\\");

            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(argv[2], &findFileData);

            if (hFind == NULL)
                puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
            else
            {
                do
                {
                    add(findFileData.cFileName, '\0');
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
        }
    }
    else if (strcmp(argv[1], "reset") == 0 && argc > 2)
    {
        if (strcmp(argv[2], "-f") == 0 && argc > 3)
        {
            for (int i = 3; i < argc; i++)
            {
                strtok(argv[i], "$");

                WIN32_FIND_DATA findFileData;
                HANDLE hFind = FindFirstFile(argv[i], &findFileData);

                if (hFind == NULL)
                    puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
                else
                {
                    do
                    {
                        reset(findFileData.cFileName);
                    } while (FindNextFile(hFind, &findFileData) != 0);
                }
            }
            resetfs();
        }
        else
        {
            strtok(argv[2], "$");

            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(argv[2], &findFileData);

            if (hFind == NULL)
                puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
            else
            {
                do
                {
                    reset(findFileData.cFileName);
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
            resetfs();
        }
    }
    else
        INVCMD;

    return 0;
}

void config(char mode, int type, char *data)
{
    if (mode == GLOBAL)
    {
        // updates configFile.neogit in main dir
        char exefileDir[DIRNAME_LEN];

        GetModuleFileName(NULL, exefileDir, DIRNAME_LEN);
        dirChange(exefileDir, "configfile.neogit", 1);

        FILE *configFile = fopen(exefileDir, "r+");
        fseek(configFile, type * DATASTR_LEN, SEEK_SET);
        fwrite(data, sizeof(char), DATASTR_LEN, configFile);
        fclose(configFile);

        char reploc[DIRNAME_LEN];
        findNeogitRep(reploc);
        if (*reploc != '\0')
        {
            dirChange(reploc, "localconfigs.neogit", 0);
            FILE *localconfigs = fopen(reploc, "r+");
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
        findNeogitRep(reploc);

        if (*reploc == '\0')
        {
            puts("ERROR: NOT IN A REPO FOLDER OR SUBFOLDER!");
            return;
        }

        dirChange(reploc, "localconfigs.neogit", 0);

        FILE *localconfigs = fopen(reploc, "r+");
        fprintf(localconfigs, "L");
        fseek(localconfigs, type * DATASTR_LEN, SEEK_CUR);
        fwrite(data, sizeof(char), DATASTR_LEN, localconfigs);
        fclose(localconfigs);

        return;
    }
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

void init()
{
    char reploc[DIRNAME_LEN];
    findNeogitRep(reploc);
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
    char branch[DATASTR_LEN] = "main";
    fwrite(branch, 1, DATASTR_LEN, status);
    fclose(status);

    CreateDirectory(".neogit\\main", NULL);
    CreateDirectory(".neogit\\main\\staged", NULL);
    FILE *staged = fopen(".\\main\\staged\\stagedfiles.neogit", "w");
    fclose(staged);
}

void add(char *fileName, char mode)
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
        dirChange(fileName, "*", 0);

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

            add(path, mode);
        }
        FindClose(hFind);
    }
    else
    {
        char filedir[DIRNAME_LEN];
        GetCurrentDirectory(DIRNAME_LEN, filedir);
        dirChange(filedir, fileName, 0);

        dirChange(dir, "status.neogit", 0);

        FILE *status = fopen(dir, "r");
        char branch[DATASTR_LEN];
        fread(branch, 1, DATASTR_LEN, status);
        fclose(status);

        dirChange(dir, branch, 1);
        dirChange(dir, "staged\\stagedfiles.neogit", 0);

        if (mode == 'n')
        {
            printf(" %s : ", fileName);
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
            fseek(stagedfiles, 0, SEEK_END);
            fwrite(&filedir, 1, DIRNAME_LEN, stagedfiles);
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
    dirChange(dir, "status.neogit", 0);

    FILE *status = fopen(dir, "r");
    char branch[DATASTR_LEN];
    fread(branch, 1, DATASTR_LEN, status);
    fclose(status);

    dirChange(dir, branch, 1);
    dirChange(dir, "staged", 0);
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

void reset(char *fileName)
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
            char *lastbs = strrchr(path, '\\');
            lastbs[1] = '\0';
            strcat(path, findFileData.cFileName);
            reset(path);
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
            puts("ERROR: NOT AN STAGED FILE!");
            return;
        }

        strcat(dir, "\\status.neogit");
        FILE *status = fopen(dir, "r");
        char branch[DATASTR_LEN];
        fread(branch, 1, DATASTR_LEN, status);
        fclose(status);

        char *lastbs = strrchr(dir, '\\');
        strcpy(lastbs + 1, branch);
        strcat(dir, "\\staged\\stagedfiles.neogit");

        FILE *stagedfiles = fopen(dir, "r+");
        fseek(stagedfiles, d * DIRNAME_LEN, SEEK_SET);
        fwrite(EMPTY_STRING, 1, DIRNAME_LEN, stagedfiles);
        fclose(stagedfiles);

        lastbs = strrchr(dir, '\\');
        strcpy(lastbs + 1, "resetfiles.neogit");
        FILE *resetfiles = fopen(dir, "a");
        fwrite(filedir, 1, DIRNAME_LEN, resetfiles);
        fclose(resetfiles);
    }
}

void resetfs()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    strcat(dir, "\\status.neogit");
    FILE *status = fopen(dir, "r");
    char branch[DATASTR_LEN];
    fread(branch, 1, DATASTR_LEN, status);
    fclose(status);
    char *lastbs = strrchr(dir, '\\');
    strcpy(lastbs + 1, branch);
    strcat(dir, "\\staged\\resetfiles.neogit");
    FILE *resetfiles = fopen(dir, "a");
    fwrite(EMPTY_STRING, 1, DIRNAME_LEN, resetfiles);
    fclose(resetfiles);
}

void dirChange(char *dir, char *dirApp, int bsnum)
{
    if (bsnum == 0)
    {
        strcar(dir, "\\");
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
    strcpy(lastbs, dirApp);
}