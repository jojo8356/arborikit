// arborikit/rbtree.hpp — arbre rouge-noir (ARN) complet, header-only, C++11.
//
// std::map et std::set sont déjà des arbres rouge-noir, mais leur structure
// interne (couleurs, rotations, nœuds) est inaccessible. Cette classe expose
// les nœuds et permet de vérifier les cinq invariants : indispensable pour
// l'enseignement et les tests d'invariants.
//
// Implémentation CLRS avec sentinelle nil_.

#ifndef ARBORIKIT_RBTREE_HPP
#define ARBORIKIT_RBTREE_HPP

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace arborikit {

/// Couleur d'un nœud.
enum class Color { Red, Black };

inline const char* colorName(Color color) {
    return color == Color::Red ? "R" : "N";
}

/// Arbre rouge-noir générique, nœuds observables.
template <typename T, typename Compare = std::less<T>>
class RBTree {
public:
    struct Node {
        T key;
        Color color = Color::Red;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    RBTree() : nil_(new Node()), root_(nil_) {
        nil_->color = Color::Black;
        nil_->left = nil_;
        nil_->right = nil_;
        nil_->parent = nullptr;
    }

    RBTree(std::initializer_list<T> items) : RBTree() {
        for (const auto& item : items) {
            insert(item);
        }
    }

    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;

    ~RBTree() {
        destroy(root_);
        delete nil_;
    }

    // --------------------------- état ---------------------------
    bool empty() const { return root_ == nil_; }
    std::size_t size() const { return size_; }

    /// Racine exposée, ou nullptr si vide.
    const Node* root() const { return root_ == nil_ ? nullptr : root_; }

    /// Couleur d'un nœud, accessible pour l'affichage et les tests.
    static Color colorOf(const Node* node) { return node->color; }

    // ------------------------ rotations -------------------------
    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nil_) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nullptr) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != nil_) {
            y->right->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nullptr) {
            root_ = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }

    // ------------------------ insertion -------------------------
    /// Insère `key`. Renvoie false si déjà présent.
    bool insert(const T& key) {
        Node* y = nullptr;
        Node* x = root_;
        while (x != nil_) {
            y = x;
            if (equal(key, x->key)) {
                return false;
            }
            x = cmp_(key, x->key) ? x->left : x->right;
        }
        Node* z = new Node();
        z->key = key;
        z->color = Color::Red;
        z->left = nil_;
        z->right = nil_;
        z->parent = y;
        if (y == nullptr) {
            root_ = z;
        } else if (cmp_(key, y->key)) {
            y->left = z;
        } else {
            y->right = z;
        }
        ++size_;
        insertFixup(z);
        return true;
    }

    // ------------------------ recherche -------------------------
    const Node* find(const T& key) const { return findImpl(root_, key); }

    bool contains(const T& key) const { return find(key) != nullptr; }

    const T& min() const {
        if (empty()) {
            throw std::out_of_range("arborikit::RBTree::min : arbre vide");
        }
        return minimum(root_)->key;
    }

    const T& max() const {
        if (empty()) {
            throw std::out_of_range("arborikit::RBTree::max : arbre vide");
        }
        return maximum(root_)->key;
    }

    const T* successor(const T& key) const {
        const Node* x = find(key);
        if (x == nullptr) {
            return nullptr;
        }
        if (x->right != nil_) {
            return &minimum(x->right)->key;
        }
        const Node* y = x->parent;
        while (y != nullptr && x == y->right) {
            x = y;
            y = y->parent;
        }
        return y == nullptr ? nullptr : &y->key;
    }

    const T* predecessor(const T& key) const {
        const Node* x = find(key);
        if (x == nullptr) {
            return nullptr;
        }
        if (x->left != nil_) {
            return &maximum(x->left)->key;
        }
        const Node* y = x->parent;
        while (y != nullptr && x == y->left) {
            x = y;
            y = y->parent;
        }
        return y == nullptr ? nullptr : &y->key;
    }

    // ----------------------- suppression ------------------------
    /// Supprime `key`. Renvoie false si absent.
    bool erase(const T& key) {
        Node* z = const_cast<Node*>(find(key));
        if (z == nullptr) {
            return false;
        }
        eraseNode(z);
        --size_;
        return true;
    }

    // ------------------------ parcours --------------------------
    /// Ordre croissant.
    std::vector<T> inorder() const {
        std::vector<T> out;
        inorderImpl(root_, out);
        return out;
    }

    std::vector<T> preorder() const {
        std::vector<T> out;
        preorderImpl(root_, out);
        return out;
    }

    std::vector<T> levelorder() const {
        std::vector<const Node*> queue;
        if (!empty()) {
            queue.push_back(root_);
        }
        std::vector<T> out;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            out.push_back(queue[i]->key);
            if (queue[i]->left != nil_) {
                queue.push_back(queue[i]->left);
            }
            if (queue[i]->right != nil_) {
                queue.push_back(queue[i]->right);
            }
        }
        return out;
    }

    /// Tous les nœuds réels en préordre (sentinelle exclue).
    std::vector<const Node*> nodes() const {
        std::vector<const Node*> out;
        collect(root_, out);
        return out;
    }

    // ------------------------- mesures --------------------------
    /// Hauteur : arbre vide = -1.
    long height() const { return empty() ? -1 : heightImpl(root_); }

    /// Hauteur noire : nœuds noirs sur un chemin racine → feuille (nil inclus).
    std::size_t blackHeight() const { return blackHeightImpl(root_); }

    std::size_t countColor(Color color) const {
        std::size_t n = 0;
        for (const Node* node : nodes()) {
            if (node->color == color) {
                ++n;
            }
        }
        return n;
    }

    // --------------------- vérification -------------------------
    /// Lève std::logic_error si un des cinq invariants est violé.
    void validate() const {
        if (empty()) {
            return;
        }
        if (root_->color != Color::Black) {
            throw std::logic_error("ARN : invariant 2 violé (racine rouge)");
        }
        if (root_->parent != nullptr) {
            throw std::logic_error("ARN : la racine possède un parent");
        }
        check(root_, nullptr, nullptr);
        if (blackHeightCheck(root_) < 0) {
            throw std::logic_error("ARN : invariant 5 violé (hauteurs noires)");
        }
    }

    // ------------------------ affichage -------------------------
    std::string toString() const {
        std::string out;
        render(root_, "", true, out);
        return out;
    }

