#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define GLOBAL 'g'
#define LOCAL 'l'
#define EMAIL 1
#define USER 0
#define COPY_STAGED 's'
#define COPY_ALL '\0'
#define DATASTR_LEN 64
#define DIRNAME_LEN 128
#define EMPTY_STRING "THIS IS AN EMPTY STRING DESIGNED TO REWRITE THE OTHER STRING THAT LAYED IN HERE. RIP DEAR OLD STRING, ALL HAIL THE NEW STRING! "
#define MAX_COMMIT_NUM 1000

#define INVCMD puts("Invalid cmd :/")

void dirChange(char *, char *, int);
void findNeogitRep(char *);
int copydir(char *, char *, char);

void config(char, int, char *);
void init();
void add(char *, char);
int checkPathInFIle(char *, char *);
void redo();
void reset(char *, char);
void undo();
void fileSep();

void loadCommits();
void findHead();
void commit(char *);
void snapshot();
void filelog();
int checkInFilelog(char *);
// stuct
struct commit
{
    int id;
    char msg[73];
    char branch[DATASTR_LEN];
    char authName[DATASTR_LEN];
    char authEmail[DATASTR_LEN];
    struct commit *pervCommit;
    time_t t;
};
typedef struct commit Commit;

Commit commits[MAX_COMMIT_NUM];
int commitCount;
Commit *head = NULL;

struct fileinfo
{
    char path[DIRNAME_LEN];
    time_t lastCommit;
};
typedef struct fileinfo Fileinfo;

int main(int argc, char *argv[])
{
    loadCommits();
    findHead();

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
        if (strcmp(argv[2], "-redo") == 0 && argc == 3)
        {
            redo();
        }
        else if (strcmp(argv[2], "-f") == 0)
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
                        if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
                            add(findFileData.cFileName, '\0');
                    } while (FindNextFile(hFind, &findFileData) != 0);
                }
            }
            fileSep();
        }
        else if (strcmp(argv[2], "-n") == 0)
        {
            strtok(argv[3], "$");

            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(argv[3], &findFileData);
            if (hFind == NULL)
                puts("ERROR: INVALID FILE NAME OR DIRECTORY!");
            else
            {
                do
                {
                    if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
                        add(findFileData.cFileName, 'n');
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
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
                    if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
                        add(findFileData.cFileName, '\0');
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
            fileSep();
        }
    }
    else if (strcmp(argv[1], "reset") == 0 && argc > 2)
    {
        if (strcmp(argv[2], "-undo") == 0)
        {
            undo();
        }
        else if (strcmp(argv[2], "-f") == 0 && argc > 3)
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
                        reset(findFileData.cFileName, 'a');
                    } while (FindNextFile(hFind, &findFileData) != 0);
                }
            }
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
                    if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
                        reset(findFileData.cFileName, 'a');
                } while (FindNextFile(hFind, &findFileData) != 0);
            }
        }
    }
    else if (strcmp(argv[1], "commit") == 0 && strcmp(argv[2], "-m") == 0 && argc == 4)
    {
        commit(argv[3]);
        snapshot();
        filelog();
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
        if (configFile == NULL)
        {
            puts("ERROR: COULD NOT FIND CONFIGFILE");
        }
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
            fseek(localconfigs, type * DATASTR_LEN, SEEK_SET);
            fwrite(data, sizeof(char), DATASTR_LEN, localconfigs);
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

    char exefileDir[DIRNAME_LEN];
    GetModuleFileName(NULL, exefileDir, DIRNAME_LEN);
    dirChange(exefileDir, "configfile.neogit", 1);
    FILE *configFile = fopen(exefileDir, "r");
    char data[DATASTR_LEN];
    fread(data, sizeof(char), DATASTR_LEN, configFile);

    FILE *localconfigs = fopen(".neogit\\localconfigs.neogit", "w");
    fwrite(data, 1, DATASTR_LEN, localconfigs);
    fread(data, sizeof(char), DATASTR_LEN, configFile);
    fwrite(data, 1, DATASTR_LEN, localconfigs);

    fclose(configFile);
    fclose(localconfigs);
    puts("Initializing neogit repo in this directory.");

    FILE *status = fopen(".neogit\\status.neogit", "w");
    char branch[DATASTR_LEN] = "main";
    fwrite(branch, 1, DATASTR_LEN, status);
    int head = 0;
    fwrite(&head, sizeof(int), 1, status);
    fclose(status);

    CreateDirectory(".neogit\\main", NULL);
    CreateDirectory(".neogit\\commits", NULL);
}

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
    else if (GetFileAttributes(fileName) & FILE_ATTRIBUTE_DIRECTORY)
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
            puts("ERROR: NOT AN STAGED FILE!");
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
    strcat(dir, "\\stagedfiles.neogit");
    FILE *resetfiles = fopen(dir, "a");
    fwrite(EMPTY_STRING, 1, DIRNAME_LEN, resetfiles);
    fclose(resetfiles);
}

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

void redo()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);

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

void loadCommits()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "commitslog.neogit", 0);

    int count = 0;

    FILE *commitslog = fopen(dir, "r");

    if (commitslog != NULL)
        while (fread(commits + count, sizeof(Commit), 1, commitslog))
            count++;

    commitCount = count;
    fclose(commitslog);
}

void findHead()
{
    if (commitCount == 0)
    {
        head = NULL;
        return;
    }

    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "status.neogit", 0);
    int headid;
    FILE *status = fopen(dir, "r");
    if (status == NULL)
    {
        puts("ERROR: UNABLE TO FIND STATUS FILE!");
        return;
    }
    fseek(status, DATASTR_LEN, SEEK_SET);
    fread(&headid, sizeof(int), 1, status);
    fclose(status);
    for (int i = 0; i < commitCount; i++)
    {
        if (headid == commits[i].id)
        {
            head = &commits[i];
            return;
        }
    }
}

