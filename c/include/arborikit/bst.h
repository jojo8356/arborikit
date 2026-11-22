/*
 * arborikit/bst.h — arbre binaire de recherche NON équilibré (ABR), en C99.
 *
 * Données de type `void *` : la lib ne dépend d'aucun type métier.
 * La comparaison se fait via un pointeur de fonction `bst_cmp_fn`.
 *
 * Par défaut (`bst_compare_int`), les éléments sont comparés comme des
 * entiers via un cast `intptr_t` : on peut donc stocker des petits entiers
 * directement dans le pointeur, sans allocation.
 *
 * Attention : un ABR non équilibré dégénère en liste (O(n)) si les
 * insertions sont triées. C'est voulu : c'est une structure pédagogique.
 * Pour de la production, utilisez `rbtree.h`.
 */
#ifndef ARBORIKIT_BST_H
#define ARBORIKIT_BST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bst bst;
typedef struct bst_node bst_node;

/** Comparateur : <0, 0, >0. */
typedef int (*bst_cmp_fn)(const void *a, const void *b);

/** Visiteur de parcours. Retourner 0 interrompt le parcours. */
typedef int (*bst_visit_fn)(const void *data, void *ctx);

/** Comparateur par défaut : entiers rangés dans le pointeur. */
int bst_compare_int(const void *a, const void *b);

/** Comparateur par défaut : chaînes C (`strcmp`). */
int bst_compare_str(const void *a, const void *b);

/** Crée un ABR. `cmp` NULL -> `bst_compare_int`. */
bst *bst_create(bst_cmp_fn cmp);

/**
 * Crée un ABR avec un destructeur appelé sur chaque donnée à la suppression
 * et dans `bst_destroy`. Passer NULL si les données ne sont pas possédées.
 */
bst *bst_create_ex(bst_cmp_fn cmp, void (*free_fn)(void *data));

/**
 * Insère `data`. Retourne 1 si inséré, 0 si doublon ou erreur.
 */
int bst_insert(bst *tree, void *data);

/** Retourne la donnée égale à `data`, ou NULL. */
void *bst_find(const bst *tree, const void *data);

/** Retourne 1 si `data` est présent. */
int bst_contains(const bst *tree, const void *data);

/** Supprime `data`. Retourne 1 si supprimé. */
int bst_delete(bst *tree, void *data);

/** Plus petit élément, ou NULL si vide. */
void *bst_min(const bst *tree);

/** Plus grand élément, ou NULL si vide. */
void *bst_max(const bst *tree);

/** Successeur strict, ou NULL. */
void *bst_successor(const bst *tree, const void *data);

/** Prédécesseur strict, ou NULL. */
void *bst_predecessor(const bst *tree, const void *data);

/** Nombre d'éléments. */
size_t bst_size(const bst *tree);

/** 1 si l'arbre est vide. */
int bst_is_empty(const bst *tree);

/** Hauteur : arbre vide = 0, un nœud = 1. */
size_t bst_height(const bst *tree);

/** 1 si l'arbre est dégénéré en liste (hauteur == size). */
int bst_is_degenerate(const bst *tree);

/** Parcours infixe (ordre croissant). Retourne le nombre d'éléments visités. */
size_t bst_inorder(const bst *tree, bst_visit_fn visit, void *ctx);

/** Parcours préfixe. */
size_t bst_preorder(const bst *tree, bst_visit_fn visit, void *ctx);

/** Parcours postfixe. */
size_t bst_postorder(const bst *tree, bst_visit_fn visit, void *ctx);

/** Parcours en largeur. */
size_t bst_levelorder(const bst *tree, bst_visit_fn visit, void *ctx);

/**
 * Vérifie la propriété ABR. Retourne 1 si valide, 0 sinon.
 * (En C, contrairement à Python/Java, pas d'exception : code retour.)
 */
int bst_validate(const bst *tree);

/** Affiche l'arbre sur stdout (entiers uniquement, pratique pour déboguer). */
void bst_print_int(const bst *tree);

/** Libère tout. NULL accepté. */
void bst_destroy(bst *tree);

#ifdef __cplusplus
}
#endif

#endif /* ARBORIKIT_BST_H */
