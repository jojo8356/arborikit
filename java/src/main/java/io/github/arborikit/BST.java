package io.github.arborikit;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;

/**
 * BST — arbre binaire de recherche NON équilibré (ABR).
 *
 * <p>Cette structure n'existe pas dans le JDK : {@code java.util.TreeMap} et
 * {@code TreeSet} sont des arbres rouge-noir équilibrés. Un ABR non équilibré
 * est indispensable pour observer la dégénérescence en O(n) et pour
 * l'enseignement des rotations.
 *
 * <p>Propriété : sous-arbre gauche &lt; nœud &lt; sous-arbre droit.
 * Les doublons sont ignorés.
 *
 * @param <T> type comparable
 */
public final class BST<T extends Comparable<? super T>> {

    /** Nœud de l'ABR, exposé en lecture seule. */
    public static final class Node<T> {
        private T key;
        private Node<T> left;
        private Node<T> right;

        Node(T key) { this.key = key; }

        public T getKey() { return key; }

        /** Mutateur réservé aux tests (corruption volontaire de l'invariant). */
        void setKeyForTest(T newKey) { this.key = newKey; }

        /** Sous-arbre gauche, ou {@code null}. */
        public Node<T> getLeft() { return left; }

        /** Sous-arbre droit, ou {@code null}. */
        public Node<T> getRight() { return right; }

        @Override
        public String toString() { return String.valueOf(key); }
    }

    private Node<T> root;
    private int size;

    public BST() { }

    @SafeVarargs
    public BST(T... items) {
        for (T item : items) {
            insert(item);
        }
    }

    public boolean isEmpty() { return root == null; }

    public int size() { return size; }

    public boolean contains(T key) { return find(key) != null; }

    /** Racine de l'arbre, ou {@code null} si vide. */
    public Node<T> getRoot() { return root; }

    // ------------------------ insertion -------------------------
    /** Insère {@code key}. Retourne {@code false} si déjà présent. */
    public boolean insert(T key) {
        if (key == null) {
            throw new IllegalArgumentException("clé null interdite");
        }
        if (root == null) {
            root = new Node<>(key);
            size = 1;
            return true;
        }
        Node<T> current = root;
        while (true) {
            int cmp = key.compareTo(current.key);
            if (cmp == 0) {
                return false;
            }
            if (cmp < 0) {
                if (current.left == null) {
                    current.left = new Node<>(key);
                    size++;
                    return true;
                }
                current = current.left;
            } else {
                if (current.right == null) {
                    current.right = new Node<>(key);
                    size++;
                    return true;
                }
                current = current.right;
            }
        }
    }

    // ------------------------ recherche -------------------------
    /** Nœud contenant la clé, ou {@code null}. */
    public Node<T> find(T key) {
        Node<T> current = root;
        while (current != null) {
            int cmp = key.compareTo(current.key);
            if (cmp == 0) {
                return current;
            }
            current = (cmp < 0) ? current.left : current.right;
        }
        return null;
    }

    public T min() {
        if (root == null) {
            throw new IllegalStateException("arbre vide");
        }
        return minNode(root).key;
    }

    public T max() {
        if (root == null) {
            throw new IllegalStateException("arbre vide");
        }
        return maxNode(root).key;
    }

    private Node<T> minNode(Node<T> node) {
        while (node.left != null) {
            node = node.left;
        }
        return node;
    }

    private Node<T> maxNode(Node<T> node) {
        while (node.right != null) {
            node = node.right;
        }
        return node;
    }

    /** Plus petit élément strictement supérieur à {@code key}, ou {@code null}. */
    public T successor(T key) {
        Node<T> current = root;
        T best = null;
        while (current != null) {
            int cmp = key.compareTo(current.key);
            if (cmp < 0) {
                best = current.key;
                current = current.left;
            } else if (cmp > 0) {
                current = current.right;
            } else {
                if (current.right != null) {
                    return minNode(current.right).key;
                }
                return best;
            }
        }
        return best;
    }

    /** Plus grand élément strictement inférieur à {@code key}, ou {@code null}. */
    public T predecessor(T key) {
        Node<T> current = root;
        T best = null;
        while (current != null) {
            int cmp = key.compareTo(current.key);
            if (cmp > 0) {
                best = current.key;
                current = current.right;
            } else if (cmp < 0) {
                current = current.left;
            } else {
                if (current.left != null) {
                    return maxNode(current.left).key;
                }
                return best;
            }
        }
        return best;
    }

    // ------------------------ suppression -----------------------
    /**
     * Supprime {@code key}. Trois cas : feuille, un enfant, deux enfants
     * (remplacement par le successeur).
     */
    public boolean delete(T key) {
        int before = size;
        root = deleteRec(root, key);
        return size < before;
    }

