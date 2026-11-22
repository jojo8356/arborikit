"""Tests arborikit — pytest. Aucune dépendance hors stdlib (+ pytest)."""

import random

import pytest

from arborikit import BST, Color, GeneralTree, RBTree


# --------------------------- GeneralTree ---------------------------
def test_general_tree_basic():
    t = GeneralTree("CEO")
    t.add("CEO", "VP1")
    t.add("CEO", "VP2")
    t.add("VP1", "GM1")
    t.add("VP1", "GM2")
    assert t.size == 5
    assert t.height == 2
    assert t.preorder() == ["CEO", "VP1", "GM1", "GM2", "VP2"]
    assert t.levelorder() == ["CEO", "VP1", "VP2", "GM1", "GM2"]
    assert t.leaves() == ["GM1", "GM2", "VP2"]


def test_general_tree_paths_and_depth():
    t = GeneralTree(1)
    a = t.root.add_child(2)
    b = a.add_child(3)
    assert b.depth == 2
    assert b.path_from_root() == [1, 2, 3]
    assert t.find(3) is b
    assert t.find(99) is None


def test_general_tree_prune():
    t = GeneralTree("r")
    a = t.root.add_child("a")
    a.add_child("a1")
    t.root.add_child("b")
    assert t.prune("a") is True
    assert t.preorder() == ["r", "b"]
    assert t.prune("r") is False  # on ne supprime pas la racine


def test_general_tree_unknown_parent():
    t = GeneralTree("r")
    with pytest.raises(KeyError):
        t.add("absent", "x")


def test_general_tree_single_node():
    t = GeneralTree("solo")
    assert t.size == 1 and t.height == 0 and t.leaves() == ["solo"]


# ------------------------------- BST -------------------------------
def test_bst_insert_inorder():
    b = BST([50, 30, 70, 20, 40, 60, 80])
    assert b.inorder() == [20, 30, 40, 50, 60, 70, 80]
    assert b.size == 7
    assert b.height == 2
    assert 40 in b and 99 not in b
    assert b.min() == 20 and b.max() == 80


def test_bst_duplicates_ignored():
    b = BST()
    assert b.insert(5) is True
    assert b.insert(5) is False
    assert len(b) == 1


def test_bst_delete_three_cases():
    b = BST([50, 30, 70, 20, 40, 60, 80])
    assert b.delete(20) is True   # feuille
    assert b.inorder() == [30, 40, 50, 60, 70, 80]
    assert b.delete(30) is True   # 1 enfant
    assert b.inorder() == [40, 50, 60, 70, 80]
    assert b.delete(50) is True   # 2 enfants
    assert b.inorder() == [40, 60, 70, 80]
    assert len(b) == 4
    assert b.delete(999) is False


def test_bst_successor_predecessor():
    b = BST([20, 30, 40, 50, 60, 70])
    assert b.successor(50) == 60
    assert b.predecessor(50) == 40
    assert b.successor(70) is None
    assert b.predecessor(20) is None


def test_bst_parcours():
    b = BST([50, 30, 70, 20, 40])
    assert b.preorder() == [50, 30, 20, 40, 70]
    assert b.postorder() == [20, 40, 30, 70, 50]
    assert b.levelorder() == [50, 30, 70, 20, 40]


def test_bst_degenerescence():
    b = BST(range(10))
    assert b.height == 9
    assert b.is_degenerate is True
    assert list(b) == list(range(10))


def test_bst_empty():
    b = BST()
    assert b.is_empty and b.height == -1
    with pytest.raises(ValueError):
        b.min()


def test_bst_validate_detects_corruption():
    b = BST([50, 30, 70])
    b.validate()  # OK
    b._root.left.key = 999  # corruption volontaire
    with pytest.raises(ValueError):
        b.validate()


# ------------------------------ RBTree -----------------------------
def test_rb_insert_order_and_size():
    r = RBTree([50, 30, 70, 20, 40, 60, 80])
    assert r.inorder() == [20, 30, 40, 50, 60, 70, 80]
    assert len(r) == 7
    assert r.root.key == 50
    assert r.root.color is Color.BLACK


def test_rb_invariants_after_inserts():
    r = RBTree()
    for i in [10, 20, 30, 15, 25, 5, 1, 100, 55, 60]:
        r.insert(i)
        r.validate()


def test_rb_invariants_after_deletes():
    data = [50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45]
    r = RBTree(data)
    r.validate()
    for k in [10, 70, 50]:
        assert r.delete(k) is True
        r.validate()
    assert r.inorder() == [20, 25, 30, 35, 40, 45, 60, 80]


def test_rb_sorted_insert_stays_balanced():
    r = RBTree(range(1, 21))
    r.validate()
    assert r.height <= 8, f"hauteur {r.height} : l'ARN ne s'équilibre pas"
    assert r.inorder() == list(range(1, 21))
    assert r.black_height >= 2


def test_rb_duplicates_and_contains():
    r = RBTree([5, 5, 5])
    assert len(r) == 1
    assert 5 in r and 6 not in r
    assert r.insert(5) is False


def test_rb_min_max_successor():
    r = RBTree([50, 30, 70, 20, 60])
    assert r.min() == 20 and r.max() == 70
    assert r.successor(50) == 60
    assert r.predecessor(50) == 30
    assert r.successor(70) is None


def test_rb_str_shows_colors():
    r = RBTree([50, 30, 70])
    s = str(r)
    assert "(N)" in s and "(R)" in s


def test_rb_stress_vs_sorted_list():
    rnd = random.Random(7)
    r = RBTree()
    ref = set()
    for _ in range(3000):
        k = rnd.randrange(500)
        if rnd.random() < 0.6:
            r.insert(k)
            ref.add(k)
        else:
            r.delete(k)
            ref.discard(k)
        if rnd.randrange(200) == 0:
            r.validate()
    r.validate()
    assert r.inorder() == sorted(ref)
    assert len(r) == len(ref)


def test_rb_empty():
    r = RBTree()
    assert r.is_empty and r.root is None and r.height == -1
    assert r.delete(1) is False
    with pytest.raises(ValueError):
        r.min()