private:
    Node* nil_;
    Node* root_;
    std::size_t size_ = 0;
    Compare cmp_{};

    bool equal(const T& a, const T& b) const { return !cmp_(a, b) && !cmp_(b, a); }

    Node* minimum(Node* node) const {
        while (node->left != nil_) {
            node = node->left;
        }
        return node;
    }

    Node* maximum(Node* node) const {
        while (node->right != nil_) {
            node = node->right;
        }
        return node;
    }

    const Node* minimum(const Node* node) const {
        while (node->left != nil_) {
            node = node->left;
        }
        return node;
    }

    const Node* maximum(const Node* node) const {
        while (node->right != nil_) {
            node = node->right;
        }
        return node;
    }

    const Node* findImpl(const Node* node, const T& key) const {
        while (node != nil_) {
            if (equal(key, node->key)) {
                return node;
            }
            node = cmp_(key, node->key) ? node->left : node->right;
        }
        return nullptr;
    }

    void insertFixup(Node* z) {
        while (z->parent != nullptr && z->parent->color == Color::Red) {
            Node* grand = z->parent->parent;
            if (z->parent == grand->left) {
                Node* uncle = grand->right;
                if (uncle->color == Color::Red) {
                    z->parent->color = Color::Black;
                    uncle->color = Color::Black;
                    grand->color = Color::Red;
                    z = grand;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = Color::Black;
                    z->parent->parent->color = Color::Red;
                    rotateRight(z->parent->parent);
                }
            } else {
                Node* uncle = grand->left;
                if (uncle->color == Color::Red) {
                    z->parent->color = Color::Black;
                    uncle->color = Color::Black;
                    grand->color = Color::Red;
                    z = grand;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = Color::Black;
                    z->parent->parent->color = Color::Red;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root_->color = Color::Black;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    void eraseNode(Node* z) {
        Node* y = z;
        Color yOriginal = y->color;
        Node* x;
        if (z->left == nil_) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil_) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            yOriginal = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        delete z;
        if (yOriginal == Color::Black) {
            eraseFixup(x);
        }
    }

    void eraseFixup(Node* x) {
        while (x != root_ && x->color == Color::Black) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;
                if (w->color == Color::Red) {
                    w->color = Color::Black;
                    x->parent->color = Color::Red;
                    rotateLeft(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Color::Black && w->right->color == Color::Black) {
                    w->color = Color::Red;
                    x = x->parent;
                } else {
                    if (w->right->color == Color::Black) {
                        w->left->color = Color::Black;
                        w->color = Color::Red;
                        rotateRight(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::Black;
                    w->right->color = Color::Black;
                    rotateLeft(x->parent);
                    x = root_;
                }
            } else {
                Node* w = x->parent->left;
                if (w->color == Color::Red) {
                    w->color = Color::Black;
                    x->parent->color = Color::Red;
                    rotateRight(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == Color::Black && w->left->color == Color::Black) {
                    w->color = Color::Red;
                    x = x->parent;
                } else {
                    if (w->left->color == Color::Black) {
                        w->right->color = Color::Black;
                        w->color = Color::Red;
                        rotateLeft(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::Black;
                    w->left->color = Color::Black;
                    rotateRight(x->parent);
                    x = root_;
                }
            }
        }
        x->color = Color::Black;
    }

    void inorderImpl(const Node* node, std::vector<T>& out) const {
        if (node == nil_) {
            return;
        }
        inorderImpl(node->left, out);
        out.push_back(node->key);
        inorderImpl(node->right, out);
    }

    void preorderImpl(const Node* node, std::vector<T>& out) const {
        if (node == nil_) {
            return;
        }
        out.push_back(node->key);
        preorderImpl(node->left, out);
        preorderImpl(node->right, out);
    }

    void collect(const Node* node, std::vector<const Node*>& out) const {
        if (node == nil_) {
            return;
        }
        out.push_back(node);
        collect(node->left, out);
        collect(node->right, out);
    }

    long heightImpl(const Node* node) const {
        if (node == nil_) {
            return -1;
        }
        const long left = heightImpl(node->left);
        const long right = heightImpl(node->right);
        return 1 + (left > right ? left : right);
    }

    std::size_t blackHeightImpl(const Node* node) const {
        if (node == nil_) {
            return 1;
        }
        return blackHeightImpl(node->left) + (node->color == Color::Black ? 1 : 0);
    }

    void check(const Node* node, const T* lo, const T* hi) const {
        if (node == nil_) {
            return;
        }
        if (lo != nullptr && !cmp_(*lo, node->key)) {
            throw std::logic_error("ARN : propriété ABR violée (borne basse)");
        }
        if (hi != nullptr && !cmp_(node->key, *hi)) {
            throw std::logic_error("ARN : propriété ABR violée (borne haute)");
        }
        if (node->color == Color::Red) {
            if (node->left->color == Color::Red || node->right->color == Color::Red) {
                throw std::logic_error("ARN : invariant 4 violé (rouge-rouge)");
            }
        }
        check(node->left, lo, &node->key);
        check(node->right, &node->key, hi);
    }

    long blackHeightCheck(const Node* node) const {
        if (node == nil_) {
            return 1;
        }
        const long left = blackHeightCheck(node->left);
        const long right = blackHeightCheck(node->right);
        if (left < 0 || right < 0 || left != right) {
            return -1;
        }
        return left + (node->color == Color::Black ? 1 : 0);
    }

    void destroy(Node* node) {
        if (node == nil_) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    void render(const Node* node, const std::string& prefix, bool isLast,
                std::string& out) const {
        if (node == nil_) {
            return;
        }
        std::ostringstream oss;
        oss << prefix << (isLast ? "└── " : "├── ") << node->key << '('
            << colorName(node->color) << ")\n";
        out += oss.str();
        const std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        const bool hasLeft = node->left != nil_;
        const bool hasRight = node->right != nil_;
        if (hasLeft && hasRight) {
            render(node->left, childPrefix, false, out);
            render(node->right, childPrefix, true, out);
        } else if (hasLeft) {
            render(node->left, childPrefix, true, out);
        } else if (hasRight) {
            render(node->right, childPrefix, true, out);
        }
    }
};

}  // namespace arborikit

#endif  // ARBORIKIT_RBTREE_HPP
