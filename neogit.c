#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define GLOBAL 'g'
#define LOCAL 'l'
#define DIRNAME_LEN 100

void config(char, char *, char *);

int main()
{
    config(LOCAL, "ali", "ali@ali.com");
    return 0;
}

void config(char mode, char *username, char *email)
{
    if (mode == GLOBAL)
    {
        // updates configFile.txt in main dir
        char exefileDir[DIRNAME_LEN];

        GetModuleFileName(NULL, exefileDir, DIRNAME_LEN);
        char *lastbs = strrchr(exefileDir, '\\');
        if (lastbs != NULL)
            *lastbs = '\0';
        strcat(exefileDir, "\\configfile.txt");

        FILE *configFile = fopen(exefileDir, "w");
        fprintf(configFile, "%s\n%s", username, email);
        fclose(configFile);

        // if in a project folder resets local configs
        unsigned long fileatt = GetFileAttributes(".neogit\\localconfigs.txt");
        if (fileatt != INVALID_FILE_ATTRIBUTES && fileatt != FILE_ATTRIBUTE_DIRECTORY)
        {
            FILE *localConfig = fopen(".neogit\\localconfigs.txt", "w");
            fputs("GLOBAL", localConfig);
            fclose(localConfig);
        }
    }
    else if (mode == LOCAL)
    {
        // check if the file exists
        if (GetFileAttributes(".neogit") != FILE_ATTRIBUTE_DIRECTORY)
        {
            fputs("ERROR: NOT IN A REPOSITORY FOLDER!", stdout);
            return;
        }

        // resets local configs
        FILE *localConfig = fopen(".neogit\\localconfigs.txt", "w");
        fprintf(localConfig, "LOCAL %s %s\n", username, email);
    }
}