    private Node<T> deleteRec(Node<T> node, T key) {
        if (node == null) {
            return null;
        }
        int cmp = key.compareTo(node.key);
        if (cmp < 0) {
            node.left = deleteRec(node.left, key);
        } else if (cmp > 0) {
            node.right = deleteRec(node.right, key);
        } else {
            size--;
            if (node.left == null) {
                return node.right;
            }
            if (node.right == null) {
                return node.left;
            }
            Node<T> next = minNode(node.right);
            node.key = next.key;
            node.right = deleteRec(node.right, next.key);
            size++; // compense le décrément du sous-appel
        }
        return node;
    }

    public void clear() {
        root = null;
        size = 0;
    }

    // ------------------------ parcours --------------------------
    /** Ordre croissant. */
    public List<T> inorder() {
        List<T> out = new ArrayList<>();
        Deque<Node<T>> stack = new ArrayDeque<>();
        Node<T> current = root;
        while (!stack.isEmpty() || current != null) {
            while (current != null) {
                stack.push(current);
                current = current.left;
            }
            current = stack.pop();
            out.add(current.key);
            current = current.right;
        }
        return out;
    }

    public List<T> preorder() {
        List<T> out = new ArrayList<>();
        if (root == null) {
            return out;
        }
        Deque<Node<T>> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node<T> node = stack.pop();
            out.add(node.key);
            if (node.right != null) {
                stack.push(node.right);
            }
            if (node.left != null) {
                stack.push(node.left);
            }
        }
        return out;
    }

    public List<T> postorder() {
        List<T> out = new ArrayList<>();
        if (root == null) {
            return out;
        }
        Deque<Node<T>> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node<T> node = stack.pop();
            out.add(node.key);
            if (node.left != null) {
                stack.push(node.left);
            }
            if (node.right != null) {
                stack.push(node.right);
            }
        }
        java.util.Collections.reverse(out);
        return out;
    }

    public List<T> levelorder() {
        List<T> out = new ArrayList<>();
        if (root == null) {
            return out;
        }
        Deque<Node<T>> queue = new ArrayDeque<>();
        queue.add(root);
        while (!queue.isEmpty()) {
            Node<T> node = queue.poll();
            out.add(node.key);
            if (node.left != null) {
                queue.add(node.left);
            }
            if (node.right != null) {
                queue.add(node.right);
            }
        }
        return out;
    }

    // ------------------------- mesures --------------------------
    /** Hauteur : arbre vide = -1, un nœud = 0. */
    public int height() {
        if (root == null) {
            return -1;
        }
        int best = 0;
        Deque<Node<T>> nodes = new ArrayDeque<>();
        Deque<Integer> depths = new ArrayDeque<>();
        nodes.push(root);
        depths.push(0);
        while (!nodes.isEmpty()) {
            Node<T> node = nodes.pop();
            int depth = depths.pop();
            best = Math.max(best, depth);
            if (node.left != null) {
                nodes.push(node.left);
                depths.push(depth + 1);
            }
            if (node.right != null) {
                nodes.push(node.right);
                depths.push(depth + 1);
            }
        }
        return best;
    }

    /** Vrai si l'arbre est dégénéré en liste (hauteur == size - 1). */
    public boolean isDegenerate() {
        return size > 0 && height() == size - 1;
    }

    // ---------------------- vérification ------------------------
    /** Lève {@link IllegalStateException} si la propriété ABR est violée. */
    public void validate() {
        validateRec(root, null, null);
    }

    private void validateRec(Node<T> node, T lo, T hi) {
        if (node == null) {
            return;
        }
        if (lo != null && node.key.compareTo(lo) <= 0) {
            throw new IllegalStateException("ABR violé : " + node.key + " <= " + lo);
        }
        if (hi != null && node.key.compareTo(hi) >= 0) {
            throw new IllegalStateException("ABR violé : " + node.key + " >= " + hi);
        }
        validateRec(node.left, lo, node.key);
        validateRec(node.right, node.key, hi);
    }

    // ------------------------ affichage -------------------------
    @Override
    public String toString() {
        List<T> ordered = inorder();
        StringBuilder sb = new StringBuilder("BST").append(ordered);
        return sb.toString();
    }

    /** Représentation arborescente indentée. */
    public String toTreeString() {
        StringBuilder sb = new StringBuilder();
        printRec(root, "", true, sb);
        return sb.toString();
    }

    private void printRec(Node<T> node, String prefix, boolean isLast, StringBuilder sb) {
        if (node == null) {
            return;
        }
        sb.append(prefix).append(isLast ? "└── " : "├── ").append(node.key).append('\n');
        String childPrefix = prefix + (isLast ? "    " : "│   ");
        if (node.left != null && node.right != null) {
            printRec(node.left, childPrefix, false, sb);
            printRec(node.right, childPrefix, true, sb);
        } else if (node.left != null) {
            printRec(node.left, childPrefix, true, sb);
        } else if (node.right != null) {
            printRec(node.right, childPrefix, true, sb);
        }
    }
}
