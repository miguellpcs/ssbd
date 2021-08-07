#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/csv_parser.h"
#include "../lib/hash.h"
#include "../lib/instance.h"
#include "../lib/heap.h"
#include "../lib/memory.h"
#include "../lib/opt.h"

typedef enum
{
    AFTER_EVERY_UPDATE,
    WHEN_WEIGHT_DOUBLES,
    AFTER_CAPACITY_INCREASES
} compress_strategy_e;

typedef struct
{
    size_t size;
    double error_bound;
    // TODO(fssn): replace with binary tree root
    Instance *values;
    compress_strategy_e compress_strategy;
    uint64_t saved_weight;
} QDigest;
QDigest *init(size_t, double, compress_strategy_e);
void free_sketch(QDigest *);

void update(QDigest *, Instance);
uint32_t rank(QDigest *);
Instance quantile(QDigest *, double);
uint64_t weight(QDigest *);
uint64_t capacity(QDigest *);
void compress(QDigest *);

void print_help();
void print_error_help()
{
    printf("Algo de inesperado aconteceu ao programa, olhe o --help por precaucao.\n\n");
    print_help();
}

void print_help()
{
    printf("$ qdig build_options data_file.csv\n[rank | quant] [query_args | --in query_args_file]\n\n");
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

    size_t size = (1 / error_bound) * log(universe_size);

    // TODO(fssn): Add compress_strategy as cmd line opt so we can change during experiment
    QDigest *sketch = init(size, error_bound, AFTER_CAPACITY_INCREASES);

    int counter = 0;
    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        int32_t weight = atoi(parser->line[id_field_no]);
        // Skip zero weighted elements
        if (weight <= 0)
            continue;

        counter++;
        counter = counter > universe_size ? universe_size : counter;

        Instance instance = {.val = counter, .weight = weight};
        update(sketch, instance);
    }

    return 0;
}

// MARK - QDigest functions
QDigest *init(size_t size, double error_bound, compress_strategy_e compress_strategy)
{
    QDigest *sketch = check_calloc(1, sizeof(QDigest));
    sketch->size = size;
    sketch->error_bound = error_bound;
    sketch->compress_strategy = compress_strategy;
    sketch->saved_weight = 0;
    sketch->values = check_calloc(size, sizeof(Instance));
    return sketch;
}

void free_sketch(QDigest *sketch)
{
    check_free(sketch->values);
    check_free(sketch);
}

void update(QDigest *sketch, Instance value)
{
    uint64_t previous_weight = sketch->saved_weight;
    uint64_t previous_capacity = capacity(sketch);

    // do update

    // Check compress strategy
    if (previous_capacity < capacity(sketch) && sketch->compress_strategy == AFTER_CAPACITY_INCREASES)
    {
        compress(sketch);
    }

    if (previous_weight > weight(sketch) / 2.0)
    {
        if (sketch->compress_strategy == WHEN_WEIGHT_DOUBLES)
        {
            compress(sketch);
        }
        sketch->saved_weight = previous_weight;
    }

    if (sketch->compress_strategy == AFTER_EVERY_UPDATE)
        compress(sketch);
}

uint32_t rank(QDigest *sketch)
{
    return 0;
}

Instance quantile(QDigest *sketch, double q)
{
    Instance x = {.val = 0, .weight = 0};
    return x;
}

uint64_t weight(QDigest *sketch)
{
    // TODO(fssn): binary_tree.fold(|acc, x| acc + x);
    uint64_t sum = 0;
    for (int i = 0; i < sketch->size; i++)
    {
        sum += sketch->values[i].weight;
    }
    return sum;
}

uint64_t capacity(QDigest *sketch)
{
    return sketch->error_bound * weight(sketch) / (log(sketch->size));
}

void compress(QDigest *sketch)
{
}