/*
 * Tests de la lib C arborikit. Aucun framework : assert + main.
 * Compilation : cc -Iinclude tests/test_arborikit.c src/general_tree.c src/bst.c src/rbtree.c -o /tmp/test_arborikit
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arborikit/bst.h"
#include "arborikit/general_tree.h"
#include "arborikit/rbtree.h"

static int failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "ECHEC %s:%d : %s\n", __FILE__, __LINE__, #cond);  \
            failures++;                                                        \
        }                                                                      \
    } while (0)

/* ------------------------- utilitaires ------------------------- */

static int collect_int(const void *data, void *ctx)
{
    int *array = (int *)ctx;
    array[0]++;
    array[array[0]] = (int)(intptr_t)data;
    return 0;
}

static char *dup_str(const char *s)
{
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, len + 1);
    return copy;
}

static void gcounter(const tree_node *node, void *ctx)
{
    (void)node;
    (*(int *)ctx)++;
}

/* ------------------------- GeneralTree ------------------------- */

static void test_general_tree(void)
{
    tree_node *root = gt_create("CEO");
    tree_node *vp1;
    tree_node *vp2;
    int count = 0;

    CHECK(root != NULL);
    vp1 = gt_add_child(root, "VP1");
    vp2 = gt_add_child(root, "VP2");
    CHECK(vp1 != NULL && vp2 != NULL);
    CHECK(gt_add_child(vp1, "GM1") != NULL);
    CHECK(gt_add_child(vp1, "GM2") != NULL);
    CHECK(gt_add_child(vp2, "GM3") != NULL);

    CHECK(gt_size(root) == 6);
    CHECK(gt_height(root) == 2);
    CHECK(gt_leaf_count(root) == 3);
    CHECK(gt_child_count(root) == 2);
    CHECK(strcmp(gt_data(root), "CEO") == 0);
    CHECK(gt_is_root(root) == 1);
    CHECK(gt_is_leaf(vp1) == 0);
    CHECK(gt_depth(gt_find(root, "GM2")) == 2);
    CHECK(gt_find(root, "inconnu") == NULL);
    CHECK(gt_parent(vp1) == root);

    gt_preorder(root, gcounter, &count);
    CHECK(count == 6);
    count = 0;
    gt_postorder(root, gcounter, &count);
    CHECK(count == 6);
    count = 0;
    gt_levelorder(root, gcounter, &count);
    CHECK(count == 6);

    /* détachement puis rattachement */
    CHECK(gt_remove(vp2) == 1);
    CHECK(gt_size(root) == 4); /* CEO, VP1, GM1, GM2 */
    CHECK(gt_remove(root) == 0);
    CHECK(gt_attach(vp1, vp1) == 0);
    gt_free(root);
}

/* ------------------------------ BST ------------------------------ */

static void test_bst(void)
{
    bst *tree = bst_create(NULL);
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    size_t i;
    intptr_t expected[] = {20, 30, 40, 50, 60, 70, 80};

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        CHECK(bst_insert(tree, (void *)(intptr_t)values[i]) == 1);
    }
    CHECK(bst_insert(tree, (void *)(intptr_t)50) == 0); /* doublon */
    CHECK(bst_size(tree) == 7);
    CHECK(bst_height(tree) == 3);
    CHECK(bst_is_degenerate(tree) == 0);
    CHECK(bst_validate(tree) == 1);

    CHECK((intptr_t)bst_min(tree) == 20);
    CHECK((intptr_t)bst_max(tree) == 80);
    CHECK((intptr_t)bst_successor(tree, (void *)(intptr_t)50) == 60);
    CHECK((intptr_t)bst_predecessor(tree, (void *)(intptr_t)50) == 40);
    CHECK(bst_successor(tree, (void *)(intptr_t)80) == NULL);
    CHECK(bst_contains(tree, (void *)(intptr_t)40) == 1);
    CHECK(bst_contains(tree, (void *)(intptr_t)99) == 0);

    {
        size_t n = bst_inorder(tree, (bst_visit_fn)NULL, NULL);
        CHECK(n == 7);
    }
    {
        int arr[16];
        arr[0] = 0;
        bst_inorder(tree, collect_int, arr);
        CHECK(arr[0] == 7);
        for (i = 0; i < 7; i++) {
            CHECK((intptr_t)expected[i] == arr[i + 1]);
        }
    }

    CHECK(bst_delete(tree, (void *)(intptr_t)20) == 1); /* feuille */
    CHECK(bst_delete(tree, (void *)(intptr_t)30) == 1); /* un enfant */
    CHECK(bst_delete(tree, (void *)(intptr_t)50) == 1); /* deux enfants */
    CHECK(bst_delete(tree, (void *)(intptr_t)999) == 0);
    CHECK(bst_size(tree) == 4);
    CHECK(bst_validate(tree) == 1);
    bst_destroy(tree);

    /* dégénérescence */
    tree = bst_create(NULL);
    for (i = 1; i <= 10; i++) {
        bst_insert(tree, (void *)(intptr_t)i);
    }
    CHECK(bst_height(tree) == 10);
    CHECK(bst_is_degenerate(tree) == 1);
    bst_destroy(tree);

    /* comparateur chaînes + destructeur */
    {
        bst *s = bst_create_ex(bst_compare_str, free);
        CHECK(bst_insert(s, dup_str("pomme")) == 1);
        CHECK(bst_insert(s, dup_str("abricot")) == 1);
        CHECK(bst_contains(s, "pomme") == 1);
        CHECK(bst_contains(s, "kiwi") == 0);
        CHECK(bst_delete(s, "pomme") == 1); /* le destructeur libère la chaîne */
        CHECK(bst_contains(s, "pomme") == 0);
        bst_destroy(s);
    }
}

