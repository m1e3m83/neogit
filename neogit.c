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

#define REPLACE 1
#define READ 2
#define NO_REPLACE 2
#define MERGE 4

#define DATASTR_LEN 128
#define DIRNAME_LEN 128
#define EMPTY_STRING "THIS IS AN EMPTY STRING DESIGNED TO REWRITE THE OTHER STRING THAT LAYED IN HERE. RIP DEAR OLD STRING, ALL HAIL THE NEW STRING. "
#define MAX_COMMIT_NUM 1000

#define RED 4
#define GREEN 2
#define WHITE 7

#define INVCMD puts("Invalid cmd :/")

struct commit
{
    int id;
    char msg[73];
    char branch[DATASTR_LEN];
    char authName[DATASTR_LEN];
    char authEmail[DATASTR_LEN];
    struct commit *pervCommit;
    int prevcomid;
    struct commit *merge;
    time_t t;
    int num;
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

struct alias
{
    char aname[DATASTR_LEN];
    char cmd[DATASTR_LEN];
};
typedef struct alias Alias;

struct shortcut
{
    char sname[DATASTR_LEN];
    char msg[DATASTR_LEN];
};
typedef struct shortcut Shortcut;

struct tag
{
    char name[DATASTR_LEN];
    char msg[DATASTR_LEN];
    int comid;
    char authName[DATASTR_LEN];
    char authEmail[DATASTR_LEN];
    time_t createT;
};
typedef struct tag Tag;

void dirChange(char *, char *, int);
int findNeogitRep(char *);
int copydir(char *, char *, char);
void wildcard(char *, void(char *, char), char);
int isNum(char *);
void setTextColor(int);

void config(char, int, char *);
void writeAlias(char *, char *, char);
int exAlias(char *);

void init();

void add(char *, char);
int checkPathInFIle(char *, char *);
void redo();
void reset(char *, char);
void undo();
void fileSep();

void loadCommits();
void findHead();
int checkstaged();
void commit(char *, int, Commit *);
void snapshot();
int filelog();
int checkInFilelog(char *);
void cleanCommit(char *);
void set(const char *, const char *);
int findShortcut(const char *, char *, char);

void comlog(int(char *, Commit *), char *);
int logcg(char *, Commit *);
int logcn(char *, Commit *);
int logcb(char *, Commit *);
int logca(char *, Commit *);
int logcdBefore(char *, Commit *);
int logcdSince(char *, Commit *);
int logcw(char *, Commit *);

void status(char *);
void statusD();

void branch();
void createBranch(char *);

void checkoutid(char *);
void checkoutb(char *);

void tag(char *, char *, char *, char);
void showtag(char *);

int diff(char *, char *);
void printline(char *, int, int);
char *findprintable(char *);
int findtokenlen(char *, char *);
void commitdiff(char *, char *);
void findcomfiles(char *, char *, char *, char *, char);

void merge(char *, char *);
int copymerge(char *, char *, char *);

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
            else if (argv[3][0] == 'a' && argv[3][1] == 'l' && argv[3][2] == 'i' && argv[3][3] == 'a' && argv[3][4] == 's' && argv[3][5] == '.' && argc == 5)
                writeAlias(argv[3] + 6, argv[4], GLOBAL);
            else
                INVCMD;
        }
        else if (strcmp(argv[2], "user.name") == 0)
            config(LOCAL, USER, argv[3]);
        else if (strcmp(argv[2], "user.email") == 0)
            config(LOCAL, EMAIL, argv[3]);
        else if (argv[2][0] == 'a' && argv[2][1] == 'l' && argv[2][2] == 'i' && argv[2][3] == 'a' && argv[2][4] == 's' && argv[2][5] == '.' && argc == 4)
            writeAlias(argv[2] + 6, argv[3], LOCAL);
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
                if (strrchr(argv[i], '$') != NULL)
                    wildcard(argv[i], add, '\0');
                else
                    add(argv[i], '\0');
            }
            fileSep();
        }
        else if (strcmp(argv[2], "-n") == 0)
        {
            if (strrchr(argv[3], '$') != NULL)
                wildcard(argv[3], add, 'n');
            else
                add(argv[3], 'n');
        }
        else
        {
            if (strrchr(argv[2], '$') != NULL)
                wildcard(argv[2], add, '\0');
            else
                add(argv[2], '\0');
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
                if (strrchr(argv[i], '$') != NULL)
                    wildcard(argv[i], reset, '\0');
                else
                    add(argv[i], '\0');
            }
        }
        else
        {
            if (strrchr(argv[2], '$') != NULL)
                wildcard(argv[2], reset, '\0');
            else
                reset(argv[2], '\0');
        }
    }
    else if (strcmp(argv[1], "commit") == 0 && argc == 4)
    {
        if (checkstaged())
        {
            if (strcmp(argv[2], "-m") == 0)
            {
                int commitNum = filelog();
                commit(argv[3], commitNum, NULL);
                snapshot();
            }
            else if (strcmp(argv[2], "-s") == 0)
            {
                char msg[DIRNAME_LEN];
                int sfound = findShortcut(argv[3], msg, READ);
                if (sfound)
                {
                    int commitNum = filelog();
                    commit(msg, commitNum, NULL);
                    snapshot();
                }
            }
        }
        else
            puts("ERROR: NO FILE IS STAGED!");
    }
    else if (strcmp(argv[1], "set") == 0 && strcmp(argv[2], "-m") == 0 && strcmp(argv[4], "-s") == 0 && argc == 6)
    {
        set(argv[5], argv[3]);
    }
    else if (strcmp(argv[1], "replace") == 0 && strcmp(argv[2], "-m") == 0 && strcmp(argv[4], "-s") == 0 && argc == 6)
    {
        findShortcut(argv[5], argv[3], REPLACE);
    }
    else if (strcmp(argv[1], "remove") == 0 && strcmp(argv[2], "-s") == 0 && argc == 4)
    {
        findShortcut(argv[3], EMPTY_STRING, REPLACE);
    }
    else if (strcmp(argv[1], "log") == 0)
    {
        if (argc == 2)
            comlog(logcg, "");
        else if (strcmp(argv[2], "-n") == 0)
            comlog(logcn, argv[3]);
        else if (strcmp(argv[2], "-branch") == 0)
            comlog(logcb, argv[3]);
        else if (strcmp(argv[2], "-author") == 0)
            comlog(logca, argv[3]);
        else if (strcmp(argv[2], "-since") == 0)
            comlog(logcdSince, argv[3]);
        else if (strcmp(argv[2], "-before") == 0)
            comlog(logcdBefore, argv[3]);
        else if (strcmp(argv[2], "-search") == 0)
            comlog(logcw, argv[3]);
        else
            INVCMD;
    }
    else if (strcmp(argv[1], "status") == 0 && argc == 2)
    {
        char dir[DIRNAME_LEN];
        findNeogitRep(dir);
        *strrchr(dir, '\\') = '\0';
        status(dir);
        statusD();
    }
    else if (strcmp(argv[1], "branch") == 0)
    {
        if (argc == 2)
        {
            branch();
        }
        else if (argc == 3)
        {
            createBranch(argv[2]);
        }
    }
    else if (strcmp(argv[1], "checkout") == 0 && (argc == 3 || argc == 4))
    {
        if (isNum(argv[2]))
        {
            checkoutid(argv[2]);
        }
        else
        {
            if (strcmp(argv[2], "HEAD") == 0)
            {
                char id[DATASTR_LEN];
                itoa(head->id, id, 10);
                checkoutid(id);
            }
            else if (argv[2][0] == 'H' && argv[2][1] == 'E' && argv[2][2] == 'A' && argv[2][3] == 'D' && argv[2][4] == '-')
            {
                int n = 0;
                Commit *tcom = head;
                sscanf(argv[2] + 5, "%d", &n);
                for (int i = 0; i < n; i++)
                {
                    if (tcom->pervCommit == NULL)
                        break;
                    tcom = tcom->pervCommit;
                }
                char id[DATASTR_LEN];
                itoa(tcom->id, id, 10);
                checkoutid(id);
            }
            else
            {
                checkoutb(argv[2]);
            }
        }
    }
    else if (strcmp(argv[1], "tag") == 0 && argc > 1 && argc < 10)
    {
        if (argc == 2)
        {
            showtag(NULL);
        }
        else if (strcmp(argv[2], "show") == 0 && argc == 4)
        {
            showtag(argv[3]);
        }
        else if (strcmp(argv[2], "-a") == 0)
        {
            if (argc > 4 && strcmp(argv[4], "-m") == 0)
            {
                if (argc > 6 && strcmp(argv[6], "-c") == 0)
                {
                    if (argc > 8 && strcmp(argv[8], "-f") == 0)
                    {
                        tag(argv[3], argv[5], argv[7], REPLACE);
                    }
                    else if (argc == 8)
                    {
                        tag(argv[3], argv[5], argv[7], 0);
                    }
                    else
                        INVCMD;
                }
                else if (argc > 6 && strcmp(argv[6], "-f") == 0)
                {
                    tag(argv[3], argv[5], NULL, REPLACE);
                }
                else if (argc == 6)
                {
                    tag(argv[3], argv[5], NULL, 0);
                }
                else
                    INVCMD;
            }
            else if (argc > 4 && strcmp(argv[4], "-c") == 0)
            {

                if (argc > 6 && strcmp(argv[6], "-f") == 0)
                {
                    tag(argv[3], NULL, argv[5], REPLACE);
                }
                else if (argc == 6)
                {
                    tag(argv[3], NULL, argv[5], 0);
                }
                else
                    INVCMD;
            }
            else if (argc > 4 && strcmp(argv[4], "-f") == 0)
            {
                tag(argv[3], NULL, NULL, REPLACE);
            }
            else if (argc == 4)
            {
                tag(argv[3], NULL, NULL, 0);
            }
            else
                INVCMD;
        }
        else
            INVCMD;
    }
    else if (strcmp(argv[1], "diff") == 0 && argc > 2)
    {
        if (strcmp(argv[2], "-f") == 0 && argc == 5)
        {
            diff(argv[3], argv[4]);
        }
        else if (strcmp(argv[2], "-c") == 0 && argc == 5)
        {
            commitdiff(argv[3], argv[4]);
        }
        else
            INVCMD;
    }
    else if (strcmp(argv[1], "merge") == 0 && strcmp(argv[2], "-b") == 0 && argc == 5)
    {
        merge(argv[3], argv[4]);
    }
    else if (!exAlias(argv[1]))
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

