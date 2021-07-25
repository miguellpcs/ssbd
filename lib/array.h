#ifndef array_h
#define array_h

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "instance.h"
#include "memory.h"

int uniform_distribution(int rangeLow, int rangeHigh)
{
    srand(time(0));
    double myRand = rand() / (1.0 + RAND_MAX);
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

double uniform_distribution_real(double rangeLow, double rangeHigh)
{
    srand(time(0));
    double myRand = rand() / (1.0 + RAND_MAX);
    double range = rangeHigh - rangeLow + 1;
    double myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

typedef struct
{
    Instance *data;
    int size;
} Array;

Array *create_sample(int size)
{
    Instance *data = (Instance *)check_malloc(size * sizeof(Instance));
    for (int i = 0; i < size; i++)
    {
        data[i].val = check_malloc(sizeof(int));
        *((int *)data[i].val) = i;
        data[i].weight = size - i + (uniform_distribution(0, 100));
    }
    Array *array = (Array *)check_malloc(1 * sizeof(Array));
    array->data = data;
    array->size = size;

    return array;
}

void print_sample(Array *array)
{
    Instance *data = array->data;
    for (int i = 0; i < array->size; i++)
    {
        printf("val = %d, weight = %d\n", *((int *)data[i].val), data[i].weight);
    }
    printf("\n");
}

#endif