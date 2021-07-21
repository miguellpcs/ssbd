#include <stdio.h>
#include <string.h>

#include "../lib/array.h"
#include "../lib/csv_parser.h"
#include "../lib/instance.h"
#include "../lib/heap.h"
#include "../lib/opt.h"

int uniform_distribution(int, int);

typedef struct
{
    Heap *high;
    Heap *low;
    float tal;
    int count;
    int limit;
    int len;
} KMV;
KMV *init(int);
KMV *update(KMV *, Instance);
int query(KMV *);

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    const char **opt_args = NULL;

    int status = opt_init(&opt, "target:eps:delta:", argc, argv);
    if (status != OPT_SUCCESS)
    {
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
    }

    const char *filename = strdup(argv[argc - 1]);
    FILE *file = fopen(filename, "r");

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
    }

    char *buffer;
    size_t buffer_size = 2048;

    buffer = (char *)malloc(buffer_size * sizeof(char));
    getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    // Define K
    int k = 0;
    KMV *sketch = init(k);

    ssize_t lines_read;
    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        getline(&buffer, &buffer_size, file);
        read_from_line(parser, buffer);

        Instance instance = {.val = atoi(parser->line[field_no]), .weight = 1};
        sketch = update(sketch, instance);
    }

    int count = query(sketch);
    printf("Element count: %d\n", count);

    return 0;
}

// MARK - KMV functions
KMV *init(int size)
{
    KMV *sketch = malloc(sizeof(KMV));
    sketch->count = 0;
    sketch->len = 0;
    sketch->tal = 0;
    sketch->limit = size;
    //sketch->data = malloc(size*sizeof(int));

    sketch->high = malloc(sizeof(Heap));
    sketch->high->instances = malloc(size * sizeof(Instance));
    sketch->high->count = 0;

    sketch->low = malloc(sizeof(Heap));
    sketch->low->instances = NULL;
    sketch->low->count = 0;

    return sketch;
}

KMV *update(KMV *sketch, Instance x)
{ // Update fors simple RS. TODO: add tal update
    if (sketch->len < sketch->limit)
    {
        insert_min_heap(sketch->high, &x);
        sketch->count += 1;
        sketch->len += 1;
    }

    else
    {
        sketch->count += 1;
        int i = uniform_distribution(0, sketch->count);
        if (i < sketch->limit)
        {
            pop_min(sketch->high);
            insert_min_heap(sketch->high, &x);
        }
    }

    return sketch;
}

int query(KMV *sketch)
{
    for (int i = 0; i < sketch->len; i++)
    {
        printf("val = %d, weight = %d \n ", sketch->high->instances[i].val, sketch->high->instances[i].weight);
    }
    printf("\n");
    return sketch->count;
}
