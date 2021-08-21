#include "../lib/csv_parser.h"
#include "../lib/opt.h"
#include "../lib/timer.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

typedef enum
{
    RANK,
    QUERY
} operation_e;
void read_query_args_from_file(FILE *in, operation_e operation, uint64_t **fields, double **fields_double, size_t *fields_count);

int64_t rank(std::vector<int64_t> &v, int64_t x)
{
    for (int i = 0; i < v.size() - 1; i++)
    {
        if (v[i] <= x && v[i + 1] > x)
            return i;
    }

    return v.size();
}

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    char pattern[] = "val:eps:univ:help:rank:quant:in:verbose:";
    int status = opt_init(&opt, pattern, argc, argv);
    if (status != OPT_SUCCESS)
    {
        return -1;
    }

    int id_field_no = 0;
    double error_bound = 0.0f;
    uint32_t universe_size = 0;
    operation_e operation = RANK;
    FILE *in = NULL;
    uint64_t *fields;
    double *fields_double;
    size_t fields_count = 0;
    bool verbose = false;
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
        else if (strcmp(opt_key, "verbose") == 0)
        {
            verbose = true;
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
        return -1;
    }

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
        return -1;
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    ssize_t lines_read;
    lines_read = getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    std::vector<int64_t> true_values;
    {
        Timer timer{"Input + Updates"};
        while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
        {
            // skip blank lines
            if (lines_read == 1)
                continue;

            read_from_line(parser, buffer);

            int64_t value = atoi(parser->line[id_field_no]);
            if (universe_size != 0 && (value < 0 || value > universe_size))
                continue;

            true_values.push_back(value);
        }
    }

    if (in)
        read_query_args_from_file(in, operation, &fields, &fields_double, &fields_count);

    std::sort(true_values.begin(), true_values.end());
    {
        Timer timer{"Query Operations"};
        for (int i = 0; i < fields_count; i++)
        {
            if (operation == RANK)
            {
                auto x = static_cast<int64_t>(fields[i]);
                printf("rank(%lu) = %ld\n", fields[i], rank(true_values, x));
            }
            else
            {
                size_t idx = ceil(fields_double[i] * (true_values.size() - 1));
                printf("query(%lf) = %lu\n", fields_double[i], true_values[idx]);
            }
        }
    }

    return 0;
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
