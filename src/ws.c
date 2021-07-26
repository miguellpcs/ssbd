#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../lib/array.h"
#include "../lib/csv_parser.h"
#include "../lib/instance.h"
#include "../lib/memory.h"
#include "../lib/heap.h"
#include "../lib/opt.h"

int uniform_distribution(int, int);

typedef struct ws
{
    Heap *L;
    Heap *T;
    float tal;
    uint32_t count;
    uint32_t size;
    uint32_t len;
} WS;
WS *init(int);
void free_sketch(WS *);
WS *update(WS *, Instance *);
uint64_t query(WS *, char *);

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    int status = opt_init(&opt, "id:weight:size:filter:", argc, argv);
    if (status != OPT_SUCCESS)
    {
    }

    int id_field_no = 0;
    int weight_field_no = 0;
    int size = 0;
    int field_no = 0;
    char *field_value = NULL;
    while (get_opt(opt, &opt_key, &opt_args) == OPT_SUCCESS)
    {
        if (strcmp(opt_key, "id") == 0)
        {
            id_field_no = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "weight") == 0)
        {
            weight_field_no = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "size") == 0)
        {
            size = atoi(opt_args[0]);
        }
        else if (strcmp(opt_key, "filter") == 0)
        {
            field_no = atoi(opt_args[0]);
            field_value = strdup(opt_args[1]);
        }
    }
    opt_free(&opt);

    const char *filename = argv[argc - 1];
    FILE *file = fopen(filename, "r");

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    WS *sketch = init(size);

    ssize_t lines_read;
    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        Instance *instance = check_calloc(1, sizeof(Instance));
        instance->val = strdup(parser->line[id_field_no]);
        instance->weight = atoi(parser->line[weight_field_no]);
        instance->extra = strdup(parser->line[field_no]);

        sketch = update(sketch, instance);
    }

    uint64_t total_weight = query(sketch, field_value);
    printf("%lu\n", total_weight);

    check_free(field_value);
    csv_parser_free(&parser);
    free_sketch(sketch);
    check_free(buffer);
    return 0;
}

// MARK - Weighted Sampling functions
WS *init(int size)
{
    WS *sketch = check_malloc(sizeof(WS));
    sketch->count = 0;
    sketch->len = 0;
    sketch->tal = 0;
    sketch->size = size;

    sketch->L = check_malloc(sizeof(Heap));
    sketch->L->instances = check_malloc((size + 1) * sizeof(Instance));
    sketch->L->count = 0;

    sketch->T = check_malloc(sizeof(Heap));
    sketch->T->instances = NULL;
    sketch->T->count = 0;

    return sketch;
}

void free_sketch(WS *sketch)
{
    check_free(sketch->L->instances);
    check_free(sketch->L);
    check_free(sketch->T->instances);
    check_free(sketch->T);
    check_free(sketch);
}

WS *update(WS *sketch, Instance *y)
{
    Heap *X = check_malloc(sizeof(Heap));
    X->count = 0;
    X->instances = check_malloc(sizeof(Instance) * (sketch->L->count + 1));
    unsigned int wy = y->weight;
    unsigned int W = sketch->tal * sketch->T->count;
    if (wy > sketch->tal)
    {
        insert_min_heap(sketch->L, y);
        sketch->count++;
    }
    else
    {
        insert_min_heap(X, y);
        W += wy;
    }

    while (sketch->L->count != 0 && W >= (sketch->size - sketch->L->count) * sketch->L->instances[0].weight)
    {
        Instance h = sketch->L->instances[0];
        W += h.weight;
        pop_min(sketch->L);
        insert_min_heap(X, &h);
        X->count++;
    }

    sketch->tal = W / (sketch->size - sketch->L->count);
    float r = uniform_distribution_real(0, 1);
    int i = 0;
    while (i < X->count && r >= 0)
    {
        r -= (1 - X->instances[i].weight / sketch->tal);
        i++;
    }

    if (r < 0)
    {
        remove_at_min(X, i - 1);
    }
    else
    {
        int random = uniform_distribution(0, sketch->T->count - 1);
        remove_at_min(sketch->T, random);
    }

    int64_t new_size = sketch->T->count + X->count;
    if (new_size > 0)
    {
        sketch->T->instances = (Instance *)check_realloc(sketch->T->instances, sizeof(Instance) * new_size);
        for (i = 0; i < X->count; i++)
        {
            insert_min_heap(sketch->T, &X->instances[i]);
        }
    }
    check_free(X);
    X = NULL;

    return sketch;
}

uint64_t query(WS *sketch, char *filter)
{
    uint64_t sum = 0;
    for (int i = 0; i < sketch->L->count; i++)
    {
        if (strcmp((char *)sketch->L->instances[i].extra, filter) == 0)
        {
            sum += sketch->L->instances[i].weight;
        }
    }
    return sum;
}
