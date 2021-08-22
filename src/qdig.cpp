#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "../lib/csv_parser.h"
#include "../lib/memory.h"
#include "../lib/opt.h"
#include "../lib/timer.hpp"
#include "../lib/tree.h"

typedef enum
{
    AFTER_EVERY_UPDATE,
    WHEN_WEIGHT_DOUBLES,
    AFTER_CAPACITY_INCREASES
} compress_strategy_e;

typedef enum
{
    RANK,
    QUERY
} operation_e;

typedef struct
{
    uint32_t log_univ;
    uint32_t univ;
    uint32_t size;
    uint64_t saved_weight;
    double error_bound;
    Node *root;
    compress_strategy_e compress_strategy;
} QDigest;
QDigest *init(size_t, double, compress_strategy_e);
void free_sketch(QDigest *);

void update(QDigest *, Instance);
uint32_t rank(QDigest *, int);
size_t quantile(QDigest *, double);
uint64_t weight(Node *);
uint64_t capacity(QDigest *);
void compress(QDigest *);
Node *compress_helper(QDigest *, Node *, int, int, int *);
Instance search_update(Node *, int, int, int, QDigest *, int);
Instance search(Node *, int, int, int, QDigest *, int, int);
void memory(QDigest *);

void print_help();
void read_query_args_from_file(FILE *in, operation_e operation, uint64_t **fields, double **fields_double, size_t *fields_count);
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

int min(int x, int y)
{
    return y ^ ((x ^ y) & -(x < y));
}

uint64_t sum = 0;
uint64_t mem_usage = 0;
uint64_t compress_count = 0;
int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    int status = opt_init(&opt, "val:eps:univ:help:rank:quant:in:compress", argc, argv);
    if (status != OPT_SUCCESS)
    {
        print_error_help();
        return -1;
    }

    int id_field_no = 0;
    double error_bound = 0.0f;
    uint32_t universe_size = 0;
    operation_e operation = RANK;
    compress_strategy_e strategy = AFTER_CAPACITY_INCREASES;
    FILE *in = NULL;
    uint64_t *fields;
    double *fields_double;
    size_t fields_count = 0;
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
            operation = RANK;
            fields_count = opt->current_args_count;
            fields = (uint64_t *)check_malloc(sizeof(uint64_t) * fields_count);
            for (int i = 0; i < fields_count; i++)
            {
                fields[i] = atoi(opt_args[i]);
            }
        }
        else if (strcmp(opt_key, "quant") == 0)
        {
            operation = QUERY;
            fields_count = opt->current_args_count;
            fields_double = (double *)check_malloc(sizeof(double) * fields_count);
            for (int i = 0; i < fields_count; i++)
            {
                fields_double[i] = atof(opt_args[i]);
            }
        }
        else if (strcmp(opt_key, "in") == 0)
        {
            in = fopen(opt_args[0], "r");
        }
        else if (strcmp(opt_key, "strategy") == 0)
        {
            if (strcmp(opt_args[0], "update") == 0)
            {
                strategy = AFTER_EVERY_UPDATE;
            }
            else if (strcmp(opt_args[0], "weight") == 0)
            {
                strategy = WHEN_WEIGHT_DOUBLES;
            }
            else
            {
                strategy = AFTER_CAPACITY_INCREASES;
            }
        }
    }

    opt_free(&opt);

    int filename_idx = 0;
    for (filename_idx = 0; filename_idx < argc; filename_idx++)
    {
        char *word = strdup(argv[filename_idx]);
        int len = strlen(word);
        if (len >= 4 && strcmp((char *)&word[len - 4], ".csv") == 0)
        {
            check_free(word);
            break;
        }
        check_free(word);
    }
    const char *filename = strdup(argv[filename_idx]);

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

    QDigest *sketch = init(universe_size, error_bound, strategy);

    while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
    {
        // skip blank lines
        if (lines_read == 1)
            continue;

        read_from_line(parser, buffer);

        int32_t val = atoi(parser->line[id_field_no]);

        if (val < 0 || val >= universe_size)
            continue;

        Instance instance = {.val = val, .weight = 1};
        update(sketch, instance);
    }

    if (in)
        read_query_args_from_file(in, operation, &fields, &fields_double, &fields_count);

    {
        Timer timer{"Query Operations"};
        for (int i = 0; i < fields_count; i++)
        {
            if (operation == RANK)
            {
                printf("rank(%lu) = %u\n", fields[i], rank(sketch, fields[i]));
            }
            else
            {
                auto x = quantile(sketch, fields_double[i]);
                printf("rank(%d) = %u\n", x, rank(sketch, x));
                printf("quantile(%.2lf) = %u\n", fields_double[i], x);
            }
        }
    }

    return 0;
}