int findNeogitRep(char *curLoc)
{
    GetCurrentDirectory(DIRNAME_LEN, curLoc);
    int bsnum = 0;
    while (1)
    {
        char *lastbs = strrchr(curLoc, '\\');
        strcat(curLoc, "\\.neogit");
        if (GetFileAttributes(curLoc) != INVALID_FILE_ATTRIBUTES && (GetFileAttributes(curLoc) & FILE_ATTRIBUTE_DIRECTORY) && (GetFileAttributes(curLoc) & FILE_ATTRIBUTE_HIDDEN))
            return bsnum;
        if (lastbs == NULL)
            break;
        *lastbs = '\0';
        bsnum++;
    }
    *curLoc = '\0';
    return -1;
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
    fprintf(status, "master");
    fclose(status);

    CreateDirectory(".neogit\\master", NULL);
    FILE *branchhead = fopen(".neogit\\master\\branchhead.neogit", "w");
    fprintf(branchhead, "%d", 0);

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

void loadCommits()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        commitCount = 0;
        return;
    }
    dirChange(dir, "commitslog.neogit", 0);

    int count = 0;

    FILE *commitslog = fopen(dir, "r");

    if (commitslog != NULL)
        while (fread(commits + count, sizeof(Commit), 1, commitslog))
            count++;

    for (int i = 0; i < commitCount; i++)
        if (commits[i].prevcomid != 0)
            commits[i].pervCommit = commits + (commits[i].prevcomid - 10000);
        else
            commits[i].pervCommit = NULL;

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
    if (*dir == '\0')
    {
        puts("ERORR: NOT IN A NEOGIT REPE!");
        return;
    }

    dirChange(dir, "status.neogit", 0);
    FILE *status = fopen(dir, "r");
    char branch[DATASTR_LEN];
    if (status == NULL)
    {
        puts("ERROR: UNABLE TO FIND STATUS FILE!");
        return;
    }
    fscanf(status, "%s", branch);
    fclose(status);

    dirChange(dir, branch, 1);
    dirChange(dir, "branchhead.neogit", 0);
    FILE *branchhead = fopen(dir, "r");
    int headid;
    fscanf(branchhead, "%d", &headid);

    for (int i = 0; i < commitCount; i++)
    {
        if (headid == commits[i].id)
        {
            head = &commits[i];
            return;
        }
    }
}

