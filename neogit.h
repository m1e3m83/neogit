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

#define DATASTR_LEN 128
#define DIRNAME_LEN 128
#define EMPTY_STRING "THIS IS AN EMPTY STRING DESIGNED TO REWRITE THE OTHER STRING THAT LAYED IN HERE. RIP DEAR OLD STRING, ALL HAIL THE NEW STRING. "
#define MAX_COMMIT_NUM 1000

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
void findNeogitRep(char *);
int copydir(char *, char *, char);
void wildcard(char *, void(char *, char), char);
int isNum(char *);

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
