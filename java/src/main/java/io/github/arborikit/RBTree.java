package io.github.arborikit;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.List;

/**
 * RBTree — arbre rouge-noir (ARN) complet, observable et vérifiable.
 *
 * <p>{@code java.util.TreeMap} est déjà un ARN, mais sa classe interne
 * {@code Entry} est privée : impossible de lire une couleur, de voir une
 * rotation, de dessiner l'arbre ou de vérifier les invariants. Cette classe
 * expose tout, ce qui est nécessaire pour l'enseignement et les tests.
 *
 * <p>Implémentation CLRS avec sentinelle {@code NIL}.
 *
 * <p>Cinq invariants vérifiables par {@link #validate()} :
 * <ol>
 *   <li>chaque nœud est rouge ou noir ;</li>
 *   <li>la racine est noire ;</li>
 *   <li>toute feuille (NIL) est noire ;</li>
 *   <li>un nœud rouge a deux enfants noirs ;</li>
 *   <li>tout chemin racine → feuille compte le même nombre de nœuds noirs.</li>
 * </ol>
 *
 * @param <T> type comparable
 */
public final class RBTree<T extends Comparable<? super T>> {

    /** Couleur d'un nœud. */
    public enum Color {
        /** Nœud rouge. */
        RED,
        /** Nœud noir. */
        BLACK
    }

    /** Nœud rouge-noir exposé. */
    public final class Node {
        private T key;
        private Node left;
        private Node right;
        private Node parent;
        private Color color;

        Node(T key, Color color) {
            this.key = key;
            this.color = color;
        }

        public T getKey() { return key; }

        public Color getColor() { return color; }

        public boolean isRed() { return color == Color.RED; }

        public boolean isBlack() { return color == Color.BLACK; }

        /** Enfant gauche, ou {@code null} pour une feuille. */
        public Node getLeft() { return left == nil ? null : left; }

        /** Enfant droit, ou {@code null} pour une feuille. */
        public Node getRight() { return right == nil ? null : right; }

        public Node getParent() { return parent; }

        @Override
        public String toString() {
            return key + (color == Color.RED ? "(R)" : "(N)");
        }
    }

    private final Node nil;
    private Node root;
    private int size;

    public RBTree() {
        nil = new Node(null, Color.BLACK);
        nil.left = nil;
        nil.right = nil;
        nil.parent = null;
        root = nil;
    }

    @SafeVarargs
    public RBTree(T... items) {
        this();
        for (T item : items) {
            insert(item);
        }
    }

    public boolean isEmpty() { return root == nil; }

    public int size() { return size; }

    public boolean contains(T key) { return find(key) != null; }

    /** Racine exposée, ou {@code null} si vide. */
    public Node getRoot() { return root == nil ? null : root; }

    // ------------------------- rotations ------------------------
    private void leftRotate(Node x) {
        Node y = x.right;
        x.right = y.left;
        if (y.left != nil) {
            y.left.parent = x;
        }
        y.parent = x.parent;
        if (x.parent == null) {
            root = y;
        } else if (x == x.parent.left) {
            x.parent.left = y;
        } else {
            x.parent.right = y;
        }
        y.left = x;
        x.parent = y;
    }

    private void rightRotate(Node x) {
        Node y = x.left;
        x.left = y.right;
        if (y.right != nil) {
            y.right.parent = x;
        }
        y.parent = x.parent;
        if (x.parent == null) {
            root = y;
        } else if (x == x.parent.right) {
            x.parent.right = y;
        } else {
            x.parent.left = y;
        }
        y.right = x;
        x.parent = y;
    }

    // ------------------------ insertion -------------------------
    /** Insère {@code key}. Retourne {@code false} si déjà présent. */
    public boolean insert(T key) {
        if (key == null) {
            throw new IllegalArgumentException("clé null interdite");
        }
        Node y = null;
        Node x = root;
        while (x != nil) {
            y = x;
            int cmp = key.compareTo(x.key);
            if (cmp == 0) {
                return false;
            }
            x = (cmp < 0) ? x.left : x.right;
        }
        Node z = new Node(key, Color.RED);
        z.left = z.right = nil;
        z.parent = y;
        if (y == null) {
            root = z;
        } else if (key.compareTo(y.key) < 0) {
            y.left = z;
        } else {
            y.right = z;
        }
        size++;
        insertFixup(z);
        return true;
    }

