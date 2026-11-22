// Tests arborikit C++ — autonomes, sans framework externe.
// Compilation :
//   c++ -std=c++11 -Wall -Wextra -Iinclude tests/test_arborikit.cpp -o test_cpp

#include <arborikit/arborikit.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "ECHEC " << __FILE__ << ":" << __LINE__ << " : "    \
                      << #cond << '\n';                                      \
            ++failures;                                                      \
        }                                                                    \
    } while (false)

// ------------------------------ GeneralTree ------------------------------
static void testGeneralTree() {
    arborikit::GeneralTree<std::string> tree("CEO");
    arborikit::GeneralTree<std::string>::Node* vp1 = tree.root()->addChild("VP1");
    arborikit::GeneralTree<std::string>::Node* vp2 = tree.root()->addChild("VP2");
    vp1->addChild("GM1");
    vp1->addChild("GM2");
    vp2->addChild("GM3");

    CHECK(tree.size() == 6);
    CHECK(tree.height() == 2);
    CHECK(tree.preorder() == (std::vector<std::string>{"CEO", "VP1", "GM1", "GM2", "VP2", "GM3"}));
    CHECK(tree.levelorder() == (std::vector<std::string>{"CEO", "VP1", "VP2", "GM1", "GM2", "GM3"}));
    CHECK(tree.leaves() == (std::vector<std::string>{"GM1", "GM2", "GM3"}));
    CHECK(tree.find("GM2") != nullptr);
    CHECK(tree.find("GM2")->depth() == 2);
    CHECK(tree.find("absent") == nullptr);
    CHECK(vp1->parent == tree.root());
    CHECK(!tree.root()->isLeaf());

    CHECK(tree.prune("VP2"));
    CHECK(tree.size() == 4);
    CHECK(!tree.prune("CEO"));

    const std::string rendered = tree.toString();
    CHECK(rendered.find("CEO") != std::string::npos);
}

// ---------------------------------- BST ----------------------------------
static void testBST() {
    arborikit::BST<int> bst{50, 30, 70, 20, 40, 60, 80};

    CHECK(bst.size() == 7);
    CHECK(bst.inorder() == (std::vector<int>{20, 30, 40, 50, 60, 70, 80}));
    CHECK(bst.height() == 2);
    CHECK(bst.min() == 20);
    CHECK(bst.max() == 80);
    CHECK(bst.contains(40));
    CHECK(!bst.contains(99));
    CHECK(!bst.insert(50));  // doublon

    CHECK(bst.successor(50) != nullptr && *bst.successor(50) == 60);
    CHECK(bst.predecessor(50) != nullptr && *bst.predecessor(50) == 40);
    CHECK(bst.successor(80) == nullptr);
    CHECK(bst.predecessor(20) == nullptr);

    CHECK(bst.erase(20));  // feuille
    CHECK(bst.inorder() == (std::vector<int>{30, 40, 50, 60, 70, 80}));
    CHECK(bst.erase(30));  // un enfant
    CHECK(bst.inorder() == (std::vector<int>{40, 50, 60, 70, 80}));
    CHECK(bst.erase(50));  // deux enfants
    CHECK(bst.inorder() == (std::vector<int>{40, 60, 70, 80}));
    CHECK(!bst.erase(999));
    bst.validate();

    CHECK(bst.preorder() == (std::vector<int>{60, 40, 70, 80}));
    CHECK(bst.levelorder() == (std::vector<int>{60, 40, 70, 80}));

    // dégénérescence
    arborikit::BST<int> degen;
    for (int i = 0; i < 10; ++i) {
        degen.insert(i);
    }
    CHECK(degen.height() == 9);
    CHECK(degen.isDegenerate());

    arborikit::BST<int> emptyTree;
    CHECK(emptyTree.empty());
    CHECK(emptyTree.height() == -1);
    bool threw = false;
    try {
        emptyTree.min();
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

// --------------------------------- RBTree --------------------------------
static void testRBTree() {
    arborikit::RBTree<int> rb{50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};

    CHECK(rb.size() == 11);
    CHECK(rb.inorder() == (std::vector<int>{10, 20, 25, 30, 35, 40, 45, 50, 60, 70, 80}));
    CHECK(rb.root() != nullptr);
    CHECK(arborikit::RBTree<int>::colorOf(rb.root()) == arborikit::Color::Black);
    CHECK(rb.blackHeight() >= 2);
    rb.validate();

    CHECK(!rb.insert(50));
    CHECK(rb.min() == 10);
    CHECK(rb.max() == 80);
    CHECK(rb.successor(45) != nullptr && *rb.successor(45) == 50);
    CHECK(rb.predecessor(45) != nullptr && *rb.predecessor(45) == 40);

    CHECK(rb.erase(10));
    rb.validate();
    CHECK(rb.erase(70));
    rb.validate();
    CHECK(rb.erase(50));
    rb.validate();
    CHECK(!rb.erase(999));
    CHECK(rb.size() == 8);

    const std::string rendered = rb.toString();
    CHECK(rendered.find('(') != std::string::npos);

    // insertions triées : doit rester équilibré
    arborikit::RBTree<int> sorted;
    for (int i = 1; i <= 50; ++i) {
        sorted.insert(i);
    }
    sorted.validate();
    CHECK(sorted.height() <= 12);
    CHECK(sorted.inorder().size() == 50);

    // stress contre std::set
    std::mt19937 rng(12345);
    arborikit::RBTree<int> stress;
    std::set<int> reference;
    for (int i = 0; i < 20000; ++i) {
        const int value = static_cast<int>(rng() % 1000u);
        if (rng() % 2u == 0u) {
            stress.insert(value);
            reference.insert(value);
        } else {
            stress.erase(value);
            reference.erase(value);
        }
        if (i % 1000 == 0) {
            stress.validate();
        }
    }
    stress.validate();
    CHECK(stress.size() == reference.size());
    CHECK(stress.inorder() == std::vector<int>(reference.begin(), reference.end()));
    CHECK(stress.countColor(arborikit::Color::Red) +
              stress.countColor(arborikit::Color::Black) ==
          reference.size());

    // comparateur personnalisé (ordre décroissant)
    arborikit::RBTree<int, std::greater<int>> desc;
    desc.insert(1);
    desc.insert(2);
    desc.insert(3);
    desc.validate();
    CHECK(desc.inorder() == (std::vector<int>{3, 2, 1}));
}

int main() {
    std::cout << "arborikit C++ — tests\n";
    testGeneralTree();
    testBST();
    testRBTree();
    if (failures == 0) {
        std::cout << "Tous les tests sont passes.\n";
        return 0;
    }
    std::cerr << failures << " echec(s)\n";
    return 1;
}
