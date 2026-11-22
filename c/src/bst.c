#include "arborikit/bst.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bst_node {
    void *data;
    struct bst_node *left;
    struct bst_node *right;
};

struct bst {
    bst_node *root;
    size_t size;
    bst_cmp_fn cmp;
    void (*free_fn)(void *data);
};

int bst_compare_int(const void *a, const void *b)
{
    intptr_t x = (intptr_t)a;
    intptr_t y = (intptr_t)b;

    if (x < y) {
        return -1;
    }
    return x > y ? 1 : 0;
}

int bst_compare_str(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static bst_node *node_new(void *data)
{
    bst_node *node = (bst_node *)calloc(1, sizeof(bst_node));

    if (node == NULL) {
        return NULL;
    }
    node->data = data;
    return node;
}

bst *bst_create_ex(bst_cmp_fn cmp, void (*free_fn)(void *data))
{
    bst *tree = (bst *)calloc(1, sizeof(bst));

    if (tree == NULL) {
        return NULL;
    }
    tree->cmp = cmp != NULL ? cmp : bst_compare_int;
    tree->free_fn = free_fn;
    return tree;
}

bst *bst_create(bst_cmp_fn cmp)
{
    return bst_create_ex(cmp, NULL);
}

int bst_insert(bst *tree, void *data)
{
    bst_node *cur;
    bst_node *node;

    if (tree == NULL) {
        return 0;
    }
    if (tree->root == NULL) {
        tree->root = node_new(data);
        if (tree->root == NULL) {
            return 0;
        }
        tree->size = 1;
        return 1;
    }
    cur = tree->root;
    for (;;) {
        int c = tree->cmp(data, cur->data);
        if (c == 0) {
            return 0;
        }
        if (c < 0) {
            if (cur->left == NULL) {
                node = node_new(data);
                if (node == NULL) {
                    return 0;
                }
                cur->left = node;
                tree->size++;
                return 1;
            }
            cur = cur->left;
        } else {
            if (cur->right == NULL) {
                node = node_new(data);
                if (node == NULL) {
                    return 0;
                }
                cur->right = node;
                tree->size++;
                return 1;
            }
            cur = cur->right;
        }
    }
}

void *bst_find(const bst *tree, const void *data)
{
    const bst_node *cur;

    if (tree == NULL) {
        return NULL;
    }
    cur = tree->root;
    while (cur != NULL) {
        int c = tree->cmp(data, cur->data);
        if (c == 0) {
            return cur->data;
        }
        cur = c < 0 ? cur->left : cur->right;
    }
    return NULL;
}

int bst_contains(const bst *tree, const void *data)
{
    return bst_find(tree, data) != NULL;
}

static bst_node *min_node(bst_node *node)
{
    while (node != NULL && node->left != NULL) {
        node = node->left;
    }
    return node;
}

static bst_node *max_node(bst_node *node)
{
    while (node != NULL && node->right != NULL) {
        node = node->right;
    }
    return node;
}

void *bst_min(const bst *tree)
{
    bst_node *n;

    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }
    n = min_node(tree->root);
    return n == NULL ? NULL : n->data;
}

void *bst_max(const bst *tree)
{
    bst_node *n;

    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }
    n = max_node(tree->root);
    return n == NULL ? NULL : n->data;
}

void *bst_successor(const bst *tree, const void *data)
{
    const bst_node *cur;
    const bst_node *best = NULL;

    if (tree == NULL) {
        return NULL;
    }
    cur = tree->root;
    while (cur != NULL) {
        int c = tree->cmp(data, cur->data);
        if (c < 0) {
            best = cur;
            cur = cur->left;
        } else if (c > 0) {
            cur = cur->right;
        } else {
            if (cur->right != NULL) {
                bst_node *m = min_node(cur->right);
                return m == NULL ? NULL : m->data;
            }
            return best == NULL ? NULL : best->data;
        }
    }
    return best == NULL ? NULL : best->data;
}