// MARK - QDigest functions
QDigest *init(size_t size, double error_bound, compress_strategy_e compress_strategy)
{
    QDigest *sketch = (QDigest *)check_malloc(sizeof(QDigest));
    sketch->root = (Node *)check_calloc(1, sizeof(Node));
    sketch->log_univ = ceil(log2(size));
    sketch->size = size;
    sketch->univ = pow(2, sketch->log_univ);
    sketch->error_bound = error_bound;
    sketch->saved_weight = 0;
    sketch->compress_strategy = compress_strategy;
    return sketch;
}

void execute_free(Node *v)
{
    check_free(v);
}
void free_sketch(QDigest *sketch)
{
    inorder(sketch->root, execute_free);
    check_free(sketch);
}

void update(QDigest *sketch, Instance value)
{
    uint64_t previous_weight = sketch->saved_weight;
    uint64_t previous_capacity = capacity(sketch);

    // do update
    sketch->saved_weight += value.weight;
    auto cap = capacity(sketch);
    Node *v = sketch->root;
    auto upper_bound = sketch->univ - 1;
    v->instance.val = upper_bound;
    auto acc = 0;
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(sketch->univ);
    while (value.weight != 0 && (right - left) > 1)
    {
        auto avail = cap - v->instance.weight;
        auto added = min(value.weight, avail);
        v->instance.weight += added;
        value.weight -= added;
        auto middle = (left + right) / 2;
        Node *u = NULL;
        if (value.val < middle)
        {
            if (v->left == NULL)
                v->left = (Node *)check_calloc(1, sizeof(Node));
            u = v->left;
            upper_bound /= 2;
            upper_bound += (acc * 2);
            u->instance.val = upper_bound;
            right = middle;
        }
        else
        {
            if (v->right == NULL)
                v->right = (Node *)check_calloc(1, sizeof(Node));
            u = v->right;
            u->instance.val = upper_bound;
            acc++;
            left = middle;
        }
        v = u;
    }
    v->instance.weight += value.weight;

    // Check compress strategy
    if (previous_capacity < capacity(sketch) && sketch->compress_strategy == AFTER_CAPACITY_INCREASES)
    {
        printf("Memory before compress: ");
        memory(sketch);
        compress(sketch);
        compress_count++;
        printf("Memory after compress: ");
        memory(sketch);
        printf("\n");
    }

    if (previous_weight > weight(sketch->root) / 2.0)
    {
        if (sketch->compress_strategy == WHEN_WEIGHT_DOUBLES)
        {
            printf("Memory before compress: ");
            memory(sketch);
            compress(sketch);
            compress_count++;
            printf("Memory after compress: ");
            memory(sketch);
            printf("\n");
        }
        sketch->saved_weight = previous_weight;
    }

    if (sketch->compress_strategy == AFTER_EVERY_UPDATE)
    {
        printf("Memory before compress: ");
        memory(sketch);
        compress(sketch);
        compress_count++;
        printf("Memory after compress: ");
        memory(sketch);
        printf("\n");
    }
}

uint32_t rank(QDigest *sketch, int x)
{
    Node *v = sketch->root;
    auto left = 0;
    auto right = static_cast<int32_t>(sketch->univ);
    auto r = 0;
    while (v != NULL && (right - left) > 1)
    {
        auto middle = (left + right) / 2;
        if (x < middle)
        {
            right = middle;
            v = v->left;
        }
        else
        {
            r += weight(v->left);
            v = v->right;
            left = middle;
        }
    }
    return r;
}

Node *qNode;
size_t cur_rank;
size_t request_rank;
void postorder_execute(Node *v)
{
    if (qNode != NULL)
        return;
    if (v == NULL)
        return;

    cur_rank += v->instance.weight;
    if (cur_rank >= request_rank)
        qNode = v;
}
size_t quantile(QDigest *sketch, double q)
{
    cur_rank = 0;
    request_rank = q * sketch->univ;
    qNode = NULL;
    postorder(sketch->root, postorder_execute);
    if (qNode == NULL || qNode->instance.val > sketch->size - 1)
        return sketch->size - 1;
    return qNode->instance.val;
}

