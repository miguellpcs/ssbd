#include <stdio.h>
#include <string.h>

#include "../lib/array.h"
#include "../lib/csv_parser.h"
#include "../lib/instance.h"
#include "../lib/heap.h"
#include "../lib/opt.h"

int uniform_distribution(int, int);

typedef struct ws
{
    Heap *high;
    Heap *low;
    float tal;
    int count;
    int limit;
    int len;
} WS;
WS *init(int);
WS *update(WS *, Instance);
void query(WS *);

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = 0xdeadbeef;
    const char **opt_args = 0xdeadbeef;

    int status = opt_init(&opt, "id:weight:size:filter:", argc, argv);
    if (status != OPT_SUCCESS)
    {
    }

    int id_field_no = 0;
    int weight_field_no = 0;
    int size = 0;
    int field_no = 0;
    const char *field_value = NULL;
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

    const char *filename = strdup(argv[argc - 1]);
    FILE *file = fopen(filename, "r");

    csv_parser_t *parser = NULL;
    csv_parser_init(&parser);

    char *buffer;
    size_t buffer_size = 2048;

    buffer = (char *)malloc(buffer_size * sizeof(char));
    getline(&buffer, &buffer_size, file);

    read_from_line(parser, buffer);

    int random_sample_size = 10;

    Array *tst = create_sample(size);

    printf("Stream:\n");
    print_sample(tst);
    printf("Random Sample size: %d \n", random_sample_size);

    WS *sketch = init(random_sample_size);

    for (int i = 0; i < size; i++)
    {
        sketch = update(sketch, tst->data[i]);
        getline(&buffer, &buffer_size, file);
        read_from_line(parser, buffer);
    }
    printf("Random Sample:\n");
    query(sketch);

    return 0;
}

// MARK - Weighted Sampling functions
WS *init(int size)
{
    WS *sketch = malloc(sizeof(WS));
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

WS *update(WS *sketch, Instance x)
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

void query(WS *sketch)
{
    for (int i = 0; i < sketch->len; i++)
    {
        printf("val = %d, weight = %d \n ", sketch->high->instances[i].val, sketch->high->instances[i].weight);
    }
    printf("\n");
}
