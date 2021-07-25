#ifndef sorting_h
#define sorting_h

#include <stdint.h>

void swap(uint64_t *a, uint64_t *b)
{
    uint64_t temp = *a;
    *a = *b;
    *b = temp;
}

int partition(uint64_t *arr, int l, int r)
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

uint64_t kthSmallest(uint64_t *arr, int l, int r, int k)
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

    return UINT64_MAX;
}

#endif