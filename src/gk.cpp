#include <cstdint>
#include <cmath>
#include <cstdio>
#include <vector>
#include <functional>

#include "../lib/csv_parser.h"
#include "../lib/memory.h"
#include "../lib/opt.h"
#include "../lib/timer.hpp"

struct GKNode
{
    int64_t x;
    uint64_t g;
    int64_t delta;

    GKNode(int64_t x, uint64_t g, int64_t delta) : x(x), g(g), delta(delta) {}
};

struct GK
{
    std::vector<GKNode> values;
    uint64_t N;
    double eps;

    GK(double eps) : eps(eps)
    {
        N = 0;
        values.emplace_back(GKNode{INT64_MAX, 1, 0});
    }

    void update(int64_t x)
    {
        this->N++;
        auto it = this->binary_search(x);
        assert(x < it->x);
        uint64_t two_eps_n = (uint64_t)ceil(2.0 * this->eps * this->N);
        if (it->x != INT64_MAX && (*it).g + (*it).delta + 1 < two_eps_n)
        {
            (*it).g++;
        }
        else
        {
            this->values.insert(it, GKNode(x, 1, (*it).g + (*it).delta - 1));
            this->compress();
        }
    }

    double rank(const int64_t y)
    {
        if (this->values.empty())
            return 0;

        // Find where y would fall in summary
        auto it = this->binary_search(y);

        // Calculate Egj, j=0..i
        double r = 0;
        for (auto tmp = this->values.begin(); tmp != it; tmp++)
        {
            r += tmp->g;
        }

        // sum(g[0..i]) - 1 + (g[i+1] + d[i+1])/2
        return r - 1.0 + (it->g + it->delta) / 2.0;
    }

    GKNode query(double q) const
    {
        // Multiply quantile per N to get approximated rank
        double r = q * this->N;

        // eN from r - eN <= rank(idx) < r + eN
        double eN = this->eps * this->N;

        // r - eN <= sum(g[0..i]) - 1 <= rank(x[i]) <= delta[i] + sum(g[0..i]) - 1 <= r + eN
        // eN <= r - (sum(g[0..i]) - 1) <= rank(x[i]) <= (delta[i] + sum(g[0..i] - 1)) - r <= eN
        double g_sum = 0;
        for (auto node : this->values)
        {
            g_sum += node.g;
            if (r - (g_sum - 1) <= eN && (node.delta + g_sum - 1) - r <= eN)
                return node;
        }

        // Something went wrong. We are guaranteed to find a xi;
        return this->values[r];
    }

    void print() const
    {
        int two_eps_n = (int)ceil(2.0 * this->eps * this->N);
        printf("N = %ld\t[2eN] = %d\t", this->N, two_eps_n);
        for (auto value : this->values)
        {
            if (value.x != INT64_MAX)
                printf("(%ld, %lu, %ld), ", value.x, value.g, value.delta);
            else
                printf("(inf, 1, 0)\n");
        }
    }

private:
    void compress()
    {
        size_t j = 0;
        while (j < this->values.size() - 2) // Ignore last element
        {
            uint64_t two_eps_n = (int)ceil(2.0 * this->eps * this->N);
            if (this->values[j].g + this->values[j + 1].g + this->values[j + 1].delta < two_eps_n)
            {
                this->values[j + 1].g += this->values[j].g;
                this->values.erase(this->values.begin() + j);
                return;
            }
            j++;
        }
    }

    std::vector<GKNode>::iterator binary_search(const int64_t x)
    {
        ssize_t left = 0;
        ssize_t right = this->values.size() - 1;

        while (left <= right)
        {
            // Avoid overflow
            size_t middle = left + (right - left) / 2;

            // Need to find this->values[middle].x <= x < this->values[middle+1].x
            if (this->values[middle].x <= x && middle + 1 <= this->values.size() - 1 && x < this->values[middle + 1].x)
                return this->values.begin() + middle + 1;
            else if (this->values[middle].x <= x)
                left = middle + 1;
            else
                right = middle - 1;
        }

        // Couldn't find any suitable spot, put it on 0 or at the end
        size_t idx = 0;
        if (!this->values.empty() && this->values[0].x < x)
            idx = this->values.size() - 1;
        return this->values.begin() + idx;
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
    printf("$ gk build_options data_file.csv [rank | quant] [query_args | --in query_args_file]\n\n");
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

    GK sketch{error_bound};
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

            sketch.update(value);

            if (verbose)
                sketch.print();
        }
    }

    if (in)
        read_query_args_from_file(in, operation, &fields, &fields_double, &fields_count);

    {
        Timer timer{"Query Operations"};
        for (int i = 0; i < fields_count; i++)
        {
            if (operation == RANK)
            {
                printf("rank(%lu) = %.2lf\n", fields[i], sketch.rank(fields[i]));
            }
            else
            {
                auto x = sketch.query(fields_double[i]).x;
                printf("rank(%lu) = %.2lf\n", x, sketch.rank(x));
                printf("quantile(%.2lf) = %lu\n", fields_double[i], x);
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
