/*
 * tunimake.c
 * A simple makefile-like utility for compiling and linking C programs.
 * The rules for compiling are in plussa instructions
 * made with help of github copilot
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

// it also determines the max length of some filenames confusingly
#define MAX_LINE_LENGTH 256
#define MAX_FILES 64

char compiler[MAX_LINE_LENGTH];
char flags[MAX_LINE_LENGTH];
char source_files[MAX_FILES][MAX_LINE_LENGTH];
char header_files[MAX_FILES][MAX_LINE_LENGTH];
char libraries[MAX_FILES][MAX_LINE_LENGTH];
char executable[MAX_LINE_LENGTH];

int num_source_files = 0;
int num_header_files = 0;
int num_libraries = 0;

/*
 * Take in a string ptr and modify it to remove leading and trailing whitespace characters.
 * source: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 */
char *trim_whitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

/*
 * parse the rulefile according the rules. Look at the first character of each line to determine the rule.
 * The rules are as follows:
 * K: compiler
 * F: flags
 * C: source files
 * H: header files
 * L: libraries
 * E: executable
 * the result of the parsing is stored in the global variables.
 */
void parse_rulefile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening rulefile");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }

        char *token = strtok(line, " ");
        if (strcmp(token, "K") == 0)
        {
            strcpy(compiler, trim_whitespace(strtok(NULL, "\n")));
        }
        else if (strcmp(token, "F") == 0)
        {
            strcpy(flags, trim_whitespace(strtok(NULL, "\n")));
        }
        else if (strcmp(token, "C") == 0)
        {
            token = strtok(NULL, ",");
            while (token)
            {
                strcpy(source_files[num_source_files++], trim_whitespace(token));
                token = strtok(NULL, ",");
            }
        }
        else if (strcmp(token, "H") == 0)
        {
            token = strtok(NULL, ",");
            while (token)
            {
                strcpy(header_files[num_header_files++], trim_whitespace(token));
                token = strtok(NULL, ",");
            }
        }
        else if (strcmp(token, "L") == 0)
        {
            token = strtok(NULL, ",");
            while (token)
            {
                strcpy(libraries[num_libraries++], trim_whitespace(token));
                token = strtok(NULL, ",");
            }
        }
        else if (strcmp(token, "E") == 0)
        {
            strcpy(executable, trim_whitespace(strtok(NULL, "\n")));
        }
    }

    fclose(file);
}

/*
 * Check if the source file needs to be recompiled. This is done by comparing the modification time of the source file and the object file.
 * Return 1 if the source file needs to be recompiled, 0 otherwise. (Because C has no built in boolean type i guess)
 */
int needs_recompilation(const char *source_file, const char *object_file)
{
    struct stat source_stat, object_stat;
    printf("Checking source file: %s\n", source_file);
    if (stat(source_file, &source_stat) != 0)
    {
        perror("Error getting source file status");
        exit(EXIT_FAILURE);
    }

    printf("Checking object file: %s\n", object_file);
    if (stat(object_file, &object_stat) != 0)
    {
        printf("Object file does not exist\n");
        return 1;
    }

    if (source_stat.st_mtime > object_stat.st_mtime)
    {
        printf("Source file is newer than object file\n");
        return 1;
    }

    for (int i = 0; i < num_header_files; i++)
    {
        struct stat header_stat;
        printf("Checking header file: %s\n", header_files[i]);
        if (stat(header_files[i], &header_stat) != 0)
        {
            perror("Error getting header file status");
            exit(EXIT_FAILURE);
        }

        if (header_stat.st_mtime > object_stat.st_mtime)
        {
            printf("Header file is newer than object file\n");
            return 1;
        }
    }

    printf("Source file does not need to be recompiled\n");
    return 0;
}

/*
 * Compile the source file into an object file. The object file is named the same as the source file, but with a .o extension.
 * Compilation is done according to the parsed command by a new process while the parent process waits for the child to finish.
 */
void compile_source_file(const char *source_file)
{
    char object_file[MAX_LINE_LENGTH];
    snprintf(object_file, sizeof(object_file), "%.252s.o", source_file);

    if (!needs_recompilation(source_file, object_file))
    {
        return;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        execlp(compiler, compiler, flags, "-c", source_file, "-o", object_file, NULL);
        perror("Error executing compiler");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
        {
            fprintf(stderr, "Compilation failed for %s\n", source_file);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }
}

/*
 * Link the object files into an executable.
 Linking is done by forking a child process and executing the compiler with the appropriate flags.
 */
void link_object_files()
{
    // The maximum number of arguments is the number of source files + 5 (compiler, flags, -o, executable, NULL)
    char *args[MAX_FILES + 5];
    int arg_index = 0;

    args[arg_index++] = compiler;
    args[arg_index++] = flags;

    for (int i = 0; i < num_source_files; i++)
    {
        char object_file[MAX_LINE_LENGTH];
        snprintf(object_file, sizeof(object_file), "%.252s.o", source_files[i]);
        args[arg_index++] = strdup(object_file);
    }

    for (int i = 0; i < num_libraries; i++)
    {
        char lib_arg[MAX_LINE_LENGTH];
        snprintf(lib_arg, sizeof(lib_arg), "-l%.252s", libraries[i]);
        args[arg_index++] = strdup(lib_arg);
    }

    args[arg_index++] = "-o";
    args[arg_index++] = executable;
    args[arg_index] = NULL;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(compiler, args);
        perror("Error executing linker");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
        {
            fprintf(stderr, "Linking failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    const char *rulefile = (argc > 1) ? argv[1] : "tunimakefile";

    parse_rulefile(rulefile);

    if (strlen(compiler) == 0 || num_source_files == 0 || strlen(executable) == 0)
    {
        fprintf(stderr, "Error: Missing required information in rulefile\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_source_files; i++)
    {
        compile_source_file(source_files[i]);
    }

    link_object_files();

    return 0;
}