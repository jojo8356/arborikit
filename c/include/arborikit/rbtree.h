/*
 * arborikit/rbtree.h — arbre rouge-noir (ARN) complet, en C99.
 *
 * Implémentation CLRS avec sentinelle. Comparateur générique (`void *`),
 * donc utilisable pour des entiers, des chaînes, des structs...
 *
 * Aucune dépendance hors libc. Aucune allocation cachée (sauf les nœuds).
 *
 * Cinq invariants vérifiables par `rb_validate()`.
 */
#ifndef ARBORIKIT_RBTREE_H
#define ARBORIKIT_RBTREE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rb_tree rb_tree;
typedef struct rb_node rb_node;

/** Couleur d'un nœud. */
typedef enum { RB_RED = 0, RB_BLACK = 1 } rb_color;

/** Comparateur : <0, 0, >0. */
typedef int (*rb_cmp_fn)(const void *a, const void *b);

/** Visiteur : retourner 0 interrompt le parcours. */
typedef int (*rb_visit_fn)(const void *data, void *ctx);

/** Comparateur par défaut : entiers rangés dans le pointeur. */
int rb_compare_int(const void *a, const void *b);

/** Comparateur par défaut : chaînes C. */
int rb_compare_str(const void *a, const void *b);

/** Crée un ARN. `cmp` NULL -> `rb_compare_int`. */
rb_tree *rb_create(rb_cmp_fn cmp);

/** Variante avec destructeur de données. */
rb_tree *rb_create_ex(rb_cmp_fn cmp, void (*free_fn)(void *data));

/** Insère `data`. 1 si inséré, 0 si doublon ou échec d'allocation. */
int rb_insert(rb_tree *tree, void *data);

/** Donnée égale à `data`, ou NULL. */
void *rb_find(const rb_tree *tree, const void *data);

/** 1 si présent. */
int rb_contains(const rb_tree *tree, const void *data);

/** Supprime `data`. 1 si supprimé. */
int rb_delete(rb_tree *tree, void *data);

/** Plus petit / plus grand élément, ou NULL. */
void *rb_min(const rb_tree *tree);
void *rb_max(const rb_tree *tree);

/** Successeur / prédécesseur strict, ou NULL. */
void *rb_successor(const rb_tree *tree, const void *data);
void *rb_predecessor(const rb_tree *tree, const void *data);

/** Nombre d'éléments. */
size_t rb_size(const rb_tree *tree);

/** 1 si vide. */
int rb_is_empty(const rb_tree *tree);

/** Hauteur (nombre d'arêtes) : arbre vide -> 0. */
size_t rb_height(const rb_tree *tree);

/** Hauteur noire : nœuds noirs sur un chemin racine → feuille. */
size_t rb_black_height(const rb_tree *tree);

/** Parcours : infixe (croissant), préfixe, postfixe, largeur. */
size_t rb_inorder(const rb_tree *tree, rb_visit_fn visit, void *ctx);
size_t rb_preorder(const rb_tree *tree, rb_visit_fn visit, void *ctx);
size_t rb_postorder(const rb_tree *tree, rb_visit_fn visit, void *ctx);
size_t rb_levelorder(const rb_tree *tree, rb_visit_fn visit, void *ctx);

/** Nombre de nœuds rouges / noirs (diagnostic, tests). */
size_t rb_count_color(const rb_tree *tree, rb_color color);

/**
 * Vérifie les cinq invariants + la propriété ABR.
 * Retourne 1 si tout est valide, 0 sinon. Écrit le motif dans `err`
 * si `err` n'est pas NULL et `err_size` > 0.
 */
int rb_validate(const rb_tree *tree, char *err, size_t err_size);

/** Affiche l'arbre sur stdout (entiers), couleurs R/N. */
void rb_print_int(const rb_tree *tree);

/** Libère tout. NULL accepté. */
void rb_destroy(rb_tree *tree);

/* ------------------------------------------------------------------ */
/* Accès bas niveau (lecture seule) — pour l'enseignement et les tests */
/* ------------------------------------------------------------------ */

/** Racine réelle, ou NULL si l'arbre est vide. */
rb_node *rb_root(const rb_tree *tree);

/** Donnée du nœud. */
const void *rb_node_data(const rb_node *node);

/** Couleur du nœud. */
rb_color rb_node_color(const rb_node *node);

/** Enfant gauche, ou NULL. */
rb_node *rb_node_left(const rb_node *node);

/** Enfant droit, ou NULL. */
rb_node *rb_node_right(const rb_node *node);

/** Parent, ou NULL pour la racine. */
rb_node *rb_node_parent(const rb_node *node);

#ifdef __cplusplus
}
#endif

#endif /* ARBORIKIT_RBTREE_H */
