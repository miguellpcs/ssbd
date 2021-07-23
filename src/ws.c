#include <stdio.h>
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
    Heap *high;
    Heap *C;
    float tal;
    int count;
    int limit;
    int len;
} WS;
WS *init(int);
void free_sketch(WS *);
WS *update(WS *, Instance *);
int query(WS *);

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

        Instance instance = {.val = atoi(parser->line[id_field_no]), .weight = atoi(parser->line[weight_field_no])};
        sketch = update(sketch, &instance);
    }

    int count = query(sketch);
    printf("%d: %s\n", field_no, field_value);
    printf("Element count: %d\n", count);

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
    sketch->limit = size;

    sketch->high = check_malloc(sizeof(Heap));
    sketch->high->instances = check_malloc((size + 1) * sizeof(Instance));
    sketch->high->count = 0;

    sketch->C = check_malloc(sizeof(Heap));
    sketch->C->instances = NULL;
    sketch->C->count = 0;

    return sketch;
}

void free_sketch(WS *sketch)
{
    check_free(sketch->high->instances);
    check_free(sketch->high);
    check_free(sketch->C->instances);
    check_free(sketch->C);
    check_free(sketch);
}

WS *update(WS *sketch, Instance *x)
{
    if (sketch->len < sketch->limit)
    { // Menos elem na amostra que o tamanho limite da amostra. Apenas insere sem nenhuma restrição
        insert_min_heap(sketch->high, x);
        sketch->count += 1;
        sketch->len += 1;
        return sketch;
    }

    int wL = sketch->tal * sketch->C->count; // Estima a massa dos pesos do conjunto de elementos leves

    Heap *new = malloc(sizeof(Heap)); // Cria uma heap auxiliar para termos um novo conjunto com s+1 elementos
    new->instances = (Instance *)check_malloc(sizeof(Instance));
    new->count = 0;

    if (x->weight < sketch->tal)
    {
        insert_min_heap(new, x);
        wL += x->weight;
    }
    else
    {
        insert_min_heap(sketch->high, x);
    }
    while (sketch->high->count != 0 && wL >= (sketch->limit - sketch->high->count) * sketch->high->instances[0].weight)
    {
        wL += sketch->high->instances[0].weight;
        new->instances = (Instance *)check_realloc(new->instances, sizeof(Instance) * (new->count + 1));
        insert_min_heap(new, &sketch->high->instances[0]);
        pop_min(sketch->high);
    }
    sketch->tal = ((float)wL) / ((sketch->limit) - (sketch->high->count));
    float sum_prob = heap_sum_prob(new, sketch->tal) + heap_sum_prob(sketch->C, sketch->tal);
    float p = uniform_distribution_real(0, sum_prob);

    int i = 0;
    for (; i < new->count &&p > 0; i++)
    {
        p = p - (1 - new->instances[i].weight / sketch->tal);
    }

    if (p < 0)
    {
        remove_at_min(new, i - 1);
    }
    else
    {
        remove_at_min(sketch->C, uniform_distribution(0, sketch->C->count - 1));
    }

    sketch->C->instances = (Instance *)check_realloc(sketch->C->instances, sizeof(Instance) * (sketch->C->count + new->count));
    for (i = 0; i < new->count; i++)
    {
        insert_min_heap(sketch->C, &new->instances[i]);
    }
    free(new->instances);
    free(new);

    return sketch;
}

int query(WS *sketch)
{
    for (int i = 0; i < sketch->len; i++)
    {
        printf("val = %d, weight = %d \n ", sketch->high->instances[i].val, sketch->high->instances[i].weight);
    }
    printf("\n");
    return sketch->count;
}