void commit(char *msg, int num, Commit *merge)
{
    if (strlen(msg) > 72)
    {
        puts("ERROR: COMMIT MSG EXCEEDS THE LIMIT(72)!");
        return;
    }

    Commit *curCommit = &commits[commitCount];
    curCommit->id = 10000 + commitCount;

    curCommit->num = num;

    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A GIT REPO FOLDER OF SUBFOLDER!");
        return;
    }
    dirChange(dir, "localconfigs.neogit", 0);
    FILE *localconfigs = fopen(dir, "r");
    fread(&curCommit->authName, 1, DATASTR_LEN, localconfigs);
    fread(&curCommit->authEmail, 1, DATASTR_LEN, localconfigs);
    fclose(localconfigs);

    strcpy(curCommit->msg, msg);

    dirChange(dir, "status.neogit", 1);
    FILE *status = fopen(dir, "r+");
    fread(&curCommit->branch, 1, DATASTR_LEN, status);
    fclose(status);

    time(&curCommit->t);

    curCommit->pervCommit = head;
    if (head != NULL)
        curCommit->prevcomid = head->id;
    else
        curCommit->prevcomid = 0;
    curCommit->merge = merge;

    dirChange(dir, "commitslog.neogit", 1);

    FILE *commitslog = fopen(dir, "a");

    fwrite(curCommit, sizeof(Commit), 1, commitslog);
    head = curCommit;
    fclose(commitslog);

    dirChange(dir, curCommit->branch, 1);
    dirChange(dir, "branchhead.neogit", 0);
    FILE *branchhead = fopen(dir, "w");
    fprintf(branchhead, "%d", curCommit->id);
    fclose(branchhead);
}

int checkstaged()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return 0;
    }

    dirChange(dir, "stagedfiles.neogit", 0);
    FILE *stagedfiles = fopen(dir, "r");
    int place = 0;
    char stagedfiledir[DIRNAME_LEN];
    while (fread(stagedfiledir, 1, DIRNAME_LEN, stagedfiles))
    {
        if (strcmp(stagedfiledir, EMPTY_STRING) != 0)
            return 1;
        place++;
        fseek(stagedfiles, place * DIRNAME_LEN, SEEK_SET);
    }
    return 0;
}

void snapshot()
{
    // setting dir to repo\.neogit\commits\#headcommit folder
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
        return;
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

    dirChange(dir, "stagedfiles.neogit", 2);
    FILE *stagedfiles = fopen(dir, "w");
    fclose(stagedfiles);
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

int filelog()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
        return 0;
    dirChange(dir, "filelog.neogit", 0);
    FILE *filelog = fopen(dir, "r+");
    if (filelog == NULL)
        filelog = fopen(dir, "w");

    dirChange(dir, "stagedfiles.neogit", 1);
    FILE *stagedfiles = fopen(dir, "r+");
    if (stagedfiles == NULL)
        return 0;

    int sfplace = 0;
    char stagedfiledir[DIRNAME_LEN];

    int stagedFilesNum = 0;
    while (fread(stagedfiledir, 1, DIRNAME_LEN, stagedfiles))
    {
        if (strcmp(stagedfiledir, EMPTY_STRING) != 0)
        {
            stagedFilesNum++;
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
                    cleanCommit(stagedfiledir);
                }
                fwrite(&fileinfo, sizeof(Fileinfo), 1, filelog);
            }
        }
        sfplace++;
        fseek(stagedfiles, sfplace * DIRNAME_LEN, SEEK_SET);
        fseek(filelog, 0, SEEK_SET);
    }
    fclose(filelog);
    fclose(stagedfiles);

    dirChange(dir, "resetfiles.neogit", 1);
    stagedfiles = fopen(dir, "w");
    fclose(stagedfiles);

    return stagedFilesNum;
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

void cleanCommit(char *fileprevpath)
{
    // this funct deletes the files in old commits that now have been removed
    char reploc[DIRNAME_LEN];
    findNeogitRep(reploc);
    if (*reploc == '\0')
        return;
    char *add = strrchr(reploc, '\\');
    *add = '\0';
    add = strrchr(reploc, '\\');
    char *mark = strstr(fileprevpath, add);
    mark += strlen(add);

    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "commits", 0);
    char headidstr[DATASTR_LEN];
    itoa(head->id, headidstr, 10);
    dirChange(dir, headidstr, 0);
    strcat(dir, mark);

    int a = DeleteFile(dir);
    if (a == 0)
        puts("ERROR");
}

void comlog(int(mode)(char *, Commit *), char *inp)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
        return;
    dirChange(dir, "commitslog.neogit", 0);

    for (int i = commitCount - 1; i >= 0; i--)
    {
        if (mode(inp, commits + i))
        {
            struct tm *timeinfo = localtime(&commits[i].t);
            printf("\t Commit %d made at %d:%d:%d in %d\\%d\\%d\n", commits[i].id, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
            printf("\t\tOn branch \'%s\' :\n", commits[i].branch);
            printf("\t\t\"%s\"\n", commits[i].msg);
            printf("\t\t%s  %s\n", commits[i].authName, commits[i].authEmail);
            printf("\t\t%-2d file(s) were commited\n", commits[i].num);
        }
    }
}