/* ----------------------------- RBTree ---------------------------- */

static void test_rbtree(void)
{
    rb_tree *tree = rb_create(NULL);
    int values[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};
    size_t i;
    char err[128];

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        CHECK(rb_insert(tree, (void *)(intptr_t)values[i]) == 1);
        CHECK(rb_validate(tree, err, sizeof(err)) == 1);
    }
    CHECK(rb_insert(tree, (void *)(intptr_t)50) == 0);
    CHECK(rb_size(tree) == 11);
    CHECK((intptr_t)rb_min(tree) == 10);
    CHECK((intptr_t)rb_max(tree) == 80);
    CHECK((intptr_t)rb_successor(tree, (void *)(intptr_t)45) == 50);
    CHECK((intptr_t)rb_predecessor(tree, (void *)(intptr_t)45) == 40);
    CHECK(rb_black_height(tree) >= 2);
    CHECK(rb_count_color(tree, RB_BLACK) + rb_count_color(tree, RB_RED) == 11);

    /* accès bas niveau exposé */
    CHECK(rb_root(tree) != NULL);
    CHECK(rb_node_color(rb_root(tree)) == RB_BLACK);
    CHECK(rb_node_parent(rb_root(tree)) == NULL);

    CHECK(rb_delete(tree, (void *)(intptr_t)10) == 1);
    CHECK(rb_validate(tree, err, sizeof(err)) == 1);
    CHECK(rb_delete(tree, (void *)(intptr_t)70) == 1);
    CHECK(rb_validate(tree, err, sizeof(err)) == 1);
    CHECK(rb_delete(tree, (void *)(intptr_t)50) == 1);
    CHECK(rb_validate(tree, err, sizeof(err)) == 1);
    CHECK(rb_delete(tree, (void *)(intptr_t)999) == 0);
    CHECK(rb_size(tree) == 8);

    {
        int arr[32];
        size_t n;
        arr[0] = 0;
        n = rb_inorder(tree, collect_int, arr);
        CHECK(n == 8);
        CHECK(arr[1] == 20 && arr[8] == 80);
        for (i = 1; i < 8; i++) {
            CHECK(arr[i] < arr[i + 1]);
        }
    }
    rb_destroy(tree);

    /* insertions triées : l'ARN doit rester équilibré */
    tree = rb_create(NULL);
    for (i = 1; i <= 50; i++) {
        rb_insert(tree, (void *)(intptr_t)i);
    }
    CHECK(rb_validate(tree, err, sizeof(err)) == 1);
    CHECK(rb_height(tree) <= 12);
    CHECK(rb_height(tree) > 0);
    rb_destroy(tree);

    /* stress aléatoire */
    {
        rb_tree *s = rb_create(NULL);
        bst *ref = bst_create(NULL);
        unsigned seed = 12345u;
        int k;
        for (k = 0; k < 5000; k++) {
            int v;
            seed = seed * 1103515245u + 12345u;
            v = (int)((seed >> 8) % 500);
            if ((seed >> 20) % 2u == 0) {
                rb_insert(s, (void *)(intptr_t)v);
                bst_insert(ref, (void *)(intptr_t)v);
            } else {
                rb_delete(s, (void *)(intptr_t)v);
                bst_delete(ref, (void *)(intptr_t)v);
            }
            if (k % 500 == 0) {
                CHECK(rb_validate(s, err, sizeof(err)) == 1);
            }
        }
        CHECK(rb_validate(s, err, sizeof(err)) == 1);
        CHECK(rb_size(s) == bst_size(ref));
        rb_destroy(s);
        bst_destroy(ref);
    }
}

int main(void)
{
    printf("arborikit C — tests\n");
    test_general_tree();
    test_bst();
    test_rbtree();
    if (failures == 0) {
        printf("Tous les tests sont passes.\n");
        return 0;
    }
    fprintf(stderr, "%d echec(s)\n", failures);
    return 1;
}
