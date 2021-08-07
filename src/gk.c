#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/array.h"
#include "../lib/csv_parser.h"
#include "../lib/hash.h"
#include "../lib/instance.h"
#include "../lib/heap.h"
#include "../lib/memory.h"
#include "../lib/opt.h"
#include "../lib/set.h"
#include "../lib/sorting.h"

typedef struct
{
    int32_t *values;
} GK;
GK *init();
void free_sketch(GK *);

GK *update(GK *, int32_t);
uint32_t rank(GK *);
int32_t quantile(GK *, double);

void print_help();
void print_error_help()
{
    printf("Algo de inesperado aconteceu ao programa, olhe o --help por precaucao.\n\n");
    print_help();
}

void print_help()
{
    printf("$ gk build_options data_file.csv\n[rank | quant] [query_args | --in query_args_file]\n\n");
    printf("--val id_field_no: ");
    printf("Especifica numero da coluna do arquivo de entrada a ser usada para alimentar o sketch. Esta coluna deve conter valores numericos. Cada valor deve ser interpretado como tendo peso 1.\n\n");
    printf("--eps error_bound: ");
    printf("Especifica o valor do parametro e do sketch.\n\n");
    printf("--univ universe_size: ");
    printf("Especifica o valor do parametro U do sketch. Ou seja, consideramos os valores contidos no intervalo [U] = {0, ..., U - 1}. Os valores fora desse intervalo devem ser desconsiderados.\n\n");
    printf("--help : Roda este comando.\n\n");
}

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    int status = opt_init(&opt, "val:eps:univ:help:rank:quant:in:", argc, argv);
    if (status != OPT_SUCCESS)
    {
        print_error_help();
        return -1;
    }

    int id_field_no = 0;
    double error_bound = 0.0f;
    uint32_t universe_size = 0;
    while (get_opt(opt, &opt_key, &opt_args) == OPT_SUCCESS)
    {
        if (strcmp(opt_key, "val") == 0)
        {
            id_field_no = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "eps") == 0)
        {
            error_bound = atof(opt_args[0]);
        }
        else if (strcmp(opt_key, "univ") == 0)
        {
            universe_size = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "help") == 0)
        {
            print_help();
            return 0;
        }
        else if (strcmp(opt_key, "rank") == 0)
        {
            // TODO(fssn): Will require opt rework
        }
        else if (strcmp(opt_key, "quant") == 0)
        {
            // TODO(fssn): Will require opt rework
        }
        else if (strcmp(opt_key, "in") == 0)
        {
            // TODO(fssn): Will require opt rework
        }
    }
    opt_free(&opt);

    const char *filename = strdup(argv[7]);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        print_error_help();
        return -1;
    }

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
        print_error_help();
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    ssize_t lines_read;
    lines_read = getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    GK *sketch = init();
    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        int32_t value = atoi(parser->line[id_field_no]);
        update(sketch, value);
    }

    return 0;
}

// MARK - GK functions
GK *init()
{
    GK *sketch = check_calloc(1, sizeof(GK));
    sketch->values = check_calloc(1, sizeof(int32_t));
    return sketch;
}

void free_sketch(GK *sketch)
{
    check_free(sketch->values);
    check_free(sketch);
}

GK *update(GK *sketch, int32_t value)
{
    return sketch;
}

uint32_t rank(GK *sketch)
{
    return 0;
}

int32_t quantile(GK *sketch, double q)
{
    return 0;
}