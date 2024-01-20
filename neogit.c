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

int main(int argc, char *argv[])
{
    if (strcmp(argv[1], "config") == 0)
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
    else
        INVCMD;

    return 0;
}

// void config(char mode, char type, char *data)
// {
//     if (mode == GLOBAL)
//     {

//         // if in a project folder resets local configs
//         unsigned long fileatt = GetFileAttributes(".neogit\\localconfigs.txt");
//         if (fileatt != INVALID_FILE_ATTRIBUTES && fileatt != FILE_ATTRIBUTE_DIRECTORY)
//         {
//             FILE *localConfig = fopen(".neogit\\localconfigs.txt", "w");
//             fputs("GLOBAL", localConfig);
//             fclose(localConfig);
//         }
//     }
//     else if (mode == LOCAL)
//     {
//         // check if the file exists
//         if (GetFileAttributes(".neogit") != FILE_ATTRIBUTE_DIRECTORY)
//         {
//             fputs("ERROR: NOT IN A REPOSITORY FOLDER!", stdout);
//             return;
//         }

//         // resets local configs
//         FILE *localConfig = fopen(".neogit\\localconfigs.txt", "w");
//         fprintf(localConfig, "LOCAL %s %s\n", username, email);
//     }
// }

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
        strcat(exefileDir, "\\configfile.txt");

        FILE *configFile = fopen(exefileDir, "r+");
        fseek(configFile, type * DATASTR_LEN, SEEK_SET);
        fwrite(data, sizeof(char), DATASTR_LEN, configFile);
        fclose(configFile);
    }
}