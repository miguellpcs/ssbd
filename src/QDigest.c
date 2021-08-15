#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
int c = 0;
typedef struct instance
{
    int val;
    int weight;
}Instance;

typedef struct node
{
    struct node *left;
    struct node *right;
    Instance instance;
}Node;


typedef struct tree
{
    Node *root;
    // add useful information?
    
}Tree;

typedef struct qd{
    Node *root;
    int W, V;
    float C, E;
    int size;
}QD;

typedef struct
{
    Instance *data;
    int size;
}Array;


int min(int x, int y)
{
return y ^ ((x ^ y) & -(x < y));
}


Node *create_tree(int N){
    Node *new;
    new = malloc(sizeof(Node));
    new->instance.val = -1;

    if(N == 0 ){
        new->right = NULL;
        new->left = NULL;
        new->instance.val = c;
        c++;
        return new;
    }

    new->left = create_tree(N-1);
    new->right = create_tree(N-1);
    
    

    return new;
    
}

void inorder(struct node *root)
{
    if(root!=NULL) 
    {   
        inorder(root->left); 
        printf(" %d ", root->instance.val);
        inorder(root->right);
        
    }
}

void search(Node *root, int key,  int w, int acc, QD *sketch, int size){
    if(root->instance.weight < sketch->C && !0){ // COLOCAR ESSE IF APENAS PRO UPDATE, NÃO É USADO NO QUERY
        float d = min(c- root->instance.weight,w);
        root->instance.weight += d;
        w -= d;
    }
    if(w==0){
        return;
    }
    if(size == 1 ){
        printf("key %d found", root->instance.val);
    }
    else if(key >= size/2 + acc){
        //printf("right\n");
        search(root->right, key, w, acc + size/2, sketch,size/2);
    }
    else{
        //printf("left\n");
        search(root->left, key, w,  acc, sketch, size/2);
    }
    

}


void init(QD *sketch, float E, float  V, int N){
    sketch->root = create_tree(N);
    sketch->V = V;
    sketch->E = E;
    sketch->W = 0;
    sketch->size = pow(2,N);
}

void update(QD *sketch, int x, int w){
    sketch->W += w;
    sketch->C = (sketch->E * sketch->W)/log(sketch->V);
    search(sketch->root,x,w,0,sketch,sketch->size);
    sketch->root->instance.weight += w;

}
void rank_query(Node root, int idx){


}
int main(){
    int N = 3;
    QD *sketch = malloc(sizeof(sketch));
    init(sketch,3,4,N);
    
  
    
    int key = 3;
    int size = pow(2,N);

    update(sketch,3,10);
    


   //inorder(tst_tree->root);


    printf("\n");

    

    

}

