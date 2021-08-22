#ifndef opt_h
#define opt_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

#define OPT_EOF 0
#define OPT_BADARG 1
#define OPT_BADKEY 2
#define OPT_SUCCESS 3

typedef struct
{
    int total;
    int current_arg;
    int current_args_count;
    const char *const *arguments;
    char **patterns;
    int patterns_count;
} opt_t;

int opt_init(opt_t **opt, char *pattern, const int argc, const char *const *argv)
{
    if (argc <= 0 || argv == NULL || pattern == NULL)
        return OPT_BADARG;

    // Initialize elements
    *opt = (opt_t *)check_malloc(sizeof(opt_t));
    (*opt)->total = argc;
    (*opt)->current_arg = 1;
    (*opt)->arguments = argv;
    (*opt)->current_args_count = 0;

    const char delim[] = ":";
    (*opt)->patterns_count = 0;
    // Count occurrences of :
    for (int i = 0; pattern[i] != '\0'; i++)
    {
        if (pattern[i] == ':')
            (*opt)->patterns_count++;
    }

    char *pattern_str = strdup(pattern);
    (*opt)->patterns = (char **)check_malloc(sizeof(char *) * (*opt)->patterns_count);

    // Save patterns
    char *ptr = strtok(pattern_str, delim);
    int current_pattern = 0;
    while (ptr != NULL)
    {
        (*opt)->patterns[current_pattern] = strdup(ptr);
        current_pattern++;
        ptr = strtok(NULL, delim);
    }

    free(pattern_str);
    return OPT_SUCCESS;
}

void opt_free(opt_t **opt)
{
    check_free((*opt)->patterns);
    check_free(*opt);
}

int get_opt(opt_t *opt, const char **opt_key, char ***opt_args)
{
    if (opt->current_arg == opt->total)
        return OPT_EOF;

    // Check formatting
    if (opt->arguments[opt->current_arg][0] != '-' || opt->arguments[opt->current_arg][1] != '-')
        return OPT_BADARG;

    // Remove --
    (*opt_key) = &(opt->arguments[opt->current_arg])[2];
    opt->current_arg++;

    // Check if key is part of pattern
    int hasKey = 0;
    for (int i = 0; i < opt->patterns_count; i++)
    {
        if (strcmp(opt->patterns[i], *opt_key) == 0)
        {
            hasKey = 1;
            break;
        }
    }

    if (hasKey == 0)
    {
        return OPT_BADKEY;
    }

    if (strcmp(*opt_key, "help") == 0)
    {
        return OPT_SUCCESS;
    }

    int tmp = opt->current_arg;
    // Check how many args until next key or end of args
    // Needed because we don't know ahead of time how many more args left before the next -- or end of args
    while (tmp < opt->total && (opt->arguments[tmp][0] != '-' || opt->arguments[tmp][1] != '-'))
    {
        tmp++;
    }

    int number_of_args = tmp - opt->current_arg;

    for (int i = 0; i < opt->current_args_count; i++)
    {
        check_free((*opt_args)[i]);
    }
    check_free(*opt_args);

    opt->current_args_count = number_of_args;
    *opt_args = (char **)check_malloc(sizeof(char *) * number_of_args);
    for (int i = 0; i < number_of_args; i++)
    {
        (*opt_args)[i] = strdup(opt->arguments[opt->current_arg + i]);
    }
    opt->current_arg += number_of_args;

    return OPT_SUCCESS;
}

#endif