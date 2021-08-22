#if !defined(tree_h)
#define tree_h

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

typedef struct
{
    int val;
    int weight;
} Instance;

typedef struct Node
{
    struct Node *left;
    struct Node *right;
    Instance instance;
} Node;

int c = 0;
Node *create_tree(int N)
{
    Node *tmp = (Node *)check_malloc(sizeof(Node));
    tmp->instance.val = -1;

    /*



     -    -    -    -    -
    0 1  2 3  4 5  6 7  8 9
    */
    if (N == 0)
    {
        tmp->right = NULL;
        tmp->left = NULL;
        tmp->instance.val = c;
        c++;
        return tmp;
    }

    tmp->left = create_tree(N - 1);
    tmp->right = create_tree(N - 1);

    return tmp;
}

typedef void (*execute_fn)(Node *v);
void inorder(Node *root, execute_fn execute)
{
    if (root != NULL)
    {
        inorder(root->left, execute);
        execute(root);
        inorder(root->right, execute);
    }
}

void postorder(Node *root, execute_fn execute)
{
    if (root != NULL)
    {
        postorder(root->left, execute);
        postorder(root->right, execute);
        execute(root);
    }
}

#endif // tree_h