void execute(Node *v)
{
    sum += v->instance.weight;
}

uint64_t weight(Node *root)
{
    sum = 0;
    inorder(root, execute);
    return sum;
}

uint64_t capacity(QDigest *sketch)
{
    return sketch->error_bound * sketch->saved_weight / sketch->log_univ;
}

void execute_zero_weight(Node *root)
{
    root->instance.weight = 0;
}

void check_memory(Node *v)
{
    if (v->instance.weight != 0)
        mem_usage += sizeof(v);
}
void memory(QDigest *sketch)
{
    mem_usage = 0;
    inorder(sketch->root, check_memory);
    mem_usage += sizeof(sketch);
    printf("QDigest mem usage = %lubytes (%lfMB)\n", mem_usage, mem_usage / 1024.0);
}

void compress(QDigest *sketch)
{
    int _x;
    compress_helper(sketch, sketch->root, capacity(sketch), 0, &_x);
}

Node *compress_helper(QDigest *sketch, Node *root, int cap, int avail_up, int *move_up)
{
    auto x = 0;
    auto avail_here = cap - root->instance.weight;
    auto move_up_from_chd = 0;
    Node *u = compress_helper(sketch, root->left, cap, avail_up - avail_here, &move_up_from_chd);
    check_free(root->left);
    root->left = u;
    auto put_here = min(avail_here, move_up_from_chd);
    root->instance.weight += put_here;
    x += (move_up_from_chd - put_here);

    avail_here = cap - root->instance.weight;
    u = compress_helper(sketch, root->right, cap, avail_up - avail_here, &move_up_from_chd);
    check_free(root->right);
    root->right = u;
    put_here = min(avail_here, move_up_from_chd);
    root->instance.weight += put_here;
    x += (move_up_from_chd - put_here);

    auto move_up_from_here = min(avail_up, root->instance.weight);
    x += move_up_from_here;
    root->instance.weight += move_up_from_here;

    *move_up = x;
    if (root->instance.weight == 0 && root->left == NULL && root->right == NULL)
    {
        return NULL;
    }
    else
    {
        return root;
    }
}

Instance search_update(Node *root, int key, int w, int acc, QDigest *sketch, int size)
{
    return search(root, key, w, acc, sketch, size, 1);
}

Instance search(Node *root, int key, int w, int acc, QDigest *sketch, int size, int is_update)
{
    auto c = capacity(sketch);
    if (root->instance.weight < c && is_update)
    {
        float d = min(c - root->instance.weight, w);
        root->instance.weight += d;
        w -= d;
    }

    if (w == 0)
    {
        return root->instance;
    }

    if (key == root->instance.val)
    {
        return root->instance;
    }
    else if (key >= size / 2 + acc)
    {
        return search(root->right, key, w, acc + size / 2, sketch, size / 2, is_update);
    }
    else
    {
        return search(root->left, key, w, acc, sketch, size / 2, is_update);
    }
}

void read_query_args_from_file(FILE *in, operation_e operation, uint64_t **fields, double **fields_double, size_t *fields_count)
{
    size_t current_size = 0;
    size_t capacity = 64;
    char *buffer = NULL;
    size_t buffer_size = 0;
    ssize_t lines_read = 0;

    *fields_count = 0;

    if (operation == RANK)
        *fields = (uint64_t *)check_malloc(sizeof(uint64_t) * capacity);
    else
        *fields_double = (double *)check_malloc(sizeof(double) * capacity);
    while ((lines_read = getline(&buffer, &buffer_size, in)) != -1)
    {
        if (current_size == capacity)
        {
            capacity *= 2;
            if (operation == RANK)
                *fields = (uint64_t *)check_realloc(fields, capacity);
            else
                *fields_double = (double *)check_realloc(fields_double, capacity);
        }

        if (operation == RANK)
            (*fields)[current_size] = atoi(buffer);
        else
            (*fields_double)[current_size] = atof(buffer);

        current_size++;
    }
    *fields_count = current_size;
}