void *bst_predecessor(const bst *tree, const void *data)
{
    const bst_node *cur;
    const bst_node *best = NULL;

    if (tree == NULL) {
        return NULL;
    }
    cur = tree->root;
    while (cur != NULL) {
        int c = tree->cmp(data, cur->data);
        if (c > 0) {
            best = cur;
            cur = cur->right;
        } else if (c < 0) {
            cur = cur->left;
        } else {
            if (cur->left != NULL) {
                bst_node *m = max_node(cur->left);
                return m == NULL ? NULL : m->data;
            }
            return best == NULL ? NULL : best->data;
        }
    }
    return best == NULL ? NULL : best->data;
}

static bst_node *remove_min(bst_node *node, void **out_data)
{
    bst_node *right;

    if (node->left == NULL) {
        *out_data = node->data;
        right = node->right;
        free(node);
        return right;
    }
    node->left = remove_min(node->left, out_data);
    return node;
}

/*
 * Supprime `data`. `freed_data` reçoit le pointeur de données réellement
 * retiré de l'arbre, pour que l'appelant applique son destructeur.
 * Dans le cas à deux enfants, le successeur est déplacé : sa donnée reste
 * vivante (elle est réutilisée), c'est l'ancienne donnée du nœud qui sort.
 */
static bst_node *delete_rec(bst_node *node, const void *data, bst_cmp_fn cmp,
                           int *removed, void **freed_data)
{
    int c;

    if (node == NULL) {
        return NULL;
    }
    c = cmp(data, node->data);
    if (c < 0) {
        node->left = delete_rec(node->left, data, cmp, removed, freed_data);
        return node;
    }
    if (c > 0) {
        node->right = delete_rec(node->right, data, cmp, removed, freed_data);
        return node;
    }

    *removed = 1;
    if (node->left == NULL) {
        bst_node *right = node->right;
        *freed_data = node->data;
        free(node);
        return right;
    }
    if (node->right == NULL) {
        bst_node *left = node->left;
        *freed_data = node->data;
        free(node);
        return left;
    }
    {
        void *successor_data = NULL;
        node->right = remove_min(node->right, &successor_data);
        *freed_data = node->data;
        node->data = successor_data;
        return node;
    }
}

int bst_delete(bst *tree, void *data)
{
    int removed = 0;
    void *freed_data = NULL;

    if (tree == NULL) {
        return 0;
    }
    tree->root = delete_rec(tree->root, data, tree->cmp, &removed, &freed_data);
    if (removed) {
        tree->size--;
        if (freed_data != NULL && tree->free_fn != NULL) {
            tree->free_fn(freed_data);
        }
    }
    return removed;
}

size_t bst_size(const bst *tree)
{
    return tree == NULL ? 0 : tree->size;
}

int bst_is_empty(const bst *tree)
{
    return tree == NULL || tree->root == NULL;
}

static size_t height_rec(const bst_node *node)
{
    size_t left;
    size_t right;

    if (node == NULL) {
        return 0;
    }
    left = height_rec(node->left);
    right = height_rec(node->right);
    return 1 + (left > right ? left : right);
}

size_t bst_height(const bst *tree)
{
    if (tree == NULL) {
        return 0;
    }
    return height_rec(tree->root);
}

int bst_is_degenerate(const bst *tree)
{
    return tree != NULL && tree->size > 0 && bst_height(tree) == tree->size;
}

static void walk(const bst_node *node, bst_visit_fn visit, int order, void *ctx,
                 size_t *count, int *stop)
{
    if (node == NULL || *stop) {
        return;
    }
    if (order == 0) { /* préordre */
        if (visit != NULL && visit(node->data, ctx) != 0) {
            *stop = 1;
            return;
        }
        (*count)++;
    }
    walk(node->left, visit, order, ctx, count, stop);
    if (order == 1) { /* infixe */
        if (*stop) {
            return;
        }
        if (visit != NULL && visit(node->data, ctx) != 0) {
            *stop = 1;
            return;
        }
        (*count)++;
    }
    walk(node->right, visit, order, ctx, count, stop);
    if (order == 2 && !*stop) { /* postfixe */
        if (visit != NULL && visit(node->data, ctx) != 0) {
            *stop = 1;
            return;
        }
        (*count)++;
    }
}

