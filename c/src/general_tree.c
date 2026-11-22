#include "arborikit/general_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_node {
    char *data;
    struct tree_node *parent;
    struct tree_node *first_child;
    struct tree_node *last_child;
    struct tree_node *next_sibling;
    size_t child_count;
};

static char *duplicate(const char *s)
{
    size_t len;
    char *copy;

    if (s == NULL) {
        return NULL;
    }
    len = strlen(s);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, len + 1);
    return copy;
}

static void detach(tree_node *node)
{
    tree_node *prev = NULL;
    tree_node *cur;

    if (node == NULL || node->parent == NULL) {
        return;
    }
    cur = node->parent->first_child;
    while (cur != NULL && cur != node) {
        prev = cur;
        cur = cur->next_sibling;
    }
    if (cur == NULL) {
        return;
    }
    if (prev == NULL) {
        node->parent->first_child = cur->next_sibling;
    } else {
        prev->next_sibling = cur->next_sibling;
    }
    if (node->parent->last_child == node) {
        node->parent->last_child = prev;
    }
    node->parent->child_count--;
    node->parent = NULL;
    node->next_sibling = NULL;
}

tree_node *gt_create(const char *data)
{
    tree_node *node = (tree_node *)calloc(1, sizeof(tree_node));

    if (node == NULL) {
        return NULL;
    }
    node->data = duplicate(data);
    if (data != NULL && node->data == NULL) {
        free(node);
        return NULL;
    }
    return node;
}

tree_node *gt_add_child(tree_node *parent, const char *data)
{
    tree_node *child;

    if (parent == NULL) {
        return NULL;
    }
    child = gt_create(data);
    if (child == NULL) {
        return NULL;
    }
    if (gt_attach(parent, child) != 1) {
        gt_free(child);
        return NULL;
    }
    return child;
}

int gt_attach(tree_node *parent, tree_node *child)
{
    if (parent == NULL || child == NULL || parent == child) {
        return 0;
    }
    detach(child);
    child->parent = parent;
    child->next_sibling = NULL;
    if (parent->last_child == NULL) {
        parent->first_child = child;
    } else {
        parent->last_child->next_sibling = child;
    }
    parent->last_child = child;
    parent->child_count++;
    return 1;
}

int gt_remove(tree_node *node)
{
    if (node == NULL || node->parent == NULL) {
        return 0;
    }
    detach(node);
    gt_free(node);
    return 1;
}

const char *gt_data(const tree_node *node)
{
    return node == NULL ? NULL : node->data;
}

tree_node *gt_parent(const tree_node *node)
{
    return node == NULL ? NULL : node->parent;
}

tree_node *gt_first_child(const tree_node *node)
{
    return node == NULL ? NULL : node->first_child;
}

tree_node *gt_next_sibling(const tree_node *node)
{
    return node == NULL ? NULL : node->next_sibling;
}

size_t gt_child_count(const tree_node *node)
{
    return node == NULL ? 0 : node->child_count;
}

int gt_is_leaf(const tree_node *node)
{
    return node != NULL && node->first_child == NULL;
}

int gt_is_root(const tree_node *node)
{
    return node != NULL && node->parent == NULL;
}

size_t gt_depth(const tree_node *node)
{
    size_t depth = 0;
    const tree_node *cur = node;

    while (cur != NULL && cur->parent != NULL) {
        depth++;
        cur = cur->parent;
    }
    return depth;
}

size_t gt_height(const tree_node *node)
{
    size_t best = 0;
    const tree_node *child;

    if (node == NULL) {
        return 0;
    }
    for (child = node->first_child; child != NULL; child = child->next_sibling) {
        size_t h = gt_height(child) + 1;
        if (h > best) {
            best = h;
        }
    }
    return best;
}

size_t gt_size(const tree_node *node)
{
    size_t total = 1;
    const tree_node *child;

    if (node == NULL) {
        return 0;
    }
    for (child = node->first_child; child != NULL; child = child->next_sibling) {
        total += gt_size(child);
    }
    return total;
}

size_t gt_leaf_count(const tree_node *node)
{
    size_t total = 0;
    const tree_node *child;

    if (node == NULL) {
        return 0;
    }
    if (node->first_child == NULL) {
        return 1;
    }
    for (child = node->first_child; child != NULL; child = child->next_sibling) {
        total += gt_leaf_count(child);
    }
    return total;
}

tree_node *gt_find(tree_node *root, const char *data)
{
    tree_node *child;

    if (root == NULL || data == NULL) {
        return NULL;
    }
    if (root->data != NULL && strcmp(root->data, data) == 0) {
        return root;
    }
    for (child = root->first_child; child != NULL; child = child->next_sibling) {
        tree_node *found = gt_find(child, data);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

size_t gt_preorder(const tree_node *root,
                   void (*visitor)(const tree_node *node, void *ctx), void *ctx)
{
    size_t count = 0;
    const tree_node *child;

    if (root == NULL) {
        return 0;
    }
    if (visitor != NULL) {
        visitor(root, ctx);
    }
    count = 1;
    for (child = root->first_child; child != NULL; child = child->next_sibling) {
        count += gt_preorder(child, visitor, ctx);
    }
    return count;
}

size_t gt_postorder(const tree_node *root,
                    void (*visitor)(const tree_node *node, void *ctx), void *ctx)
{
    size_t count = 0;
    const tree_node *child;

    if (root == NULL) {
        return 0;
    }
    for (child = root->first_child; child != NULL; child = child->next_sibling) {
        count += gt_postorder(child, visitor, ctx);
    }
    if (visitor != NULL) {
        visitor(root, ctx);
    }
    return count + 1;
}

size_t gt_levelorder(const tree_node *root,
                     void (*visitor)(const tree_node *node, void *ctx), void *ctx)
{
    size_t capacity;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    const tree_node **queue;
    const tree_node *cur;

    if (root == NULL) {
        return 0;
    }
    capacity = gt_size(root) + 1;
    queue = (const tree_node **)malloc(capacity * sizeof(*queue));
    if (queue == NULL) {
        return 0;
    }
    queue[tail++] = root;
    while (head < tail) {
        cur = queue[head++];
        count++;
        if (visitor != NULL) {
            visitor(cur, ctx);
        }
        for (const tree_node *child = cur->first_child; child != NULL;
             child = child->next_sibling) {
            if (tail < capacity) {
                queue[tail++] = child;
            }
        }
    }
    free(queue);
    return count;
}

static void print_rec(const tree_node *node, const char *prefix, int is_last)
{
    const tree_node *child;

    if (node == NULL) {
        return;
    }
    printf("%s%s%s\n", prefix, is_last ? "\\-- " : "|-- ",
           node->data == NULL ? "(null)" : node->data);
    for (child = node->first_child; child != NULL; child = child->next_sibling) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s%s", prefix, is_last ? "    " : "|   ");
        print_rec(child, buffer, child->next_sibling == NULL);
    }
}

void gt_print(const tree_node *root)
{
    print_rec(root, "", 1);
}

void gt_free(tree_node *node)
{
    tree_node *child;
    tree_node *next;

    if (node == NULL) {
        return;
    }
    child = node->first_child;
    while (child != NULL) {
        next = child->next_sibling;
        gt_free(child);
        child = next;
    }
    free(node->data);
    free(node);
}