int logcg(char *inp, Commit *com)
{
    return 1;
}

int logcn(char *num, Commit *com)
{
    int n;
    char suc = sscanf(num, "%d", &n);
    if (suc == -1)
        return 0;
    if (commitCount - (com - commits) <= n)
        return 1;
    else
        return 0;
}

int logcb(char *branch, Commit *com)
{
    if (strcmp(com->branch, branch) == 0)
        return 1;
    else
        return 0;
}

int logca(char *autName, Commit *com)
{
    if (strcmp(com->authName, autName) == 0)
        return 1;
    else
        return 0;
}

int logcdSince(char *inpdate, Commit *com)
{
    int y, m = 0, d = 0, h = 0, min = 0, s = 0;
    time_t curtime;
    time(&curtime);
    struct tm *ti = localtime(&curtime);

    if (strrchr(inpdate, '\\') == NULL && strrchr(inpdate, ':') != NULL)
    {
        sscanf(inpdate, "%d:%d:%d", &h, &m, &s);
    }
    else if (strrchr(inpdate, '\\') != NULL)
    {
        sscanf(inpdate, "%d\\%d\\%d  %d:%d:%d", &y, &m, &d, &h, &min, &s);
        if (y > 2000)
            ti->tm_year = y - 1900;
        else
            ti->tm_year = y + 100;
        ti->tm_mon = m - 1;
        ti->tm_mday = d;
    }
    else
        return 0;

    ti->tm_hour = h;
    ti->tm_min = m;
    ti->tm_sec = s;
    time_t targetTime = mktime(ti);
    if (targetTime < com->t)
        return 1;
    else
        return 0;
}
int logcdBefore(char *inpdate, Commit *com)
{
    int y, m = 0, d = 0, h = 0, min = 0, s = 0;
    time_t curtime;
    time(&curtime);
    struct tm *ti = localtime(&curtime);

    if (strrchr(inpdate, '\\') == NULL && strrchr(inpdate, ':') != NULL)
    {
        sscanf(inpdate, "%d:%d:%d", &h, &m, &s);
    }
    else if (strrchr(inpdate, '\\') != NULL)
    {
        sscanf(inpdate, "%d\\%d\\%d  %d:%d:%d", &y, &m, &d, &h, &min, &s);
        if (y > 2000)
            ti->tm_year = y - 1900;
        else
            ti->tm_year = y + 100;
        ti->tm_mon = m - 1;
        ti->tm_mday = d;
    }
    else
        return 0;

    ti->tm_hour = h;
    ti->tm_min = m;
    ti->tm_sec = s;
    time_t targetTime = mktime(ti);
    if (targetTime > com->t)
        return 1;
    else
        return 0;
}

int logcw(char *word, Commit *com)
{
    if (strstr(com->msg, word) == NULL)
        return 0;
    else
        return 1;
}

void status(char *fileName)
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
    else if ((GetFileAttributes(fileName) & FILE_ATTRIBUTE_DIRECTORY))
    {
        dirChange(fileName, "*", 0);

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(fileName, &findFileData);
        FindNextFile(hFind, &findFileData);
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0)
            {
                char path[DIRNAME_LEN];
                strcpy(path, fileName);
                dirChange(path, findFileData.cFileName, 1);

                status(path);
            }
        }
        FindClose(hFind);
    }
    else
    {
        int stageIdx = checkPathInFIle(fileName, "stagedfiles.neogit");
        int logIdx = checkInFilelog(fileName);
        char Y, X;

        if (stageIdx == -1)
            X = '-';
        else
            X = '+';

        if (logIdx == -1)
            Y = 'A';
        else
        {
            dirChange(dir, "filelog.neogit", 0);
            FILE *filelog = fopen(dir, "r");
            Fileinfo fileinfo;
            fseek(filelog, logIdx * sizeof(Fileinfo), SEEK_SET);
            fread(&fileinfo, sizeof(Fileinfo), 1, filelog);
            fclose(filelog);

            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(fileName, &findFileData);

            FILETIME lastWriteTime = findFileData.ftLastWriteTime;
            ULARGE_INTEGER lastWriteTimeULarge;
            lastWriteTimeULarge.LowPart = lastWriteTime.dwLowDateTime;
            lastWriteTimeULarge.HighPart = lastWriteTime.dwHighDateTime;

            time_t lastWriteTimeInSeconds = (time_t)((lastWriteTimeULarge.QuadPart - 116444736000000000) / 10000000);
            if (lastWriteTimeInSeconds > fileinfo.lastCommit)
            {
                Y = 'M';
            }
            else
                return;
        }

        printf("\t%-55s : %c%c\n", fileName, X, Y);
    }
}

void statusD()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
        return;

    dirChange(dir, "filelog.neogit", 0);
    FILE *filelog = fopen(dir, "r");
    int placement = 0;
    Fileinfo fileinfo;
    while (fread(&fileinfo, sizeof(Fileinfo), 1, filelog))
    {
        if (GetFileAttributes(fileinfo.path) == INVALID_FILE_ATTRIBUTES && strcmp(fileinfo.path, EMPTY_STRING) != 0)
        {
            char Y = 'D';
            char X;
            if (checkPathInFIle(fileinfo.path, "stagedfiles.neogit") == -1)
                X = '-';
            else
                X = '+';
            printf("\t%-55s : %c%c\n", fileinfo.path, X, Y);
        }
        placement++;
        fseek(filelog, placement * sizeof(fileinfo), SEEK_SET);
    }
}