size_t bst_inorder(const bst *tree, bst_visit_fn visit, void *ctx)
{
    size_t count = 0;
    int stop = 0;

    if (tree == NULL) {
        return 0;
    }
    walk(tree->root, visit, 1, ctx, &count, &stop);
    return count;
}

size_t bst_preorder(const bst *tree, bst_visit_fn visit, void *ctx)
{
    size_t count = 0;
    int stop = 0;

    if (tree == NULL) {
        return 0;
    }
    walk(tree->root, visit, 0, ctx, &count, &stop);
    return count;
}

size_t bst_postorder(const bst *tree, bst_visit_fn visit, void *ctx)
{
    size_t count = 0;
    int stop = 0;

    if (tree == NULL) {
        return 0;
    }
    walk(tree->root, visit, 2, ctx, &count, &stop);
    return count;
}

size_t bst_levelorder(const bst *tree, bst_visit_fn visit, void *ctx)
{
    size_t capacity;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    bst_node **queue;

    if (tree == NULL || tree->root == NULL) {
        return 0;
    }
    capacity = tree->size + 1;
    queue = (bst_node **)malloc(capacity * sizeof(*queue));
    if (queue == NULL) {
        return 0;
    }
    queue[tail++] = tree->root;
    while (head < tail) {
        bst_node *cur = queue[head++];
        count++;
        if (visit != NULL && visit(cur->data, ctx) != 0) {
            break;
        }
        if (cur->left != NULL && tail < capacity) {
            queue[tail++] = cur->left;
        }
        if (cur->right != NULL && tail < capacity) {
            queue[tail++] = cur->right;
        }
    }
    free(queue);
    return count;
}

static int validate_rec(const bst *tree, const bst_node *node, const void *lo,
                        const void *hi)
{
    if (node == NULL) {
        return 1;
    }
    if (lo != NULL && tree->cmp(node->data, lo) <= 0) {
        return 0;
    }
    if (hi != NULL && tree->cmp(node->data, hi) >= 0) {
        return 0;
    }
    return validate_rec(tree, node->left, lo, node->data) &&
           validate_rec(tree, node->right, node->data, hi);
}

int bst_validate(const bst *tree)
{
    if (tree == NULL) {
        return 1;
    }
    return validate_rec(tree, tree->root, NULL, NULL);
}

static void print_rec(const bst_node *node, const char *prefix, int is_last)
{
    if (node == NULL) {
        return;
    }
    printf("%s%s%ld\n", prefix, is_last ? "\\-- " : "|-- ",
           (long)(intptr_t)node->data);
    {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s%s", prefix, is_last ? "    " : "|   ");
        if (node->left != NULL && node->right != NULL) {
            print_rec(node->left, buffer, 0);
            print_rec(node->right, buffer, 1);
        } else if (node->left != NULL) {
            print_rec(node->left, buffer, 1);
        } else if (node->right != NULL) {
            print_rec(node->right, buffer, 1);
        }
    }
}

void bst_print_int(const bst *tree)
{
    if (tree == NULL) {
        return;
    }
    print_rec(tree->root, "", 1);
}

static void free_rec(bst *tree, bst_node *node)
{
    if (node == NULL) {
        return;
    }
    free_rec(tree, node->left);
    free_rec(tree, node->right);
    if (tree->free_fn != NULL && node->data != NULL) {
        tree->free_fn(node->data);
    }
    free(node);
}

void bst_destroy(bst *tree)
{
    if (tree == NULL) {
        return;
    }
    free_rec(tree, tree->root);
    free(tree);
}
