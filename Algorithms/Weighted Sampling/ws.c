#include <stdio.h>
#include <stdlib.h>

typedef struct ws
{
    int *data;
    int count;
    int limit;
}WS;

WS *init(int size){
    WS *sketch = malloc(sizeof(WS));
    sketch->count = 0;
    sketch->limit = size;
    sketch->data = malloc(size*sizeof(int));
    return sketch;
}

WS *update(WS *sketch, int x){
    
    if(sizeof(sketch->data)/sizeof(int) < sketch->limit){
        printf("x = %d\n", x);
        sketch->data[sketch->count] = x;
        sketch->count += 1;
    }
    
    else{
        sketch->count += 1;
        int i = uniform_distribution(1,sketch->count);
        if(i < sketch->limit){
            sketch->data[i] = x;
        }
    }

    return sketch;
}
void query(WS *sketch){
    for(int i  = 0 ; i < (sizeof(sketch->data)/sizeof(int));i++){
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
    //srand(time(0));
    double myRand = rand()/(1.0 + RAND_MAX); 
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

Array *create_sample(int size){
    int *data = malloc(size*sizeof(int));
    for(int i = 0; i < size; i++){
        data[i] = uniform_distribution(0,100);
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
    Array *tst = create_sample(tst_size);
    print_sample(tst);

    WS *sketch = init(3);
    
    for(int i = 0; i <  tst_size; i++){
        sketch = update(sketch, tst->data[i]);
    }
        
    query(sketch);

    return 0;   
}