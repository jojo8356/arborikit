#include "arborikit/rbtree.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rb_node {
    void *data;
    struct rb_node *left;
    struct rb_node *right;
    struct rb_node *parent;
    rb_color color;
};

struct rb_tree {
    rb_node *nil;    /* sentinelle, toujours noire */
    rb_node *root;
    size_t size;
    rb_cmp_fn cmp;
    void (*free_fn)(void *data);
};

int rb_compare_int(const void *a, const void *b)
{
    intptr_t x = (intptr_t)a;
    intptr_t y = (intptr_t)b;

    if (x < y) {
        return -1;
    }
    return x > y ? 1 : 0;
}

int rb_compare_str(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static rb_node *node_new(void *data, rb_color color)
{
    rb_node *node = (rb_node *)calloc(1, sizeof(rb_node));

    if (node == NULL) {
        return NULL;
    }
    node->data = data;
    node->color = color;
    return node;
}

rb_tree *rb_create_ex(rb_cmp_fn cmp, void (*free_fn)(void *data))
{
    rb_tree *tree = (rb_tree *)calloc(1, sizeof(rb_tree));

    if (tree == NULL) {
        return NULL;
    }
    tree->nil = node_new(NULL, RB_BLACK);
    if (tree->nil == NULL) {
        free(tree);
        return NULL;
    }
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = NULL;
    tree->root = tree->nil;
    tree->cmp = cmp != NULL ? cmp : rb_compare_int;
    tree->free_fn = free_fn;
    return tree;
}

rb_tree *rb_create(rb_cmp_fn cmp)
{
    return rb_create_ex(cmp, NULL);
}

/* ---------------------------- rotations ---------------------------- */

static void left_rotate(rb_tree *t, rb_node *x)
{
    rb_node *y = x->right;

    x->right = y->left;
    if (y->left != t->nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

static void right_rotate(rb_tree *t, rb_node *x)
{
    rb_node *y = x->left;

    x->left = y->right;
    if (y->right != t->nil) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        t->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

/* ---------------------------- insertion ---------------------------- */

static void insert_fixup(rb_tree *t, rb_node *z)
{
    while (z->parent != NULL && z->parent->color == RB_RED) {
        rb_node *grand = z->parent->parent;

        if (z->parent == grand->left) {
            rb_node *uncle = grand->right;
            if (uncle->color == RB_RED) {          /* cas 1 */
                z->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grand->color = RB_RED;
                z = grand;
            } else {
                if (z == z->parent->right) {       /* cas 2 */
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = RB_BLACK;       /* cas 3 */
                z->parent->parent->color = RB_RED;
                right_rotate(t, z->parent->parent);
            }
        } else {
            rb_node *uncle = grand->left;
            if (uncle->color == RB_RED) {
                z->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grand->color = RB_RED;
                z = grand;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = RB_BLACK;
}

int rb_insert(rb_tree *t, void *data)
{
    rb_node *y = NULL;
    rb_node *x;
    rb_node *z;

    if (t == NULL) {
        return 0;
    }
    x = t->root;
    while (x != t->nil) {
        int c = t->cmp(data, x->data);
        if (c == 0) {
            return 0;
        }
        y = x;
        x = c < 0 ? x->left : x->right;
    }
    z = node_new(data, RB_RED);
    if (z == NULL) {
        return 0;
    }
    z->left = t->nil;
    z->right = t->nil;
    z->parent = y;
    if (y == NULL) {
        t->root = z;
    } else if (t->cmp(data, y->data) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }
    t->size++;
    insert_fixup(t, z);
    return 1;
}

/* ---------------------------- recherche ---------------------------- */

static rb_node *find_node(const rb_tree *t, const void *data)
{
    rb_node *cur;

    if (t == NULL) {
        return NULL;
    }
    cur = t->root;
    while (cur != t->nil) {
        int c = t->cmp(data, cur->data);
        if (c == 0) {
            return cur;
        }
        cur = c < 0 ? cur->left : cur->right;
    }
    return NULL;
}

void *rb_find(const rb_tree *t, const void *data)
{
    rb_node *n = find_node(t, data);
    return n == NULL ? NULL : n->data;
}

int rb_contains(const rb_tree *t, const void *data)
{
    return find_node(t, data) != NULL;
}

static rb_node *subtree_min(rb_tree *t, rb_node *x)
{
    while (x->left != t->nil) {
        x = x->left;
    }
    return x;
}

static rb_node *subtree_max(rb_tree *t, rb_node *x)
{
    while (x->right != t->nil) {
        x = x->right;
    }
    return x;
}

void *rb_min(const rb_tree *t)
{
    if (t == NULL || t->root == t->nil) {
        return NULL;
    }
    return subtree_min((rb_tree *)t, t->root)->data;
}

void *rb_max(const rb_tree *t)
{
    if (t == NULL || t->root == t->nil) {
        return NULL;
    }
    return subtree_max((rb_tree *)t, t->root)->data;
}

void *rb_successor(const rb_tree *t, const void *data)
{
    rb_node *x;
    rb_node *y;

    if (t == NULL) {
        return NULL;
    }
    x = find_node(t, data);
    if (x == NULL) {
        return NULL;
    }
    if (x->right != t->nil) {
        return subtree_min((rb_tree *)t, x->right)->data;
    }
    y = x->parent;
    while (y != NULL && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y == NULL ? NULL : y->data;
}

void *rb_predecessor(const rb_tree *t, const void *data)
{
    rb_node *x;
    rb_node *y;

    if (t == NULL) {
        return NULL;
    }
    x = find_node(t, data);
    if (x == NULL) {
        return NULL;
    }
    if (x->left != t->nil) {
        return subtree_max((rb_tree *)t, x->left)->data;
    }
    y = x->parent;
    while (y != NULL && x == y->left) {
        x = y;
        y = y->parent;
    }
    return y == NULL ? NULL : y->data;
}

/* --------------------------- suppression --------------------------- */

static void transplant(rb_tree *t, rb_node *u, rb_node *v)
{
    if (u->parent == NULL) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static rb_node *subtree_min_const(const rb_tree *t, rb_node *x)
{
    while (x->left != t->nil) {
        x = x->left;
    }
    return x;
}

static void delete_fixup(rb_tree *t, rb_node *x)
{
    while (x != t->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            rb_node *w = x->parent->right;
            if (w->color == RB_RED) {                        /* cas 1 */
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                left_rotate(t, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;                           /* cas 2 */
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {           /* cas 3 */
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    right_rotate(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;                 /* cas 4 */
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            rb_node *w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                right_rotate(t, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    left_rotate(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = RB_BLACK;
}

int rb_delete(rb_tree *t, void *data)
{
    rb_node *z;
    rb_node *y;
    rb_node *x;
    rb_color y_original;
    void *freed = NULL;

    if (t == NULL) {
        return 0;
    }
    z = find_node(t, data);
    if (z == NULL) {
        return 0;
    }

    y = z;
    y_original = y->color;
    if (z->left == t->nil) {
        x = z->right;
        freed = z->data;
        transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        freed = z->data;
        transplant(t, z, z->left);
    } else {
        y = subtree_min_const(t, z->right);
        y_original = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
        freed = z->data;
    }
    free(z);
    t->size--;
    if (y_original == RB_BLACK) {
        delete_fixup(t, x);
    }
    if (freed != NULL && t->free_fn != NULL) {
        t->free_fn(freed);
    }
    return 1;
}

/* ----------------------------- parcours ---------------------------- */

static void walk(const rb_tree *t, rb_node *node, rb_visit_fn visit, int order,
                 void *ctx, size_t *count)
{
    if (node == t->nil) {
        return;
    }
    if (order == 0 && visit != NULL && visit(node->data, ctx) != 0) {
        return;
    }
    walk(t, node->left, visit, order, ctx, count);
    if (order == 1 && visit != NULL && visit(node->data, ctx) != 0) {
        return;
    }
    walk(t, node->right, visit, order, ctx, count);
    if (order == 2 && visit != NULL && visit(node->data, ctx) != 0) {
        return;
    }
    (*count)++;
}

static size_t walk_dispatch(const rb_tree *t, rb_visit_fn visit, int order, void *ctx)
{
    size_t count = 0;

    if (t == NULL) {
        return 0;
    }
    walk(t, t->root, visit != NULL ? visit : NULL, order, ctx, &count);
    return count;
}

size_t rb_inorder(const rb_tree *t, rb_visit_fn visit, void *ctx)
{
    return walk_dispatch(t, visit, 1, ctx);
}

size_t rb_preorder(const rb_tree *t, rb_visit_fn visit, void *ctx)
{
    return walk_dispatch(t, visit, 0, ctx);
}

size_t rb_postorder(const rb_tree *t, rb_visit_fn visit, void *ctx)
{
    return walk_dispatch(t, visit, 2, ctx);
}

size_t rb_levelorder(const rb_tree *t, rb_visit_fn visit, void *ctx)
{
    size_t capacity;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    rb_node **queue;

    if (t == NULL || t->root == t->nil) {
        return 0;
    }
    capacity = t->size + 1;
    queue = (rb_node **)malloc(capacity * sizeof(*queue));
    if (queue == NULL) {
        return 0;
    }
    queue[tail++] = t->root;
    while (head < tail) {
        rb_node *cur = queue[head++];
        count++;
        if (visit != NULL && visit(cur->data, ctx) != 0) {
            break;
        }
        if (cur->left != t->nil && tail < capacity) {
            queue[tail++] = cur->left;
        }
        if (cur->right != t->nil && tail < capacity) {
            queue[tail++] = cur->right;
        }
    }
    free(queue);
    return count;
}

/* ----------------------------- mesures ----------------------------- */

size_t rb_size(const rb_tree *t)
{
    return t == NULL ? 0 : t->size;
}

int rb_is_empty(const rb_tree *t)
{
    return t == NULL || t->root == t->nil;
}

static size_t height_rec(const rb_tree *t, rb_node *node)
{
    size_t left;
    size_t right;

    if (node == t->nil) {
        return 0;
    }
    left = height_rec(t, node->left);
    right = height_rec(t, node->right);
    return 1 + (left > right ? left : right);
}

size_t rb_height(const rb_tree *t)
{
    if (t == NULL || t->root == t->nil) {
        return 0;
    }
    return height_rec(t, t->root) - 1; /* nombre d'arêtes */
}

static size_t black_height_rec(const rb_tree *t, rb_node *node)
{
    if (node == t->nil) {
        return 1;
    }
    return black_height_rec(t, node->left) + (node->color == RB_BLACK ? 1 : 0);
}

size_t rb_black_height(const rb_tree *t)
{
    if (t == NULL || t->root == t->nil) {
        return 0;
    }
    return black_height_rec(t, t->root);
}

static size_t count_color_rec(const rb_tree *t, rb_node *node, rb_color color)
{
    size_t total;

    if (node == t->nil) {
        return 0;
    }
    total = node->color == color ? 1 : 0;
    total += count_color_rec(t, node->left, color);
    total += count_color_rec(t, node->right, color);
    return total;
}

size_t rb_count_color(const rb_tree *t, rb_color color)
{
    if (t == NULL || t->root == t->nil) {
        return 0;
    }
    return count_color_rec(t, t->root, color);
}

/* --------------------------- vérification -------------------------- */

static int set_err(char *err, size_t err_size, const char *msg)
{
    if (err != NULL && err_size > 0) {
        strncpy(err, msg, err_size - 1);
        err[err_size - 1] = '\0';
    }
    return 0;
}

static int check_abrr(const rb_tree *t, rb_node *node, const void *lo, const void *hi,
                      char *err, size_t err_size)
{
    if (node == t->nil) {
        return 1;
    }
    if (lo != NULL && t->cmp(node->data, lo) <= 0) {
        return set_err(err, err_size, "propriete ABR violee (borne basse)");
    }
    if (hi != NULL && t->cmp(node->data, hi) >= 0) {
        return set_err(err, err_size, "propriete ABR violee (borne haute)");
    }
    if (node->color == RB_RED) {
        if (node->left->color == RB_RED || node->right->color == RB_RED) {
            return set_err(err, err_size, "invariant 4 viole (rouge-rouge)");
        }
    }
    if (!check_abrr(t, node->left, lo, node->data, err, err_size)) {
        return 0;
    }
    return check_abrr(t, node->right, node->data, hi, err, err_size);
}

/* Retourne la hauteur noire, ou -1 si incohérente. */
static long check_black_height(const rb_tree *t, rb_node *node)
{
    long left;
    long right;

    if (node == t->nil) {
        return 1;
    }
    left = check_black_height(t, node->left);
    right = check_black_height(t, node->right);
    if (left < 0 || right < 0 || left != right) {
        return -1;
    }
    return left + (node->color == RB_BLACK ? 1 : 0);
}

int rb_validate(const rb_tree *t, char *err, size_t err_size)
{
    if (t == NULL) {
        return set_err(err, err_size, "arbre NULL");
    }
    if (t->root == t->nil) {
        return 1;
    }
    if (t->root->color == RB_RED) {
        return set_err(err, err_size, "invariant 2 viole (racine rouge)");
    }
    if (t->root->parent != NULL) {
        return set_err(err, err_size, "la racine possede un parent");
    }
    if (!check_abrr(t, t->root, NULL, NULL, err, err_size)) {
        return 0;
    }
    if (check_black_height(t, t->root) < 0) {
        return set_err(err, err_size, "invariant 5 viole (hauteurs noires)");
    }
    return 1;
}

/* ---------------------------- affichage ---------------------------- */

static void print_rec(const rb_tree *t, rb_node *node, const char *prefix, int is_last)
{
    char buffer[512];

    if (node == t->nil) {
        return;
    }
    printf("%s%s%ld(%c)\n", prefix, is_last ? "\\-- " : "|-- ",
           (long)(intptr_t)node->data, node->color == RB_RED ? 'R' : 'N');
    snprintf(buffer, sizeof(buffer), "%s%s", prefix, is_last ? "    " : "|   ");
    if (node->left != t->nil && node->right != t->nil) {
        print_rec(t, node->left, buffer, 0);
        print_rec(t, node->right, buffer, 1);
    } else if (node->left != t->nil) {
        print_rec(t, node->left, buffer, 1);
    } else if (node->right != t->nil) {
        print_rec(t, node->right, buffer, 1);
    }
}

void rb_print_int(const rb_tree *t)
{
    if (t == NULL) {
        return;
    }
    print_rec(t, t->root, "", 1);
}

/* ----------------------------- accès ------------------------------- */

rb_node *rb_root(const rb_tree *t)
{
    if (t == NULL || t->root == t->nil) {
        return NULL;
    }
    return t->root;
}

const void *rb_node_data(const rb_node *node)
{
    return node == NULL ? NULL : node->data;
}

rb_color rb_node_color(const rb_node *node)
{
    return node == NULL ? RB_BLACK : node->color;
}

rb_node *rb_node_left(const rb_node *node)
{
    return node == NULL ? NULL : node->left;
}

rb_node *rb_node_right(const rb_node *node)
{
    return node == NULL ? NULL : node->right;
}

rb_node *rb_node_parent(const rb_node *node)
{
    return node == NULL ? NULL : node->parent;
}

/* ---------------------------- libération --------------------------- */

static void free_rec(rb_tree *t, rb_node *node)
{
    if (node == t->nil) {
        return;
    }
    free_rec(t, node->left);
    free_rec(t, node->right);
    if (t->free_fn != NULL && node->data != NULL) {
        t->free_fn(node->data);
    }
    free(node);
}

void rb_destroy(rb_tree *t)
{
    if (t == NULL) {
        return;
    }
    free_rec(t, t->root);
    free(t->nil);
    free(t);
}
