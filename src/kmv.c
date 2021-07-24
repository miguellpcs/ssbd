#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/array.h"
#include "../lib/csv_parser.h"
#include "../lib/instance.h"
#include "../lib/heap.h"
#include "../lib/memory.h"
#include "../lib/opt.h"
#include "../lib/set.h"

int uniform_distribution(int, int);
double kthSmallest(double *arr, int l, int r, int k);
unsigned int murmurhash(unsigned int *key, unsigned int len, unsigned int seed);

typedef struct
{
    Heap *heap;
    Set *set;
    double r;
    unsigned int k;
} KMV;
KMV *init(unsigned int, unsigned int);
void free_sketch(KMV *);
KMV *update(KMV *, Instance);
double query(KMV *);

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
    unsigned int k = 20 / pow(error_bound, 2.0);
    unsigned int n = 4 * log(1.0 / error_probability);

    KMV **sketches = (KMV **)check_malloc(sizeof(KMV *) * n);
    for (int i = 0; i < n; i++)
    {
        sketches[i] = init(k, 0x85ebca6b);
    }

    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        char *eptr;
        unsigned int value = strtoul(parser->line[field_no], &eptr, 10);
        for (int i = 0; i < n; i++)
        {
            srand(i);
            unsigned int hash_value = murmurhash(&value, sizeof(unsigned int), rand());
            Instance instance = {.val = value, .weight = hash_value};
            sketches[i] = update(sketches[i], instance);
        }
    }

    double *p = (double *)check_malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++)
    {
        p[i] = query(sketches[i]);
    }

    double fst = kthSmallest(p, 0, n - 1, (n / 2) + 1);
    double snd = fst;

    if (n % 2 == 0)
    {
        snd = kthSmallest(p, 0, n - 1, (n / 2));
    }

    double median = (fst + snd) / 2.0;
    printf("Estimative: %u\n", (unsigned int)median);

    for (int i = 0; i < n; i++)
    {
        free_sketch(sketches[i]);
    }
    check_free(sketches);

    return 0;
}

int partition(double *arr, int l, int r);

double kthSmallest(double *arr, int l, int r, int k)
{
    if (k > 0 && k <= r - l + 1)
    {
        int pos = partition(arr, l, r);

        if (pos - l == k - 1)
            return arr[pos];
        if (pos - l > k - 1)
            return kthSmallest(arr, l, pos - 1, k);

        return kthSmallest(arr, pos + 1, r, k - pos + l - 1);
    }

    return NAN;
}

void swap(double *a, double *b)
{
    double temp = *a;
    *a = *b;
    *b = temp;
}

int partition(double *arr, int l, int r)
{
    int i = l;
    for (int j = l; j <= r - 1; j++)
    {
        if (arr[j] <= arr[r])
        {
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    swap(&arr[i], &arr[r]);
    return i;
}

// MARK - KMV functions
KMV *init(unsigned int size, unsigned int r)
{
    KMV *sketch = check_malloc(sizeof(KMV));
    sketch->set = set_init(size);
    sketch->r = (double)r;
    sketch->k = size;

    sketch->heap = check_malloc(sizeof(Heap));
    sketch->heap->instances = check_malloc((size + 1) * sizeof(Instance));
    sketch->heap->count = 0;

    return sketch;
}

void free_sketch(KMV *sketch)
{
    check_free(sketch->heap->instances);
    check_free(sketch->heap);
    free_set(sketch->set);
    check_free(sketch);
}

unsigned int murmurhash(unsigned int *key, unsigned int len, unsigned int seed)
{
    unsigned int c1 = 0xcc9e2d51;
    unsigned int c2 = 0x1b873593;
    unsigned int r1 = 15;
    unsigned int r2 = 13;
    unsigned int m = 5;
    unsigned int n = 0xe6546b64;
    unsigned int h = 0;
    unsigned int k = 0;
    unsigned char *d = (unsigned char *)key; // 32 bit extract from `key'
    const unsigned int *chunks = NULL;
    const unsigned char *tail = NULL; // tail - last 8 bytes
    int i = 0;
    int l = len / 4; // chunk length

    h = seed;

    chunks = (const unsigned int *)(d + l * 4); // body
    tail = (const unsigned char *)(d + l * 4);  // last 8 byte chunk of `key'

    // for each 4 byte chunk of `key'
    for (i = -l; i != 0; ++i)
    {
        // next 4 byte chunk of `key'
        k = chunks[i];

        // encode next 4 byte chunk of `key'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    // remainder
    switch (len & 3)
    { // `len % 4'
    case 3:
        k ^= (tail[2] << 16);
    case 2:
        k ^= (tail[1] << 8);

    case 1:
        k ^= tail[0];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        h ^= k;
    }

    h ^= len;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}

int is_present(KMV *sketch, Instance instance)
{
    return set_is_present(sketch->set, instance.val);
}

KMV *update(KMV *sketch, Instance x)
{
    if (!is_present(sketch, x))
    {
        insert_max_heap(sketch->heap, &x);
        set_insert(sketch->set, x.val);
        if (sketch->heap->count > sketch->k)
        {
            set_del(sketch->set, sketch->heap->instances[0].val);
            pop_max(sketch->heap);
        }
    }

    return sketch;
}

double query(KMV *sketch)
{
    return (sketch->k - 1) * (sketch->r / sketch->heap->instances[0].weight);
}
