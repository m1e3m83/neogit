#include "neogit.h"

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
        commits[i].pervCommit = commits + (commits[i].prevcomid - 10000);

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
    fread(branch, 1, DATASTR_LEN, status);
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
    curCommit->prevcomid = head->id;

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

void cleanCommit(char *fileprevpath)
{
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
