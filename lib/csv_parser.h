#ifndef csv_parser_h
#define csv_parser_h

#include <stdlib.h>
#include <string.h>

#include "memory.h"

#define PARSER_ERROR 0
#define PARSER_SUCCESS 1

typedef struct
{
    char **keys;
    char **line;
    int keys_count;
} csv_parser_t;

int csv_parser_init(csv_parser_t **parser)
{
    *parser = (csv_parser_t *)malloc(sizeof(csv_parser_t));
    if (*parser == NULL)
        return PARSER_ERROR;
    (*parser)->keys = (char **)0xdeadbeef;
    (*parser)->line = (char **)0xdeadbeef;
    (*parser)->keys_count = 0;

    return PARSER_SUCCESS;
}

void csv_parser_free(csv_parser_t **parser)
{
    for (int i = 0; i < (*parser)->keys_count; i++)
    {
        check_free((*parser)->keys[i]);
    }
    check_free((*parser)->keys);
    check_free((*parser)->line);
    check_free(*parser);
}

int read_from_line(csv_parser_t *parser, char *line)
{
    // read keys
    if (parser->keys_count == 0)
    {
        // count values
        for (int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == ',' || (line[i] == '\n' && i != 0))
            {
                parser->keys_count++;
            }
        }

        if (parser->keys_count == 0)
            return PARSER_ERROR;

        // parse keys
        parser->keys = (char **)check_malloc(sizeof(char *) * parser->keys_count);
        parser->line = (char **)check_malloc(sizeof(char *) * parser->keys_count);

        char delim[] = ",\n";
        char *ptr = strtok(line, delim);
        int current_key = 0;
        while (ptr != NULL)
        {
            if (ptr[0] == '\0')
                return PARSER_ERROR;
            parser->keys[current_key] = strdup(ptr);
            current_key++;
            ptr = strtok(NULL, delim);
        }

        return PARSER_SUCCESS;
    }
    else
    {
        // Read values
        char delim[] = ",\n";
        char *ptr = strtok(line, delim);
        int current_value = 0;
        while (ptr != NULL)
        {
            if (ptr[0] == '\0')
                return PARSER_ERROR;
            if (current_value >= parser->keys_count)
                return PARSER_ERROR;

            parser->line[current_value] = ptr;
            current_value++;
            ptr = strtok(NULL, delim);
        }

        return PARSER_SUCCESS;
    }

    return PARSER_ERROR;
}

#endif