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
    int *C;
    uint32_t k;
    uint32_t logk;
    double alpha;
    uint32_t seed;
} HLL;
HLL *init(uint32_t, uint32_t, uint32_t);
void free_sketch(HLL *);
HLL *update(HLL *, char *);
uint32_t query(HLL *);
int64_t leading_zeroes(int64_t, int64_t);

uint64_t max(uint64_t lhs, uint64_t rhs)
{
    return lhs > rhs ? lhs : rhs;
}

void print_help();
void print_error_help()
{
    printf("Algo de inesperado aconteceu ao programa, olhe o --help por precaucao.\n\n");
    print_help();
}

void print_help()
{
    printf("$ hll [options] inputfile.csv\n\n");
    printf("--target field_no:\n");
    printf("Especifica o numero da coluna alvo (t≥0) do arquivo \nde entrada cuja quantidade de valores ́unicos devem estimada.\n\n");
    printf("--eps error_bound:\n");
    printf("Especifica o valor do parametro e do estimador \n(limite do erro relativo desejado).\n\n");
    printf("--delta error_probability:\n");
    printf("Especifica o valor do parametro δ do estimador \n(limite para a probabilidade que o erro relativo da \nestimacao seja superior ao limite especificado).\n\n");
    printf("--help : Roda este comando.\n\n");
}

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    int status = opt_init(&opt, "target:eps:delta:help:", argc, argv);
    if (status != OPT_SUCCESS)
    {
        print_error_help();
    }

    int field_no = 0;
    double error_bound = 0.0f;
    double error_probability = 0.0f;
    while (get_opt(opt, &opt_key, &opt_args) == OPT_SUCCESS)
    {
        if (strcmp(opt_key, "target") == 0)
        {
            field_no = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "eps") == 0)
        {
            error_bound = atof(opt_args[0]);
        }
        else if (strcmp(opt_key, "delta") == 0)
        {
            error_probability = atof(opt_args[0]);
        }
        else if (strcmp(opt_key, "help") == 0)
        {
            print_help();
            return 0;
        }
    }
    opt_free(&opt);

    const char *filename = strdup(argv[argc - 1]);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        print_error_help();
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

    // Define N
    uint32_t n;
    if (error_probability == 0)
    {
        n = 1;
    }
    else
    {
        n = ceil(log(1.0 / error_probability));
    }

    uint32_t logk = ceil(log2(pow(1.04 / error_bound, 2.0)));
    uint32_t k = ceil(pow(2.0, logk));

    srand(time(0));
    uint32_t seed = rand() - n;

    HLL **sketches = (HLL **)check_malloc(sizeof(HLL *) * n);
    for (int i = 0; i < n; i++)
    {
        sketches[i] = init(k, logk, seed + i);
    }

    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        char *value = parser->line[field_no];

        for (int i = 0; i < n; i++)
        {
            sketches[i] = update(sketches[i], value);
        }
    }

    uint64_t *p = (uint64_t *)check_malloc(sizeof(uint64_t) * n);
    for (int i = 0; i < n; i++)
    {
        p[i] = query(sketches[i]);
    }

    uint64_t fst = kthSmallest(p, 0, n - 1, (n / 2) + 1);
    uint64_t snd = fst;

    if (n % 2 == 0)
    {
        snd = kthSmallest(p, 0, n - 1, (n / 2));
    }

    int64_t median = (fst + snd) / 2;
    printf("%lu\n", median);

    return 0;
}

// MARK - HLL functions
HLL *init(uint32_t k, uint32_t logk, uint32_t h)
{
    HLL *sketch = check_malloc(sizeof(HLL));
    sketch->C = check_calloc(k, sizeof(int));
    sketch->seed = h;
    sketch->k = k;
    sketch->logk = logk;
    sketch->alpha = 0.7213 / (1.0 + (1.079 / k));

    return sketch;
}

void free_sketch(HLL *sketch)
{
    check_free(sketch->C);
    check_free(sketch);
}

HLL *update(HLL *sketch, char *value)
{
    size_t value_len = strlen(value);
    uint32_t h_x = murmurhash(value, value_len, sketch->seed);
    uint32_t j = h_x % sketch->k;
    uint32_t g_x = h_x >> sketch->logk;

    sketch->C[j] = max(sketch->C[j], leading_zeroes(g_x, 64 - sketch->logk));

    return sketch;
}

uint32_t query(HLL *sketch)
{
    double x = 0;
    for (int j = 0; j < sketch->k; j++)
    {
        x += (1.0 / (1 << sketch->C[j]));
    }

    uint32_t first_estimate = ceil((sketch->alpha * sketch->k * sketch->k) / x);
    uint32_t estimate = first_estimate;

    if (first_estimate < ((5.0 / 2.0) * sketch->k))
    {
        int V = 0;
        for (int j = 0; j < sketch->k; j++)
        {
            if (sketch->C[j] == 0)
                V++;
        }
        if (V != 0)
        {
            estimate = sketch->k * log(sketch->k / (double)V);
        }
    }
    else if (first_estimate > ((1.0 / 3.0) * ((int64_t)1 << 32)))
    {
        estimate = -((int64_t)1 << 32) * log(1 - (double)first_estimate / ((int64_t)1 << 32));
    }
    return estimate;
}

int64_t leading_zeroes(int64_t value, int64_t nbits)
{
    int64_t res = 0;
    uint64_t mask = (uint64_t)1 << (nbits - 1);
    while ((mask & value) == 0)
    {
        value = (value << 1);
        res++;

        if (value == 0)
            break;
    }

    return res + 1;
}