    private void insertFixup(Node z) {
        while (z.parent != null && z.parent.color == Color.RED) {
            Node grandParent = z.parent.parent;
            if (z.parent == grandParent.left) {
                Node uncle = grandParent.right;
                if (uncle.color == Color.RED) {                 // cas 1
                    z.parent.color = Color.BLACK;
                    uncle.color = Color.BLACK;
                    grandParent.color = Color.RED;
                    z = grandParent;
                } else {
                    if (z == z.parent.right) {                  // cas 2
                        z = z.parent;
                        leftRotate(z);
                    }
                    z.parent.color = Color.BLACK;               // cas 3
                    z.parent.parent.color = Color.RED;
                    rightRotate(z.parent.parent);
                }
            } else {
                Node uncle = grandParent.left;
                if (uncle.color == Color.RED) {
                    z.parent.color = Color.BLACK;
                    uncle.color = Color.BLACK;
                    grandParent.color = Color.RED;
                    z = grandParent;
                } else {
                    if (z == z.parent.left) {
                        z = z.parent;
                        rightRotate(z);
                    }
                    z.parent.color = Color.BLACK;
                    z.parent.parent.color = Color.RED;
                    leftRotate(z.parent.parent);
                }
            }
        }
        root.color = Color.BLACK;
    }

    // ------------------------ recherche -------------------------
    /** Nœud contenant la clé, ou {@code null}. */
    public Node find(T key) {
        Node x = root;
        while (x != nil) {
            int cmp = key.compareTo(x.key);
            if (cmp == 0) {
                return x;
            }
            x = (cmp < 0) ? x.left : x.right;
        }
        return null;
    }

    public T min() {
        if (isEmpty()) {
            throw new IllegalStateException("arbre vide");
        }
        return minimum(root).key;
    }

    public T max() {
        if (isEmpty()) {
            throw new IllegalStateException("arbre vide");
        }
        return maximum(root).key;
    }

    private Node minimum(Node x) {
        while (x.left != nil) {
            x = x.left;
        }
        return x;
    }

    private Node maximum(Node x) {
        while (x.right != nil) {
            x = x.right;
        }
        return x;
    }

    /** Plus petit élément strictement supérieur à {@code key}. */
    public T successor(T key) {
        Node x = find(key);
        if (x == null) {
            return null;
        }
        if (x.right != nil) {
            return minimum(x.right).key;
        }
        Node y = x.parent;
        while (y != null && x == y.right) {
            x = y;
            y = y.parent;
        }
        return y == null ? null : y.key;
    }

    /** Plus grand élément strictement inférieur à {@code key}. */
    public T predecessor(T key) {
        Node x = find(key);
        if (x == null) {
            return null;
        }
        if (x.left != nil) {
            return maximum(x.left).key;
        }
        Node y = x.parent;
        while (y != null && x == y.left) {
            x = y;
            y = y.parent;
        }
        return y == null ? null : y.key;
    }

    // ----------------------- suppression ------------------------
    public boolean delete(T key) {
        Node z = find(key);
        if (z == null) {
            return false;
        }
        deleteNode(z);
        size--;
        return true;
    }

    private void transplant(Node u, Node v) {
        if (u.parent == null) {
            root = v;
        } else if (u == u.parent.left) {
            u.parent.left = v;
        } else {
            u.parent.right = v;
        }
        v.parent = u.parent;
    }