void commit(char *msg)
{
    if (strlen(msg) > 72)
    {
        puts("ERROR: COMMIT MSG EXCEEDS THE LIMIT(72)!");
        return;
    }

    Commit *curCommit = &commits[commitCount];
    curCommit->id = 10000 + commitCount;

    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "localconfigs.neogit", 0);
    FILE *localconfigs = fopen(dir, "r");
    fread(&curCommit->authName, 1, DATASTR_LEN, localconfigs);
    fread(&curCommit->authEmail, 1, DATASTR_LEN, localconfigs);
    fclose(localconfigs);

    strcpy(curCommit->msg, msg);

    dirChange(dir, "status.neogit", 1);
    FILE *status = fopen(dir, "r+");
    fread(&curCommit->branch, 1, DATASTR_LEN, status);
    fseek(status, DATASTR_LEN, SEEK_SET);
    fwrite(&(curCommit->id), sizeof(int), 1, status);
    fclose(status);

    time(&curCommit->t);

    curCommit->pervCommit = head;

    dirChange(dir, "commitslog.neogit", 1);

    FILE *commitslog = fopen(dir, "a");

    fwrite(curCommit, sizeof(Commit), 1, commitslog);
    head = curCommit;
    fclose(commitslog);
}

void snapshot()
{
    // setting dir to repo\.neogit\commits\#headcommit folder
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "commits", 0);
    char headidstr[DATASTR_LEN];
    itoa(head->id, headidstr, 10);
    dirChange(dir, headidstr, 0);
    CreateDirectory(dir, NULL);

    // setting reploc to main repo\.neogit folder
    char reploc[DIRNAME_LEN];
    findNeogitRep(reploc);

    if (head->pervCommit != NULL)
    {
        // going to prevcom folder
        char prevCom[DIRNAME_LEN];
        strcpy(prevCom, dir);
        char prevCommitid[DATASTR_LEN];
        itoa(head->pervCommit->id, prevCommitid, 10);
        dirChange(prevCom, prevCommitid, 1);

        char filepath[DIRNAME_LEN];
        strcpy(filepath, prevCom);
        dirChange(filepath, "*", 0);
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(filepath, &findFileData);
        FindNextFile(hFind, &findFileData);
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            strcpy(filepath, prevCom);
            dirChange(filepath, findFileData.cFileName, 0);
            copydir(filepath, dir, COPY_ALL);
        }
        CloseHandle(hFind);
    }

    char filepath[DIRNAME_LEN];
    strcpy(filepath, reploc);
    dirChange(filepath, "*", 1);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(filepath, &findFileData);
    FindNextFile(hFind, &findFileData);
    while (FindNextFile(hFind, &findFileData) != 0)
    {
        if (strcmp(findFileData.cFileName, ".neogit") == 0)
            continue;
        strcpy(filepath, reploc);
        dirChange(filepath, findFileData.cFileName, 1);
        copydir(filepath, dir, COPY_STAGED);
    }
    CloseHandle(hFind);
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

void filelog()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);

    dirChange(dir, "filelog.neogit", 0);
    FILE *filelog = fopen(dir, "r+");
    if (filelog == NULL)
        filelog = fopen(dir, "w");

    dirChange(dir, "stagedfiles.neogit", 1);
    FILE *stagedfiles = fopen(dir, "r+");
    if (stagedfiles == NULL)
        return;

    int sfplace = 0;
    char stagedfiledir[DIRNAME_LEN];
    while (fread(stagedfiledir, 1, DIRNAME_LEN, stagedfiles))
    {
        if (strcmp(stagedfiledir, EMPTY_STRING) == 0)
            continue;

        int flplace = checkInFilelog(stagedfiledir);

        Fileinfo fileinfo;
        strcpy(fileinfo.path, stagedfiledir);
        time(&fileinfo.lastCommit);

        if (flplace == -1)
        {
            int emptyPlace = checkInFilelog(EMPTY_STRING);
            if (emptyPlace == -1)
                fseek(filelog, 0, SEEK_END);
            else
                fseek(filelog, emptyPlace * sizeof(Fileinfo), SEEK_SET);
            fwrite(&fileinfo, sizeof(Fileinfo), 1, filelog);
        }
        else
        {
            fseek(filelog, flplace * sizeof(Fileinfo), SEEK_SET);
            if (GetFileAttributes(stagedfiledir) == INVALID_FILE_ATTRIBUTES)
            {
                strcpy(fileinfo.path, EMPTY_STRING);
                // delete it from the cur commit
            }
            fwrite(&fileinfo, sizeof(fileinfo), 1, filelog);
        }

        sfplace++;
        fseek(stagedfiles, sfplace * DIRNAME_LEN, SEEK_SET);
        fseek(filelog, 0, SEEK_SET);
    }
    fclose(filelog);
    fclose(stagedfiles);
    stagedfiles = fopen(dir, "w");
    fclose(stagedfiles);
}

int checkInFilelog(char *filepath)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);

    dirChange(dir, "filelog.neogit", 0);

    FILE *filelog = fopen(dir, "r");
    int placement = 0;
    Fileinfo fileinfo;
    while (fread(&fileinfo, sizeof(Fileinfo), 1, filelog))
    {
        if (strcmp(fileinfo.path, filepath) == 0)
            return placement;
        placement++;
        fseek(filelog, placement * sizeof(fileinfo), SEEK_SET);
    }
    return -1;
}