void writeAlias(char *name, char *cmd, char mode)
{
    Alias alias;
    strcpy(alias.aname, name);
    strcpy(alias.cmd, cmd);
    char dir[DIRNAME_LEN];
    if (mode == LOCAL)
    {
        findNeogitRep(dir);
        dirChange(dir, "localalias.neogit", 0);
    }
    else if (mode == GLOBAL)
    {
        GetModuleFileName(NULL, dir, DIRNAME_LEN);
        dirChange(dir, "aliasfile.neogit", 1);
        puts(dir);
    }
    FILE *aliasfile = fopen(dir, "a");
    fwrite(&alias, sizeof(Alias), 1, aliasfile);
    fclose(aliasfile);
}

int exAlias(char *inp)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "localalias.neogit", 0);
    FILE *aliasfile = fopen(dir, "r");
    Alias alias;
    int place = 0;
    while (fread(&alias, sizeof(Alias), 1, aliasfile))
    {
        if (strcmp(alias.aname, inp) == 0)
        {
            char cmd[DATASTR_LEN] = "neogit ";
            strcat(cmd, alias.cmd);
            system(cmd);
            return 1;
        }
        place++;
        fseek(aliasfile, place * sizeof(Alias), SEEK_SET);
    }
    fclose(aliasfile);
    GetModuleFileName(NULL, dir, DIRNAME_LEN);
    dirChange(dir, "aliasfile.neogit", 1);
    aliasfile = fopen(dir, "r");
    while (fread(&alias, sizeof(Alias), 1, aliasfile))
    {
        if (strcmp(alias.aname, inp) == 0)
        {
            char cmd[DATASTR_LEN] = "neogit ";
            strcat(cmd, alias.cmd);
            system(cmd);
            return 1;
        }
    }
    return 0;
}

void set(const char *sname, const char *msg)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO.");
        return;
    }
    dirChange(dir, "shortcuts.neogit", 0);
    FILE *shortcuts = fopen(dir, "a");
    Shortcut shortcut;
    strcpy(shortcut.sname, sname);
    strcpy(shortcut.msg, msg);
    fwrite(&shortcut, sizeof(Shortcut), 1, shortcuts);
    fclose(shortcuts);
}

int findShortcut(const char *sname, char *msg, char mode)
{
    Shortcut shortcut;
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO.");
        return 0;
    }
    dirChange(dir, "shortcuts.neogit", 0);
    FILE *shortcuts = fopen(dir, "r+");
    while (fread(&shortcut, sizeof(Shortcut), 1, shortcuts))
    {
        if (strcmp(shortcut.sname, sname) == 0)
        {
            if (strcmp(shortcut.msg, EMPTY_STRING) == 0 && mode == READ)
                continue;

            if (mode == REPLACE)
                strcpy(shortcut.msg, msg);
            else if (mode == READ)
                strcpy(msg, shortcut.msg);

            fseek(shortcuts, (long)(-1 * sizeof(Shortcut)), SEEK_CUR);
            fwrite(&shortcut, sizeof(Shortcut), 1, shortcuts);
            return 1;
        }
    }
    puts("ERROR : INVALID SHORTCUT NAME!");
    return 0;
}

void createBranch(char *branchName)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(dir, "status.neogit", 0);
    FILE *status = fopen(dir, "w");
    fprintf(status, "%s", branchName);
    fclose(status);

    dirChange(dir, branchName, 1);
    CreateDirectory(dir, NULL);

    dirChange(dir, "branchhead.neogit", 0);
    FILE *branchhead = fopen(dir, "w");
    fprintf(branchhead, "%d", head->id);
    fclose(branchhead);
}

void branch()
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(dir, "status.neogit", 0);
    FILE *status = fopen(dir, "r");
    char branch[DATASTR_LEN];
    fgets(branch, DATASTR_LEN, status);
    fclose(status);

    dirChange(dir, "*", 1);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(dir, &findFileData);
    do
    {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(findFileData.cFileName, ".") != 0 &&
            strcmp(findFileData.cFileName, "..") != 0 && strcmp(findFileData.cFileName, ".neogit") != 0 &&
            strcmp(findFileData.cFileName, "commits") != 0 && findFileData.dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            printf("   %s", findFileData.cFileName);
            if (strcmp(branch, findFileData.cFileName) == 0)
                printf("  <-- current branch");
            printf("\n");
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
}

void checkoutid(char *id)
{
    char comfol[DIRNAME_LEN];
    findNeogitRep(comfol);
    if (*comfol == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(comfol, "commits", 0);
    dirChange(comfol, id, 0);

    if (GetFileAttributes(comfol) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(comfol) & FILE_ATTRIBUTE_DIRECTORY))
    {
        puts("ERROR: NOT A VALID COMMIT ID!");
        return;
    }

    char main[DIRNAME_LEN];
    findNeogitRep(main);
    char *lastbs = strrchr(main, '\\');
    *lastbs = '\0';

    char mainfilepath[DIRNAME_LEN];
    strcpy(mainfilepath, main);
    dirChange(mainfilepath, "*", 0);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(mainfilepath, &findFileData);
    FindNextFile(hFind, &findFileData);
    while (FindNextFile(hFind, &findFileData) != 0)
    {
        if (strcmp(findFileData.cFileName, ".neogit") != 0)
        {
            strcpy(mainfilepath, main);
            dirChange(mainfilepath, findFileData.cFileName, 0);
            char cmd[DATASTR_LEN];
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                strcpy(cmd, "rmdir /s /q ");
            else
                strcpy(cmd, "del ");
            strcat(cmd, mainfilepath);
            system(cmd);
        }
    }

    char comfilepath[DIRNAME_LEN];
    strcpy(comfilepath, comfol);
    dirChange(comfilepath, "*", 0);

    findFileData;
    hFind = FindFirstFile(comfilepath, &findFileData);
    FindNextFile(hFind, &findFileData);
    while (FindNextFile(hFind, &findFileData) != 0)
    {
        strcpy(comfilepath, comfol);
        dirChange(comfilepath, findFileData.cFileName, 0);
        copydir(comfilepath, main, COPY_ALL);
    }

    CloseHandle(hFind);

    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    dirChange(dir, "stagedfiles.neogit", 0);
    FILE *stagedfiles = fopen(dir, "w");
    dirChange(dir, "resetfiles.neogit", 1);
    FILE *resetfiles = fopen(dir, "w");
    fclose(stagedfiles);
    fclose(resetfiles);
}

