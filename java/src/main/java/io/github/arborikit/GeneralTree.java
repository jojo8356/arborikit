package io.github.arborikit;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.List;
import java.util.Objects;

/**
 * GeneralTree — arbre généraliste (n-aire), sans dépendance.
 *
 * <p>Chaque nœud possède un parent (sauf la racine) et une liste ordonnée
 * d'enfants. Générique {@code <T>}, sans lien avec Swing : contrairement à
 * {@code javax.swing.tree.DefaultMutableTreeNode}, cette classe ne dépend
 * d'aucun package d'interface graphique et n'utilise pas {@code Object}.
 *
 * @param <T> type des données stockées
 */
public final class GeneralTree<T> {

    /** Nœud d'arbre généraliste. */
    public static final class Node<T> {
        private T data;
        private Node<T> parent;
        private final List<Node<T>> children = new ArrayList<>();

        Node(T data, Node<T> parent) {
            this.data = data;
            this.parent = parent;
        }

        public T getData() { return data; }

        public void setData(T data) { this.data = data; }

        public Node<T> getParent() { return parent; }

        public List<Node<T>> getChildren() { return Collections.unmodifiableList(children); }

        public boolean isLeaf() { return children.isEmpty(); }

        public boolean isRoot() { return parent == null; }

        public int childCount() { return children.size(); }

        /** Ajoute un enfant et retourne le nœud créé. */
        public Node<T> addChild(T childData) {
            Node<T> child = new Node<>(childData, this);
            children.add(child);
            return child;
        }

        /** Rattache un nœud existant (le détache d'abord de son parent). */
        public Node<T> addNode(Node<T> node) {
            Objects.requireNonNull(node, "node");
            if (node.parent != null) {
                node.parent.children.remove(node);
            }
            node.parent = this;
            this.children.add(node);
            return node;
        }

        /** Supprime un enfant direct. */
        public boolean removeChild(Node<T> child) {
            boolean removed = children.remove(child);
            if (removed) {
                child.parent = null;
            }
            return removed;
        }

        /** Profondeur : la racine vaut 0. */
        public int depth() {
            int depth = 0;
            Node<T> current = this;
            while (current.parent != null) {
                depth++;
                current = current.parent;
            }
            return depth;
        }

        /** Hauteur du sous-arbre : une feuille vaut 0. */
        public int height() {
            if (children.isEmpty()) {
                return 0;
            }
            int max = 0;
            for (Node<T> child : children) {
                max = Math.max(max, child.height());
            }
            return 1 + max;
        }

        /** Nombre de nœuds du sous-arbre, soi inclus. */
        public int subtreeSize() {
            int size = 1;
            for (Node<T> child : children) {
                size += child.subtreeSize();
            }
            return size;
        }

        /** Chemin racine → ce nœud. */
        public List<T> pathFromRoot() {
            List<T> path = new ArrayList<>();
            Node<T> current = this;
            while (current != null) {
                path.add(current.data);
                current = current.parent;
            }
            Collections.reverse(path);
            return path;
        }

        @Override
        public String toString() { return String.valueOf(data); }
    }

    private final Node<T> root;

    public GeneralTree(T rootData) {
        this.root = new Node<>(rootData, null);
    }

    public Node<T> getRoot() { return root; }

    /** Taille totale. */
    public int size() { return root.subtreeSize(); }

    /** Hauteur totale : racine seule = 0. */
    public int height() { return root.height(); }

    // ------------------------- parcours -------------------------
    /** DFS préordre. */
    public List<T> preorder() {
        List<T> out = new ArrayList<>();
        Deque<Node<T>> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node<T> node = stack.pop();
            out.add(node.data);
            for (int i = node.children.size() - 1; i >= 0; i--) {
                stack.push(node.children.get(i));
            }
        }
        return out;
    }

    /** DFS postordre. */
    public List<T> postorder() {
        List<T> out = new ArrayList<>();
        postorderRec(root, out);
        return out;
    }

    private void postorderRec(Node<T> node, List<T> out) {
        for (Node<T> child : node.children) {
            postorderRec(child, out);
        }
        out.add(node.data);
    }

    /** BFS. */
    public List<T> levelorder() {
        List<T> out = new ArrayList<>();
        Deque<Node<T>> queue = new ArrayDeque<>();
        queue.add(root);
        while (!queue.isEmpty()) {
            Node<T> node = queue.poll();
            out.add(node.data);
            queue.addAll(node.children);
        }
        return out;
    }

    /** Toutes les feuilles, de gauche à droite. */
    public List<T> leaves() {
        List<T> out = new ArrayList<>();
        leavesRec(root, out);
        return out;
    }

    private void leavesRec(Node<T> node, List<T> out) {
        if (node.isLeaf()) {
            out.add(node.data);
            return;
        }
        for (Node<T> child : node.children) {
            leavesRec(child, out);
        }
    }

    // ------------------------ recherche -------------------------
    /** Premier nœud contenant {@code data}, ou {@code null}. */
    public Node<T> find(T data) {
        Deque<Node<T>> stack = new ArrayDeque<>();
        stack.push(root);
        while (!stack.isEmpty()) {
            Node<T> node = stack.pop();
            if (Objects.equals(node.data, data)) {
                return node;
            }
            for (int i = node.children.size() - 1; i >= 0; i--) {
                stack.push(node.children.get(i));
            }
        }
        return null;
    }

    /** Ajoute sous le nœud contenant {@code parentData}. */
    public Node<T> add(T parentData, T childData) {
        Node<T> parent = find(parentData);
        if (parent == null) {
            throw new IllegalArgumentException("Nœud parent introuvable : " + parentData);
        }
        return parent.addChild(childData);
    }

    /** Supprime le nœud et tout son sous-arbre. La racine n'est pas supprimable. */
    public boolean prune(T data) {
        Node<T> node = find(data);
        if (node == null || node.parent == null) {
            return false;
        }
        return node.parent.removeChild(node);
    }

    // ------------------------ affichage -------------------------
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        printRec(root, "", true, sb);
        return sb.toString();
    }

    private void printRec(Node<T> node, String prefix, boolean isLast, StringBuilder sb) {
        sb.append(prefix).append(isLast ? "└── " : "├── ").append(node.data).append('\n');
        String childPrefix = prefix + (isLast ? "    " : "│   ");
        for (int i = 0; i < node.children.size(); i++) {
            printRec(node.children.get(i), childPrefix, i == node.children.size() - 1, sb);
        }
    }
}
