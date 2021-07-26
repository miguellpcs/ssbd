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
    Heap *heap;
    Set *set;
    uint32_t r;
    uint64_t k;
} KMV;
KMV *init(uint64_t, uint32_t);
void free_sketch(KMV *);
KMV *update(KMV *, Instance *);
uint32_t query(KMV *);

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

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
    opt_free(&opt);

    const char *filename = strdup(argv[argc - 1]);
    FILE *file = fopen(filename, "r");

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    ssize_t lines_read;
    lines_read = getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    // Define K
    uint64_t k;
    if (error_bound == 0.0)
    {
        k = (uint64_t)1 << 22;
    }
    else
    {
        k = ceil(1 / (pow(error_bound, 2.0)));
    }
    uint32_t n;
    if (error_probability == 0)
    {
        n = 1;
    }
    else
    {
        n = 2 * log(1.0 / error_probability);
    }

    KMV **sketches = (KMV **)check_malloc(sizeof(KMV *) * n);
    for (int i = 0; i < n; i++)
    {
        sketches[i] = init(k, UINT32_MAX);
    }

    srand(time(0));
    uint32_t seed = rand() - n;

    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        char *value = parser->line[field_no];

        for (int i = 0; i < n; i++)
        {
            uint32_t hash_value = murmurhash(value, strlen(value), seed + i);
            Instance *instance = check_calloc(1, sizeof(Instance));
            instance->val = strdup(value);
            instance->weight = hash_value;
            instance->extra = NULL;
            sketches[i] = update(sketches[i], instance);
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

    for (int i = 0; i < n; i++)
    {
        free_sketch(sketches[i]);
    }
    check_free(sketches);

    return 0;
}

// MARK - KMV functions
KMV *init(uint64_t size, uint32_t r)
{
    KMV *sketch = check_malloc(sizeof(KMV));
    sketch->set = set_init(size);
    sketch->r = r;
    sketch->k = size;

    sketch->heap = check_malloc(sizeof(Heap));
    sketch->heap->instances = check_malloc((size + 1) * sizeof(Instance));
    sketch->heap->count = 0;

    return sketch;
}

void free_sketch(KMV *sketch)
{
    for (int i = 0; i < sketch->heap->count; i++)
    {
        check_free(sketch->heap->instances[i].val);
    }
    check_free(sketch->heap->instances);
    check_free(sketch->heap);
    free_set(sketch->set);
    check_free(sketch);
}

int is_present(KMV *sketch, Instance instance)
{
    return set_is_present(sketch->set, (const char *)instance.val);
}

KMV *update(KMV *sketch, Instance *x)
{
    if (!is_present(sketch, *x))
    {
        insert_max_heap(sketch->heap, x);
        set_insert(sketch->set, x->val);
        if (sketch->heap->count > sketch->k)
        {
            Instance *instance = pop_max(sketch->heap);
            set_del(sketch->set, instance->val);
            check_free(instance->val);
            check_free(instance);
        }
    }

    return sketch;
}

uint32_t query(KMV *sketch)
{
    if (sketch->heap->count < sketch->k)
        return sketch->heap->count;
    return sketch->k * (sketch->r / ((double)sketch->heap->instances[0].weight));
}