void checkoutb(char *branch)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(dir, branch, 0);
    if (GetFileAttributes(dir) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(dir) & FILE_ATTRIBUTE_DIRECTORY))
    {
        puts("ERROR: INVALID BRANCH NAME!");
        return;
    }

    dirChange(dir, "branchhead.neogit", 0);
    char branchheadid[DATASTR_LEN];
    FILE *branchhead = fopen(dir, "r");
    fscanf(branchhead, "%s", branchheadid);
    fclose(branchhead);

    checkoutid(branchheadid);

    dirChange(dir, "status.neogit", 2);
    FILE *status = fopen(dir, "w");
    fprintf(status, "%s", branch);
    fclose(status);

    findHead();
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

void tag(char *name, char *msg, char *comid, char mode)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(dir, "localconfigs.neogit", 0);
    FILE *config = fopen(dir, "rb");

    dirChange(dir, "tags.neogit", 1);
    FILE *tags = fopen(dir, "rb+");
    if (tags == NULL)
        tags = fopen(dir, "wb+");

    Tag tag;
    strcpy(tag.name, name);

    fread(&tag.authName, 1, DATASTR_LEN, config);
    fread(&tag.authEmail, 1, DATASTR_LEN, config);
    fclose(config);

    time(&tag.createT);

    if (msg == NULL)
        *tag.msg = '\0';
    else
        strcpy(tag.msg, msg);

    if (comid == NULL)
        tag.comid = head->id;
    else
    {
        sscanf(comid, "%d", &tag.comid);
        if (tag.comid - 10000 >= commitCount)
        {
            puts("ERROR: INVALID COMMIT ID!");
            return;
        }
    }

    Tag lestag[MAX_COMMIT_NUM];
    int num = fread(lestag, sizeof(Tag), MAX_COMMIT_NUM, tags);

    int i = 0;
    for (; i < num; i++)
    {
        if (strcmp(tag.name, lestag[i].name) <= 0)
            break;
    }

    fclose(tags);
    tags = fopen(dir, "wb");

    if (i == 0)
        fwrite(&tag, sizeof(Tag), 1, tags);
    for (int j = 0; j < num; j++)
    {
        if (!(mode == REPLACE && strcmp(lestag[i].name, tag.name) == 0)) // bug
            fwrite(lestag + j, sizeof(Tag), 1, tags);
        if (j + 1 == i)
            fwrite(&tag, sizeof(Tag), 1, tags);
    }

    fclose(tags);
}

void showtag(char *name)
{
    char dir[DIRNAME_LEN];
    findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }

    dirChange(dir, "tags.neogit", 0);
    FILE *tags = fopen(dir, "rb");
    if (tags == NULL)
    {
        puts("ERROR: NO AVAILABLE TAGS!");
        return;
    }

    Tag lestag[MAX_COMMIT_NUM];
    int num = fread(lestag, sizeof(Tag), MAX_COMMIT_NUM, tags);

    for (int i = 0; i < num; i++)
    {
        Tag *tag = lestag + i;
        if (name == NULL)
        {
            struct tm *timeinfo = localtime(&tag->createT);
            printf("\tTag \'%s\':\n\t\tcreated at %.2d:%.2d:%.2d on %d\\%.2d\\%.2d\n", tag->name, timeinfo->tm_hour,
                   timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_year - 100, timeinfo->tm_mon + 1, timeinfo->tm_mday);
            if (*tag->msg != '\0')
                printf("\t\t\"%s\"\n", tag->msg);
            printf("\t\t%s %s\n", tag->authName, tag->authEmail);
            printf("\t\tOn commit %d\n", tag->comid);
        }
        else if (strcmp(name, tag->name) == 0)
        {
            struct tm *timeinfo = localtime(&tag->createT);
            printf("\tTag \'%s\':\n\t\tcreated at %.2d:%.2d:%.2d on %d\\%.2d\\%.2d\n", tag->name, timeinfo->tm_hour,
                   timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_year - 100, timeinfo->tm_mon + 1, timeinfo->tm_mday);
            if (*tag->msg != '\0')
                printf("\t\t\"%s\"\n", tag->msg);
            printf("\t\t%s %s\n", tag->authName, tag->authEmail);
            printf("\t\tOn commit %d\n", tag->comid);
        }
    }
}

