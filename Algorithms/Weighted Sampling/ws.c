#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct instance
{
    int val;
    int weight;
}Instance;

typedef struct heap
{
    Instance *instances;
    int       count;

}Heap;

typedef struct ws
{
    Heap  *high;
    Heap  *low;
    float tal;
    int   count;
    int   limit;
    int   len;
}WS;

void insert_min_heap(Heap *heap, Instance *instance){
    heap->instances[heap->count] = *instance;
    heapify_bottom_top(heap,heap->count);
    heap->count++;
    
    return;
}

void heapify_bottom_top(Heap *heap,int index){
    Instance temp;
    int parent_node = (index-1)/2;

    int new_val= heap->instances[index].weight;
    int parent_val = heap->instances[parent_node].weight;

    if(parent_val > new_val){

        temp = heap->instances[parent_node];
        heap->instances[parent_node] = heap->instances[index];
        heap->instances[index] = temp;
        heapify_bottom_top(heap,parent_node);

    return;
    }
}


void heapify_top_bottom(Heap *heap, int parent_node){
    int left = parent_node*2+1;
    int right = parent_node*2+2;
    int min;
    Instance temp;

    if(left >= heap->count || left <0)
        left = -1;
    if(right >= heap->count || right <0)
        right = -1;

    if(left != -1 && heap->instances[left].weight < heap->instances[parent_node].weight)
        min=left;
    else
        min =parent_node;
    if(right != -1 && heap->instances[right].weight < heap->instances[min].weight)
        min = right;

    if(min != parent_node){
        temp = heap->instances[min];
        heap->instances[min] = heap->instances[parent_node];
        heap->instances[parent_node] = temp;

        heapify_top_bottom(heap, min);
    }
}

void PopMin(Heap *heap){
    Instance pop;
    if(heap->count==0){
        printf("\n__Heap is Empty__\n");
        return -1;
    }
    pop = heap->instances[0];
    heap->instances[0] = heap->instances[heap->count-1];
    heap->count--;
    heapify_top_bottom(heap, 0);
    return;
}


WS *init(int size){
    WS *sketch = malloc(sizeof(WS));
    sketch->count = 0;
    sketch->len = 0;
    sketch->tal = 0;
    sketch->limit = size;
    //sketch->data = malloc(size*sizeof(int));
    
    sketch->high = malloc(sizeof(Heap));
    sketch->high->instances = malloc(size*sizeof(Instance));
    sketch->high->count = 0;

    sketch->low = malloc(sizeof(Heap));
    sketch->low->instances = NULL;
    sketch->low->count = 0;
   
    return sketch;
}

WS *update(WS *sketch, Instance x){   // Update fors simple RS. TODO: add tal update
    if( sketch->len < sketch->limit){
        insert_min_heap(sketch->high,&x);
        sketch->count += 1;
        sketch->len   += 1;
    }
    
    else{
        sketch->count += 1;
        int i = uniform_distribution(0,sketch->count);
        if(i < sketch->limit){
            PopMin(sketch->high);
            insert_min_heap(sketch->high,&x);
            
        }
    }

    return sketch;
}

void query(WS *sketch){
    for(int i  = 0 ; i < sketch->len ;i++){
        printf("val = %d, weight = %d \n ", sketch->high->instances[i].val, sketch->high->instances[i].weight);
    }
    printf("\n");
}

typedef struct
{
    Instance *data;
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
    Instance *data = malloc(size*sizeof(Instance));
    for(int i = 0; i < size; i++){
        data[i].val = i;
        data[i].weight = 10 - i;
    }
    Array *array = malloc(1*sizeof(Array));
    array->data = data;
    array->size = size;

    return array;

}

void print_sample(Array *array){
    Instance *data = array->data;
    for(int i = 0; i < array->size; i++){
        printf("val = %d, weight = %d\n", data[i].val, data[i].weight);
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
    
    for(int i = 0; i < tst_size; i++){
        sketch = update(sketch, tst->data[i]);
    }
    printf("Random Sample:\n"); 
    query(sketch);

    return 0;   
}