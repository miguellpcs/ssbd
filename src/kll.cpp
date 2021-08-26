#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <vector>
#include <functional>

#include "../lib/csv_parser.h"
#include "../lib/memory.h"
#include "../lib/opt.h"
#include "../lib/timer.hpp"

uint64_t compress_count;
uint64_t mem_saved;

struct KLL
{
    std::vector<std::vector<uint64_t>> values;
    double eps;
    double c;
    size_t h;
    uint64_t size;

    KLL(double eps) : eps(eps), c(0.5)
    {
        size = ceil(sqrt(log(1 / this->eps)) / this->eps);
        values.reserve(size);
    }

    void update(int64_t x)
    {
        values[0].push_back(x);
        compress();
    }

    double rank(const int64_t x)
    {
        auto r = 0;
        for (int level = 0; level < this->values.size(); level++)
        {
            // TODO(fssn): binary search
            for (auto value : this->values[level])
            {
                if (value < x)
                {
                    r += pow(2, level);
                }
                else
                {
                    break;
                }
            }
        }
        return r;
    }

    uint64_t query(double q) const
    {
    }

    void print() const
    {
    }

    static KLL sketch(KLL &s1, KLL &s2)
    {
        KLL s{s1.eps};
        size_t size = std::max(s1.h, s2.h);
        for (int level = 0; level < size; level++)
        {
            s.values[level].insert(s.values[level].end(), s1.values[level].begin(), s1.values[level].end());
            s.values[level].insert(s.values[level].end(), s2.values[level].begin(), s2.values[level].end());
        }
        s.compress();
        return s;
    }

private:
    void compress()
    {
        compress_count++;
        auto b_h = this->h;
        for (int level = 0; level < this->h; level++)
        {
            if (this->values[level].size() >= capacity(level))
            {
                std::sort(this->values[level].begin(), this->values[level].end());
                size_t i = 0;
                if (level == this->h - 1)
                    this->values.push_back({});
                for (int j = 0; j < this->values.size(); j++)
                {
                    if (i % 2 != 0)
                    {
                        if (j % 2 != 0)
                            this->values[level + 1].push_back(this->values[level][j]);
                    }
                    else
                    {
                        if (j % 2 == 0)
                            this->values[level + 1].push_back(this->values[level][j]);
                    }
                    this->values[level].clear();
                }
            }
        }
        if (this->values.size() > b_h)
            this->h++;
    }

    uint64_t capacity(size_t level)
    {
        return std::max(2, static_cast<int>(ceil(pow(this->c, this->h - level) * this->size)));
    }
};

void print_help();
void print_error_help()
{
    printf("Algo de inesperado aconteceu ao programa, olhe o --help por precaucao.\n\n");
    print_help();
}

void print_help()
{
    printf("$ kll build_options data_file.csv [rank | quant] [query_args | --in query_args_file]\n\n");
    printf("--val id_field_no: ");
    printf("Especifica numero da coluna do arquivo de entrada a ser usada para alimentar o sketch. Esta coluna deve conter valores numericos. Cada valor deve ser interpretado como tendo peso 1.\n\n");
    printf("--eps error_bound: ");
    printf("Especifica o valor do parametro e do sketch.\n\n");
    printf("--univ universe_size: ");
    printf("Especifica o valor do parametro U do sketch. Ou seja, consideramos os valores contidos no intervalo [U] = {0, ..., U - 1}. Os valores fora desse intervalo devem ser desconsiderados.\n\n");
    printf("--help : Roda este comando.\n\n");
}

typedef enum
{
    RANK,
    QUERY
} operation_e;
void read_query_args_from_file(FILE *in, operation_e operation, uint64_t **fields, double **fields_double, size_t *fields_count);

int main(int argc, const char *argv[])
{
    opt_t *opt;
    const char *opt_key = NULL;
    char **opt_args = NULL;

    int status = opt_init(&opt, "val:eps:univ:help:rank:quant:in:verbose:", argc, argv);
    if (status != OPT_SUCCESS)
    {
        print_error_help();
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
        print_error_help();
        return -1;
    }

    csv_parser_t *parser = NULL;
    status = csv_parser_init(&parser);
    if (status != PARSER_SUCCESS)
    {
        print_error_help();
        return -1;
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    ssize_t lines_read;
    lines_read = getline(&buffer, &buffer_size, file);

    // Read keys from csv
    read_from_line(parser, buffer);

    KLL sketch{error_bound};
    {
        Timer timer{"Input + Updates", verbose};
        while ((lines_read = getline(&buffer, &buffer_size, file)) != -1)
        {
            // skip blank lines
            if (lines_read == 1)
                continue;

            read_from_line(parser, buffer);

            int64_t value = atoi(parser->line[id_field_no]);
            if (universe_size != 0 && (value < 0 || value > universe_size))
                continue;

            sketch.update(value);
        }
    }

    if (in)
        read_query_args_from_file(in, operation, &fields, &fields_double, &fields_count);

    std::vector<int64_t> results;
    results.reserve(fields_count);
    {
        Timer timer{"Query Operations", verbose};
        for (int i = 0; i < fields_count; i++)
        {
            if (operation == RANK)
            {
                auto res = sketch.rank(fields[i]);
                if (!verbose)
                    printf("rank(%lu) = %.2lf\n", fields[i], res);
                else
                    results.push_back(res);
            }
            else
            {
                auto x = sketch.query(fields_double[i]);
                if (!verbose)
                    printf("quantile(%.2lf) = %lu\n", fields_double[i], x);
                else
                {
                    results.push_back(x);
                    results.push_back(sketch.rank(x));
                }
            }
        }
    }

    if (verbose)
    {
        auto mem_usage = sizeof(sketch) + (sizeof(std::vector<uint64_t>) * sketch.values.size());
        auto cmp_avg = mem_saved / compress_count;
        printf("%lu,%ld,%ld", mem_usage, compress_count, cmp_avg);
        for (auto res : results)
        {
            printf(",%ld", res);
        }
        printf("\n");
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