int diff(char *file1dir, char *file2dir)
{
    FILE *file1 = fopen(file1dir, "r");
    FILE *file2 = fopen(file2dir, "r");

    char l1[DATASTR_LEN];
    char l2[DATASTR_LEN];

    int line1num = 0;
    int line2num = 0;
    int diffnum = 0;
    while (!feof(file1) && !feof(file2))
    {
        fgets(l1, DATASTR_LEN, file1);
        fgets(l2, DATASTR_LEN, file2);
        line1num++;
        line2num++;

        if (strrchr(l1, '\n') != NULL)
            *strrchr(l1, '\n') = '\0';
        char *w1 = findprintable(l1);

        if (strrchr(l2, '\n') != NULL)
            *strrchr(l2, '\n') = '\0';
        char *w2 = findprintable(l2);

        while (w1 == NULL)
        {
            if (fgets(l1, DATASTR_LEN, file1))
            {
                if (strrchr(l1, '\n') != NULL)
                    *strrchr(l1, '\n') = '\0';
                w1 = findprintable(l1);
                line1num++;
            }
            else
                break;
        }
        while (w2 == NULL)
        {
            if (fgets(l2, DATASTR_LEN, file2))
            {
                if (strrchr(l2, '\n') != NULL)
                    *strrchr(l2, '\n') = '\0';
                w2 = findprintable(l2);
                line2num++;
            }
            else
                break;
        }
        int wordnum = -1;
        int lastdiffword = -1;
        do
        {
            wordnum++;
            if (w2 == NULL || (w1 != NULL && strncmp(w1, w2, findtokenlen(w1, w2)) != 0))
            {
                diffnum++;
                lastdiffword = wordnum;
            }
            else if (w1 == NULL || strncmp(w1, w2, findtokenlen(w1, w2)) != 0)
            {
                diffnum++;
                lastdiffword = wordnum;
            }

            if (w1 != NULL)
                w1 = findprintable(strstr(w1, " "));
            if (w2 != NULL)
                w2 = findprintable(strstr(w2, " "));

        } while (w1 || w2);

        if (diffnum > 0)
        {
            puts(">>>>>>");
            printf("%s -> line %d :\n    ", file1dir, line1num);

            setTextColor(RED);
            printline(l1, diffnum, lastdiffword);
            setTextColor(WHITE);

            printf("%s -> line %d :\n    ", file2dir, line2num);

            setTextColor(GREEN);
            printline(l2, diffnum, lastdiffword);
            setTextColor(WHITE);
            puts("<<<<<<");
            puts("");
        }
    }

    char line[DATASTR_LEN];

    while (fgets(line, DATASTR_LEN, file1))
    {
        line1num++;

        if (strrchr(line, '\n') != NULL)
            *strrchr(line, '\n') = '\0';

        if (findprintable(line) == NULL)
            continue;

        puts(">>>>>>");
        printf("%s -> line %d :\n    ", file1dir, line1num);

        setTextColor(RED);
        printline(line, -1, -1);
        setTextColor(WHITE);
        puts("<<<<<<");
        puts("");
        diffnum++;
    }
    while (fgets(line, DATASTR_LEN, file2))
    {
        line2num++;

        if (strrchr(line, '\n') != NULL)
            *strrchr(line, '\n') = '\0';

        if (findprintable(line) == NULL)
            continue;

        puts(">>>>>>");
        printf("%s -> line %d :\n    ", file2dir, line2num);

        setTextColor(GREEN);
        printline(line, -1, -1);
        setTextColor(WHITE);
        puts("<<<<<<");
        puts("");
        diffnum++;
    }

    fclose(file1);
    fclose(file2);
    return diffnum;
}

void printline(char *lc, int diffnum, int lastdiffword)
{
    int wordcount = 0;
    while (*lc != '\0')
    {
        while (*lc == ' ' && *lc != '\0')
        {
            printf("%c", *lc);
            lc++;
        }
        if (lastdiffword == wordcount && diffnum == 1)
            ///////////////////////////////////////////////////////////////
            /////////////////////A MOMENT OF SILENCE//////////////////////
            ///////////////////////FOR ALL THOSE/////////////////////////
            /////////////////////WHO FELL FIGHTING//////////////////////
            //////////////////////AGAINST FACISM///////////////////////
            ///////////////////////:((((((((((////////////////////////
            /////////////////////////////////////////////////////////
            printf(">>");
        while (*lc != ' ' && *lc != '\0')
        {
            printf("%c", *lc);
            lc++;
        }
        if (lastdiffword == wordcount && diffnum == 1)
            printf("<<");
        wordcount++;
    }
    printf("\n");
}

char *findprintable(char *ptr)
{
    if (ptr == NULL)
        return NULL;
    while (*ptr == ' ' || *ptr == '\0')
        if (*ptr == '\0')
            return NULL;
        else
            ptr++;
    return ptr;
}

int findtokenlen(char *ptr, char *nptr)
{
    int i = 0;
    while (*ptr != ' ' && *ptr != '\0')
    {
        ptr++;
        i++;
    }
    int j = 0;
    while (*nptr != ' ' && *nptr != '\0')
    {
        nptr++;
        j++;
    }
    if (i > j)
        return i;
    else
        return j;
}

void setTextColor(int colorCode)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
}

void commitdiff(char *com1, char *com2)
{
    char dir1[DIRNAME_LEN];
    findNeogitRep(dir1);
    if (dir1 == NULL)
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }
    dirChange(dir1, "commits", 0);
    dirChange(dir1, com1, 0);
    if (GetFileAttributes(dir1) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(dir1) & FILE_ATTRIBUTE_DIRECTORY))
    {
        puts("ERROR: INVALID COMMIT ID!");
        return;
    }
    char root[DIRNAME_LEN] = "";

    char dir2[DIRNAME_LEN];
    findNeogitRep(dir2);
    dirChange(dir2, "commits", 0);
    dirChange(dir2, com2, 0);
    if (GetFileAttributes(dir1) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(dir1) & FILE_ATTRIBUTE_DIRECTORY))
    {
        puts("ERROR: INVALID COMMIT ID!");
        return;
    }

    findcomfiles(dir1, dir2, root, com1, NO_REPLACE);
    findcomfiles(dir2, dir1, root, com2, REPLACE);
}

void findcomfiles(char *path1, char *path2, char *root, char *comid, char mode)
{
    if ((GetFileAttributes(path1) & FILE_ATTRIBUTE_DIRECTORY) && (GetFileAttributes(path1) != INVALID_FILE_ATTRIBUTES))
    {
        char path[DIRNAME_LEN];
        strcpy(path, path1);
        dirChange(path, "*", 0);

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(path, &findFileData);
        FindNextFile(hFind, &findFileData);
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            dirChange(path, findFileData.cFileName, 1);

            char rootpath[DIRNAME_LEN];
            strcpy(rootpath, root);
            dirChange(rootpath, findFileData.cFileName, 0);

            findcomfiles(path, path2, rootpath, comid, mode);
        }
        FindClose(hFind);
        return;
    }
    else
    {
        root++;
        char tdir[DIRNAME_LEN];
        strcpy(tdir, path2);
        dirChange(tdir, root, 0);
        if (GetFileAttributes(tdir) == INVALID_FILE_ATTRIBUTES || (GetFileAttributes(tdir) & FILE_ATTRIBUTE_DIRECTORY))
        {
            printf("Was in repo exclusively in commit %s\n", comid);
            printf("    ");
            puts(root);
            return;
        }
        else if (mode == REPLACE)
            return;
        else
        {
            diff(path1, tdir);
            return;
        }
    }
}

