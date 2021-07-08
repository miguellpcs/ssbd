#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct ws
{
    int *data;
    int count;
    int limit;
    int len;
}WS;

WS *init(int size){
    WS *sketch = malloc(sizeof(WS));
    sketch->count = 0;
    sketch->len = 0;
    sketch->limit = size;
    sketch->data = malloc(size*sizeof(int));
    
    
    return sketch;
}

WS *update(WS *sketch, int x){
    if( sketch->len < sketch->limit){
        sketch->data[sketch->count] = x;
        sketch->count += 1;
        sketch->len   += 1;
    }
    
    else{
        sketch->count += 1;
        int i = uniform_distribution(0,sketch->count);
        if(i < sketch->limit){
            sketch->data[i] = x;
        }
    }

    return sketch;
}
void query(WS *sketch){
    for(int i  = 0 ; i < sketch->len ;i++){
        printf("%d ", sketch->data[i]);
    }
    printf("\n");
}

typedef struct
{
    int *data;
    int size;
}Array;

int uniform_distribution(int rangeLow, int rangeHigh) {
    srand(time(0));
    double myRand = rand()/(1.0 + RAND_MAX); 
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

Array *create_sample(int size){
    int *data = malloc(size*sizeof(int));
    for(int i = 0; i < size; i++){
        data[i] = i;
    }
    Array *array = malloc(1*sizeof(Array));
    array->data = data;
    array->size = size;

    return array;

}

void print_sample(Array *array){
    int *data = array->data;
    for(int i = 0; i < array->size; i++){
        printf("%d ", data[i]);
    }
    printf("\n");
    

}

int main(){
    int tst_size = 10;
    int random_sample_size=5;

    Array *tst = create_sample(tst_size);

    printf("Stream:\n");
    print_sample(tst);
    printf("Random Sample size: %d \n", random_sample_size);

    WS *sketch = init(random_sample_size);
    
    for(int i = 0; i <  tst_size; i++){
        sketch = update(sketch, tst->data[i]);
    }
    printf("Random Sample:\n"); 
    query(sketch);

    return 0;   
}