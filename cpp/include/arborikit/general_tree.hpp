// arborikit/general_tree.hpp — arbre généraliste (n-aire), header-only, C++11.
//
// Aucune dépendance. Aucune allocation hormis les nœuds (std::vector).
// Contrairement à un arbre binaire, chaque nœud possède 0..n enfants ordonnés.

#ifndef ARBORIKIT_GENERAL_TREE_HPP
#define ARBORIKIT_GENERAL_TREE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace arborikit {

/// Arbre généraliste (n-aire) générique.
template <typename T>
class GeneralTree {
public:
    /// Nœud : parent non possédant, enfants possédés par la racine.
    struct Node {
        T data;
        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;

        explicit Node(const T& value) : data(value) {}
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        /// Ajoute un enfant et renvoie un pointeur stable vers lui.
        Node* addChild(const T& value) {
            children.emplace_back(new Node(value));
            children.back()->parent = this;
            return children.back().get();
        }

        bool isLeaf() const { return children.empty(); }
        bool isRoot() const { return parent == nullptr; }
        std::size_t childCount() const { return children.size(); }

        /// Profondeur : racine = 0.
        std::size_t depth() const {
            std::size_t d = 0;
            for (const Node* cur = this; cur->parent != nullptr; cur = cur->parent) {
                ++d;
            }
            return d;
        }

        /// Hauteur du sous-arbre : feuille = 0.
        std::size_t height() const {
            std::size_t best = 0;
            for (const auto& child : children) {
                const std::size_t h = child->height() + 1;
                if (h > best) {
                    best = h;
                }
            }
            return best;
        }

        /// Nombre de nœuds du sous-arbre, soi inclus.
        std::size_t subtreeSize() const {
            std::size_t total = 1;
            for (const auto& child : children) {
                total += child->subtreeSize();
            }
            return total;
        }

        /// Chemin racine → ce nœud.
        std::vector<T> pathFromRoot() const {
            std::vector<T> path;
            for (const Node* cur = this; cur != nullptr; cur = cur->parent) {
                path.push_back(cur->data);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        void visitPreorder(const std::function<void(const Node&)>& fn) const {
            fn(*this);
            for (const auto& child : children) {
                child->visitPreorder(fn);
            }
        }
    };

    explicit GeneralTree(const T& rootData) : root_(new Node(rootData)) {}

    Node* root() { return root_.get(); }
    const Node* root() const { return root_.get(); }

    std::size_t size() const { return root_->subtreeSize(); }
    std::size_t height() const { return root_->height(); }

    /// Ajoute sous le nœud dont la donnée égale `parentData`.
    Node* add(const T& parentData, const T& childData) {
        Node* parent = find(parentData);
        if (parent == nullptr) {
            throw std::invalid_argument("arborikit::GeneralTree::add : parent introuvable");
        }
        return parent->addChild(childData);
    }

    /// Premier nœud dont la donnée égale `data`, ou nullptr.
    Node* find(const T& data) { return findImpl(root_.get(), data); }

    const Node* find(const T& data) const { return findImpl(root_.get(), data); }

    /// Supprime le nœud (et son sous-arbre). La racine n'est pas supprimable.
    bool prune(const T& data) {
        Node* node = find(data);
        if (node == nullptr || node->parent == nullptr) {
            return false;
        }
        Node* parent = node->parent;
        for (auto it = parent->children.begin(); it != parent->children.end(); ++it) {
            if (it->get() == node) {
                parent->children.erase(it);
                return true;
            }
        }
        return false;
    }

    std::vector<T> preorder() const {
        std::vector<T> out;
        root_->visitPreorder([&out](const Node& n) { out.push_back(n.data); });
        return out;
    }

    std::vector<T> postorder() const {
        std::vector<T> out;
        postorderImpl(root_.get(), out);
        return out;
    }

    std::vector<T> levelorder() const {
        std::vector<const Node*> queue;
        queue.push_back(root_.get());
        std::vector<T> out;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            out.push_back(queue[i]->data);
            for (const auto& child : queue[i]->children) {
                queue.push_back(child.get());
            }
        }
        return out;
    }

    std::vector<T> leaves() const {
        std::vector<T> out;
        leavesImpl(root_.get(), out);
        return out;
    }

    /// Représentation arborescente (UTF-8 si le terminal le supporte).
    std::string toString() const {
        std::string out;
        render(root_.get(), "", true, out);
        return out;
    }

private:
    std::unique_ptr<Node> root_;

    static Node* findImpl(Node* node, const T& data) {
        if (node->data == data) {
            return node;
        }
        for (const auto& child : node->children) {
            Node* found = findImpl(child.get(), data);
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    }

    static void postorderImpl(const Node* node, std::vector<T>& out) {
        for (const auto& child : node->children) {
            postorderImpl(child.get(), out);
        }
        out.push_back(node->data);
    }

    static void leavesImpl(const Node* node, std::vector<T>& out) {
        if (node->isLeaf()) {
            out.push_back(node->data);
            return;
        }
        for (const auto& child : node->children) {
            leavesImpl(child.get(), out);
        }
    }

    static void render(const Node* node, const std::string& prefix, bool isLast,
                       std::string& out) {
        out += prefix;
        out += isLast ? "└── " : "├── ";
        out += toText(node->data);
        out += '\n';
        const std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            render(node->children[i].get(), childPrefix,
                   i + 1 == node->children.size(), out);
        }
    }

    template <typename U>
    static std::string toText(const U& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
};

}  // namespace arborikit

#endif  // ARBORIKIT_GENERAL_TREE_HPP
