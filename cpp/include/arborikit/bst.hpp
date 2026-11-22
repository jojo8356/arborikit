// arborikit/bst.hpp — arbre binaire de recherche NON équilibré, header-only, C++11.
//
// La std::lib C++ ne fournit que des conteneurs équilibrés (std::map/set sont des
// arbres rouge-noir). Cette classe expose un ABR non équilibré, indispensable
// pour observer la dégénérescence en O(n) et enseigner les rotations.
//
// Propriété : gauche < nœud < droite. Doublons ignorés.

#ifndef ARBORIKIT_BST_HPP
#define ARBORIKIT_BST_HPP

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace arborikit {

/// Arbre binaire de recherche non équilibré.
template <typename T, typename Compare = std::less<T>>
class BST {
public:
    struct Node {
        T key;
        Node* left = nullptr;
        Node* right = nullptr;

        explicit Node(const T& value) : key(value) {}
    };

    BST() = default;

    BST(std::initializer_list<T> items) {
        for (const auto& item : items) {
            insert(item);
        }
    }

    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    ~BST() { clear(); }

    // ------------------------- état -------------------------
    bool empty() const { return root_ == nullptr; }
    std::size_t size() const { return size_; }
    const Node* root() const { return root_; }

    // ---------------------- insertion -----------------------
    /// Insère `key`. Renvoie false si déjà présent.
    bool insert(const T& key) {
        if (root_ == nullptr) {
            root_ = new Node(key);
            size_ = 1;
            return true;
        }

        Node* cur = root_;
        for (;;) {
            if (cmp_(key, cur->key) == false && cmp_(cur->key, key) == false) {
                return false;
            }
            if (cmp_(key, cur->key)) {
                if (cur->left == nullptr) {
                    cur->left = new Node(key);
                    ++size_;
                    return true;
                }
                cur = cur->left;
            } else {
                if (cur->right == nullptr) {
                    cur->right = new Node(key);
                    ++size_;
                    return true;
                }
                cur = cur->right;
            }
        }
    }

    // ---------------------- recherche -----------------------
    bool contains(const T& key) const { return find(key) != nullptr; }

    const Node* find(const T& key) const {
        const Node* cur = root_;
        while (cur != nullptr) {
            if (cmp_(key, cur->key)) {
                cur = cur->left;
            } else if (cmp_(cur->key, key)) {
                cur = cur->right;
            } else {
                return cur;
            }
        }
        return nullptr;
    }

    const T& min() const {
        if (root_ == nullptr) {
            throw std::out_of_range("arborikit::BST::min : arbre vide");
        }
        return minNode(root_)->key;
    }

    const T& max() const {
        if (root_ == nullptr) {
            throw std::out_of_range("arborikit::BST::max : arbre vide");
        }
        return maxNode(root_)->key;
    }

    /// Plus petit élément strictement supérieur, ou nullptr.
    const T* successor(const T& key) const {
        const Node* cur = root_;
        const Node* best = nullptr;
        while (cur != nullptr) {
            if (cmp_(key, cur->key)) {
                best = cur;
                cur = cur->left;
            } else if (cmp_(cur->key, key)) {
                cur = cur->right;
            } else {
                if (cur->right != nullptr) {
                    return &minNode(cur->right)->key;
                }
                return best == nullptr ? nullptr : &best->key;
            }
        }
        return best == nullptr ? nullptr : &best->key;
    }

    /// Plus grand élément strictement inférieur, ou nullptr.
    const T* predecessor(const T& key) const {
        const Node* cur = root_;
        const Node* best = nullptr;
        while (cur != nullptr) {
            if (cmp_(cur->key, key)) {
                best = cur;
                cur = cur->right;
            } else if (cmp_(key, cur->key)) {
                cur = cur->left;
            } else {
                if (cur->left != nullptr) {
                    return &maxNode(cur->left)->key;
                }
                return best == nullptr ? nullptr : &best->key;
            }
        }
        return best == nullptr ? nullptr : &best->key;
    }

    // --------------------- suppression ----------------------
    /// Supprime `key`. Renvoie false si absent.
    bool erase(const T& key) {
        const std::size_t before = size_;
        root_ = eraseImpl(root_, key);
        return size_ < before;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }

    // ---------------------- parcours ------------------------
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

    std::vector<T> postorder() const {
        std::vector<T> out;
        postorderImpl(root_, out);
        return out;
    }

    std::vector<T> levelorder() const {
        std::vector<const Node*> queue;
        if (root_ != nullptr) {
            queue.push_back(root_);
        }
        std::vector<T> out;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            out.push_back(queue[i]->key);
            if (queue[i]->left != nullptr) {
                queue.push_back(queue[i]->left);
            }
            if (queue[i]->right != nullptr) {
                queue.push_back(queue[i]->right);
            }
        }
        return out;
    }

    // ----------------------- mesures ------------------------
    /// Hauteur : arbre vide = -1, un nœud = 0.
    long height() const { return heightImpl(root_); }

    /// Vrai si l'arbre est une liste chaînée (hauteur == size - 1).
    bool isDegenerate() const {
        return size_ > 0 && static_cast<std::size_t>(height()) == size_ - 1;
    }

    /// Lève std::logic_error si la propriété ABR est violée.
    void validate() const { validateImpl(root_, nullptr, nullptr); }

    /// Équivalent de std::map::operator== sur les clés ? Non : comparaison de clés triées.
    bool operator==(const BST& other) const { return inorder() == other.inorder(); }

    std::string toString() const {
        std::string out;
        render(root_, "", true, out);
        return out;
    }

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    Compare cmp_{};

    static const Node* minNode(const Node* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    static const Node* maxNode(const Node* node) {
        while (node->right != nullptr) {
            node = node->right;
        }
        return node;
    }

    static Node* minNode(Node* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    Node* eraseImpl(Node* node, const T& key) {
        if (node == nullptr) {
            return nullptr;
        }
        if (cmp_(key, node->key)) {
            node->left = eraseImpl(node->left, key);
            return node;
        }
        if (cmp_(node->key, key)) {
            node->right = eraseImpl(node->right, key);
            return node;
        }
        --size_;
        if (node->left == nullptr) {
            Node* right = node->right;
            delete node;
            return right;
        }
        if (node->right == nullptr) {
            Node* left = node->left;
            delete node;
            return left;
        }
        Node* successor = minNode(node->right);
        node->key = successor->key;
        const T replacement = successor->key;
        node->right = eraseImpl(node->right, replacement);
        ++size_;  // compense le décrément du sous-appel
        return node;
    }

    static void inorderImpl(const Node* node, std::vector<T>& out) {
        if (node == nullptr) {
            return;
        }
        inorderImpl(node->left, out);
        out.push_back(node->key);
        inorderImpl(node->right, out);
    }

    static void preorderImpl(const Node* node, std::vector<T>& out) {
        if (node == nullptr) {
            return;
        }
        out.push_back(node->key);
        preorderImpl(node->left, out);
        preorderImpl(node->right, out);
    }

    static void postorderImpl(const Node* node, std::vector<T>& out) {
        if (node == nullptr) {
            return;
        }
        postorderImpl(node->left, out);
        postorderImpl(node->right, out);
        out.push_back(node->key);
    }

    static long heightImpl(const Node* node) {
        if (node == nullptr) {
            return -1;
        }
        const long left = heightImpl(node->left);
        const long right = heightImpl(node->right);
        return 1 + (left > right ? left : right);
    }

    void validateImpl(const Node* node, const T* lo, const T* hi) const {
        if (node == nullptr) {
            return;
        }
        if (lo != nullptr && !cmp_(*lo, node->key)) {
            throw std::logic_error("arborikit::BST : propriété ABR violée");
        }
        if (hi != nullptr && !cmp_(node->key, *hi)) {
            throw std::logic_error("arborikit::BST : propriété ABR violée");
        }
        validateImpl(node->left, lo, &node->key);
        validateImpl(node->right, &node->key, hi);
    }

    static void destroy(Node* node) {
        if (node == nullptr) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static void render(const Node* node, const std::string& prefix, bool isLast,
                       std::string& out) {
        if (node == nullptr) {
            return;
        }
        std::ostringstream oss;
        oss << prefix << (isLast ? "└── " : "├── ") << node->key << '\n';
        out += oss.str();
        const std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        if (node->left != nullptr && node->right != nullptr) {
            render(node->left, childPrefix, false, out);
            render(node->right, childPrefix, true, out);
        } else if (node->left != nullptr) {
            render(node->left, childPrefix, true, out);
        } else if (node->right != nullptr) {
            render(node->right, childPrefix, true, out);
        }
    }
};

}  // namespace arborikit

#endif  // ARBORIKIT_BST_HPP