void merge(char *branch1, char *branch2) // perv commit of this commit needs to be corrected
{
    char root[DIRNAME_LEN];
    int bsnum = findNeogitRep(root);
    if (*root == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }
    *strrchr(root, '\\') = '\0';

    // finding b1head and b1head folder dir
    char b1hdir[DIRNAME_LEN];
    char b1headid[DATASTR_LEN];
    findNeogitRep(b1hdir);
    dirChange(b1hdir, branch1, 0);
    dirChange(b1hdir, "branchhead.neogit", 0);
    FILE *branchhead = fopen(b1hdir, "r");
    fscanf(branchhead, "%s", b1headid);
    dirChange(b1hdir, "commits", 2);
    dirChange(b1hdir, b1headid, 0);
    fclose(branchhead);

    // finding b2head and b2head folder dir
    char b2hdir[DIRNAME_LEN];
    char b2headid[DATASTR_LEN];
    findNeogitRep(b2hdir);
    dirChange(b2hdir, branch2, 0);
    dirChange(b2hdir, "branchhead.neogit", 0);
    branchhead = fopen(b2hdir, "r");
    fscanf(branchhead, "%s", b2headid);
    dirChange(b2hdir, "commits", 2);
    dirChange(b2hdir, b2headid, 0);
    fclose(branchhead);

    checkoutid(b1headid);

    int conflict = copymerge(b2hdir, root, b2headid);
    if (conflict)
    {
        char headid[DATASTR_LEN];
        sprintf(headid, "%d", head->id);
        checkoutid(headid);
        puts("ERROR: CONFLICT! MERGE ABORTED!");
        return;
    }

    head = &commits[atoi(b1headid) - 10000];

    char reldir[DIRNAME_LEN] = "*$";
    for (int i = 0; i < bsnum; i++)
    {
        char temp[DIRNAME_LEN] = "";
        sprintf(temp, "..\\%s", reldir);
        strcpy(reldir, temp);
    }
    char branchName[DATASTR_LEN];
    snprintf(branchName, DATASTR_LEN, "%s-%s", branch1, branch2);
    createBranch(branchName);

    wildcard(reldir, add, '\0');

    int stagedFilesNum = filelog();
    char commsg[DATASTR_LEN];
    snprintf(commsg, DATASTR_LEN, "merging \'%s\' and \'%s\'.");
    commit(commsg, stagedFilesNum, NULL);
    snapshot();
}

int copymerge(char *src, char *dest, char *commitid)
{
    if (GetFileAttributes(src) == INVALID_FILE_ATTRIBUTES)
    {
        fputs("ERROR: INVALID COMMIT ID", stdout);
        return 1;
    }
    else if (GetFileAttributes(src) & FILE_ATTRIBUTE_DIRECTORY)
    {
        // finding the name of the folder and creating it
        char *lastbs = strrchr(src, '\\');
        char foldername[DIRNAME_LEN];
        strcpy(foldername, dest);
        if (strcmp(lastbs + 1, commitid) != 0)
        {
            strcat(foldername, lastbs);
            if (GetFileAttributes(foldername) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(foldername) & FILE_ATTRIBUTE_DIRECTORY))
                CreateDirectory(foldername, NULL);
        } // coping commitd folder path and searching its contents
        char subfilepath[DIRNAME_LEN];
        strcpy(subfilepath, src);

        dirChange(subfilepath, "*", 0);
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(subfilepath, &findFileData);
        FindNextFile(hFind, &findFileData);

        int conflict = 0;
        while (FindNextFile(hFind, &findFileData) != 0)
        {
            strcpy(subfilepath, src);
            dirChange(subfilepath, findFileData.cFileName, 0);
            conflict += copymerge(subfilepath, foldername, commitid);
        }
        CloseHandle(hFind);
        return conflict;
    }
    else
    {
        char *lastbs = strrchr(src, '\\');
        char destfilepath[DATASTR_LEN];
        strcpy(destfilepath, dest);
        strcat(destfilepath, lastbs);

        if (GetFileAttributes(destfilepath) == INVALID_FILE_ATTRIBUTES || (GetFileAttributes(destfilepath) & FILE_ATTRIBUTE_DIRECTORY))
        {
            CopyFile(src, destfilepath, 0);
            return 0;
        }
        else
        {
            if (diff(src, destfilepath))
                return 1;
            else
                return 0;
        }
    }
}

void revert(char *id, char *msg)
{
    char dir[DIRNAME_LEN];
    int bsnum = findNeogitRep(dir);
    if (*dir == '\0')
    {
        puts("ERROR: NOT IN A NEOGIT REPO!");
        return;
    }
    dirChange(dir, "commits", 0);
    dirChange(dir, id, 0);
    if (GetFileAttributes(dir) == INVALID_FILE_ATTRIBUTES || !(GetFileAttributes(dir) & FILE_ATTRIBUTE_DIRECTORY))
    {
        puts("ERROR: INVALID COMMIT ID!");
        return;
    }

    if (msg == NULL)
    {
        char temp[DATASTR_LEN];
        msg = temp;
        strcpy(msg, commits[atoi(id) - 10000].msg);
    }

    char reldir[DIRNAME_LEN] = "*$";
    for (int i = 0; i < bsnum; i++)
    {
        char temp[DIRNAME_LEN] = "";
        sprintf(temp, "..\\%s", reldir);
        strcpy(reldir, temp);
    }

    checkoutid(id);
    wildcard(reldir, add, '\0');
    int num = filelog();
    commit(msg, num, NULL);
    snapshot();
}