    private void deleteNode(Node z) {
        Node y = z;
        Color yOriginalColor = y.color;
        Node x;
        if (z.left == nil) {
            x = z.right;
            transplant(z, z.right);
        } else if (z.right == nil) {
            x = z.left;
            transplant(z, z.left);
        } else {
            y = minimum(z.right);
            yOriginalColor = y.color;
            x = y.right;
            if (y.parent == z) {
                x.parent = y;
            } else {
                transplant(y, y.right);
                y.right = z.right;
                y.right.parent = y;
            }
            transplant(z, y);
            y.left = z.left;
            y.left.parent = y;
            y.color = z.color;
        }
        if (yOriginalColor == Color.BLACK) {
            deleteFixup(x);
        }
    }

    private void deleteFixup(Node x) {
        while (x != root && x.color == Color.BLACK) {
            if (x == x.parent.left) {
                Node w = x.parent.right;
                if (w.color == Color.RED) {                      // cas 1
                    w.color = Color.BLACK;
                    x.parent.color = Color.RED;
                    leftRotate(x.parent);
                    w = x.parent.right;
                }
                if (w.left.color == Color.BLACK && w.right.color == Color.BLACK) { // cas 2
                    w.color = Color.RED;
                    x = x.parent;
                } else {
                    if (w.right.color == Color.BLACK) {          // cas 3
                        w.left.color = Color.BLACK;
                        w.color = Color.RED;
                        rightRotate(w);
                        w = x.parent.right;
                    }
                    w.color = x.parent.color;                    // cas 4
                    x.parent.color = Color.BLACK;
                    w.right.color = Color.BLACK;
                    leftRotate(x.parent);
                    x = root;
                }
            } else {
                Node w = x.parent.left;
                if (w.color == Color.RED) {
                    w.color = Color.BLACK;
                    x.parent.color = Color.RED;
                    rightRotate(x.parent);
                    w = x.parent.left;
                }
                if (w.right.color == Color.BLACK && w.left.color == Color.BLACK) {
                    w.color = Color.RED;
                    x = x.parent;
                } else {
                    if (w.left.color == Color.BLACK) {
                        w.right.color = Color.BLACK;
                        w.color = Color.RED;
                        leftRotate(w);
                        w = x.parent.left;
                    }
                    w.color = x.parent.color;
                    x.parent.color = Color.BLACK;
                    w.left.color = Color.BLACK;
                    rightRotate(x.parent);
                    x = root;
                }
            }
        }
        x.color = Color.BLACK;
    }

    public void clear() {
        root = nil;
        size = 0;
    }

    // ------------------------- parcours -------------------------
    /** Ordre croissant. */
    public List<T> inorder() {
        List<T> out = new ArrayList<>();
        Deque<Node> stack = new ArrayDeque<>();
        Node current = root;
        while (!stack.isEmpty() || current != nil) {
            while (current != nil) {
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
        if (root == nil) {
            return out;
        }
        Deque<Node> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node node = stack.pop();
            out.add(node.key);
            if (node.right != nil) {
                stack.push(node.right);
            }
            if (node.left != nil) {
                stack.push(node.left);
            }
        }
        return out;
    }

    public List<T> levelorder() {
        List<T> out = new ArrayList<>();
        if (root == nil) {
            return out;
        }
        Deque<Node> queue = new ArrayDeque<>();
        queue.add(root);
        while (!queue.isEmpty()) {
            Node node = queue.poll();
            out.add(node.key);
            if (node.left != nil) {
                queue.add(node.left);
            }
            if (node.right != nil) {
                queue.add(node.right);
            }
        }
        return out;
    }

    /** Tous les nœuds réels en préordre (sentinelle exclue). */
    public List<Node> nodes() {
        List<Node> out = new ArrayList<>();
        if (root == nil) {
            return out;
        }
        Deque<Node> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node node = stack.pop();
            out.add(node);
            if (node.right != nil) {
                stack.push(node.right);
            }
            if (node.left != nil) {
                stack.push(node.left);
            }
        }
        return out;
    }

