/*
 * arborikit/general_tree.h — arbre généraliste (n-aire) en C99.
 *
 * Aucune dépendance hors libc. Le nœud possède un parent et une liste
 * ordonnée d'enfants (liste simplement chaînée pour rester léger).
 *
 * Usage typique :
 *
 *     tree_node *root = gt_create("CEO");
 *     tree_node *vp1  = gt_add_child(root, "VP1");
 *     gt_add_child(vp1, "GM1");
 *     gt_print(root);
 *     gt_free(root);
 */
#ifndef ARBORIKIT_GENERAL_TREE_H
#define ARBORIKIT_GENERAL_TREE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tree_node tree_node;

/** Crée la racine. Retourne NULL si l'allocation échoue. */
tree_node *gt_create(const char *data);

/** Ajoute un enfant (données copiées). Retourne l'enfant, ou NULL. */
tree_node *gt_add_child(tree_node *parent, const char *data);

/** Rattache un nœud existant (le détache d'abord de son parent actuel). */
int gt_attach(tree_node *parent, tree_node *child);

/** Supprime le nœud et tout son sous-arbre. Retourne 1 si supprimé. */
int gt_remove(tree_node *node);

/** Données du nœud (jamais NULL pour un nœud créé par gt_create). */
const char *gt_data(const tree_node *node);

/** Parent, ou NULL pour la racine. */
tree_node *gt_parent(const tree_node *node);

/** Premier enfant, ou NULL si feuille. */
tree_node *gt_first_child(const tree_node *node);

/** Frère suivant, ou NULL. */
tree_node *gt_next_sibling(const tree_node *node);

/** Nombre d'enfants directs. */
size_t gt_child_count(const tree_node *node);

/** 1 si le nœud n'a aucun enfant. */
int gt_is_leaf(const tree_node *node);

/** 1 si le nœud est la racine. */
int gt_is_root(const tree_node *node);

/** Profondeur : racine = 0. */
size_t gt_depth(const tree_node *node);

/** Hauteur du sous-arbre : feuille = 0. */
size_t gt_height(const tree_node *node);

/** Nombre de nœuds du sous-arbre, nœud inclus. */
size_t gt_size(const tree_node *node);

/** Premier nœud dont les données valent `data` (comparaison strcmp), ou NULL. */
tree_node *gt_find(tree_node *root, const char *data);

/** Nombre de feuilles du sous-arbre. */
size_t gt_leaf_count(const tree_node *node);

/**
 * Parcours préordre (nœud puis enfants). Appelle `visitor` sur chaque nœud.
 * `ctx` est passé tel quel au visiteur. Retourne le nombre de nœuds visités.
 */
size_t gt_preorder(const tree_node *root,
                   void (*visitor)(const tree_node *node, void *ctx), void *ctx);

/** Parcours postordre (enfants puis nœud). */
size_t gt_postorder(const tree_node *root,
                    void (*visitor)(const tree_node *node, void *ctx), void *ctx);

/**
 * Parcours en largeur. Alloue une file temporaire ; retourne 0 si
 * l'allocation échoue.
 */
size_t gt_levelorder(const tree_node *root,
                     void (*visitor)(const tree_node *node, void *ctx), void *ctx);

/** Affiche l'arbre sur stdout. */
void gt_print(const tree_node *root);

/** Libère le nœud et tout son sous-arbre. NULL est accepté. */
void gt_free(tree_node *node);

#ifdef __cplusplus
}
#endif

#endif /* ARBORIKIT_GENERAL_TREE_H */