    // -------------------------- mesures -------------------------
    /** Hauteur : arbre vide = -1. */
    public int height() {
        if (root == nil) {
            return -1;
        }
        int best = 0;
        Deque<Node> nodes = new ArrayDeque<>();
        Deque<Integer> depths = new ArrayDeque<>();
        nodes.push(root);
        depths.push(0);
        while (!nodes.isEmpty()) {
            Node node = nodes.pop();
            int depth = depths.pop();
            best = Math.max(best, depth);
            if (node.left != nil) {
                nodes.push(node.left);
                depths.push(depth + 1);
            }
            if (node.right != nil) {
                nodes.push(node.right);
                depths.push(depth + 1);
            }
        }
        return best;
    }

    /** Nombre de nœuds noirs sur un chemin racine → feuille (NIL inclus). */
    public int blackHeight() {
        return blackHeight(root);
    }

    private int blackHeight(Node node) {
        if (node == nil) {
            return 1;
        }
        return blackHeight(node.left) + (node.color == Color.BLACK ? 1 : 0);
    }

    // --------------------- vérification -------------------------
    /** Lève {@link IllegalStateException} si un des cinq invariants est violé. */
    public void validate() {
        if (root == nil) {
            return;
        }
        if (root.color == Color.RED) {
            throw new IllegalStateException("invariant 2 violé : racine rouge");
        }
        if (root.parent != null) {
            throw new IllegalStateException("la racine possède un parent");
        }
        check(root, null, null);
        if (checkBlackHeight(root) < 0) {
            throw new IllegalStateException("invariant 5 violé : hauteurs noires différentes");
        }
    }

    private void check(Node node, T lo, T hi) {
        if (node == nil) {
            return;
        }
        if (lo != null && node.key.compareTo(lo) <= 0) {
            throw new IllegalStateException("ABR violé : " + node.key + " <= " + lo);
        }
        if (hi != null && node.key.compareTo(hi) >= 0) {
            throw new IllegalStateException("ABR violé : " + node.key + " >= " + hi);
        }
        if (node.color == Color.RED) {
            if (node.left.color == Color.RED) {
                throw new IllegalStateException("invariant 4 violé : " + node.key + "/" + node.left.key);
            }
            if (node.right.color == Color.RED) {
                throw new IllegalStateException("invariant 4 violé : " + node.key + "/" + node.right.key);
            }
        }
        check(node.left, lo, node.key);
        check(node.right, node.key, hi);
    }

    private int checkBlackHeight(Node node) {
        if (node == nil) {
            return 1;
        }
        int left = checkBlackHeight(node.left);
        int right = checkBlackHeight(node.right);
        if (left < 0 || right < 0 || left != right) {
            return -1;
        }
        return left + (node.color == Color.BLACK ? 1 : 0);
    }

    // ------------------------ affichage -------------------------
    @Override
    public String toString() {
        return "RBTree" + inorder();
    }

    /** Représentation arborescente avec couleurs. */
    public String toTreeString() {
        StringBuilder sb = new StringBuilder();
        printRec(root, "", true, sb);
        return sb.toString();
    }

    private void printRec(Node node, String prefix, boolean isLast, StringBuilder sb) {
        if (node == nil) {
            return;
        }
        sb.append(prefix).append(isLast ? "└── " : "├── ").append(node).append('\n');
        String childPrefix = prefix + (isLast ? "    " : "│   ");
        boolean hasLeft = node.left != nil;
        boolean hasRight = node.right != nil;
        if (hasLeft && hasRight) {
            printRec(node.left, childPrefix, false, sb);
            printRec(node.right, childPrefix, true, sb);
        } else if (hasLeft) {
            printRec(node.left, childPrefix, true, sb);
        } else if (hasRight) {
            printRec(node.right, childPrefix, true, sb);
        }
    }

    /** Nombre de nœuds noirs et rouges (utile pour les tests). */
    public List<Integer> colorCounts() {
        int red = 0;
        int black = 0;
        for (Node node : nodes()) {
            if (node.color == Color.RED) {
                red++;
            } else {
                black++;
            }
        }
        return Collections.unmodifiableList(java.util.Arrays.asList(red, black));
    }
}
