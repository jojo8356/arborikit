"""Arbre binaire de recherche NON équilibré (ABR / BST).

Propriété : pour tout nœud, sous-arbre gauche < nœud < sous-arbre droit.
Complexités : O(log n) en moyenne, O(n) au pire (insertion triée).
"""

from __future__ import annotations

from typing import Any, Iterator, List, Optional


class _Node:
    __slots__ = ("key", "left", "right")

    def __init__(self, key: Any) -> None:
        self.key = key
        self.left: Optional[_Node] = None
        self.right: Optional[_Node] = None

    def __repr__(self) -> str:
        return f"_Node({self.key!r})"


class BST:
    """ABR non équilibré, doublons ignorés."""

    __slots__ = ("_root", "_size")

    def __init__(self, items: Optional[Any] = None) -> None:
        self._root: Optional[_Node] = None
        self._size = 0
        if items is not None:
            for x in items:
                self.insert(x)

    # -- état ----------------------------------------------------
    def __len__(self) -> int:
        return self._size

    def __bool__(self) -> bool:
        return self._root is not None

    def __contains__(self, key: Any) -> bool:
        return self.find(key) is not None

    def __iter__(self) -> Iterator[Any]:
        return iter(self.inorder())

    @property
    def size(self) -> int:
        return self._size

    @property
    def is_empty(self) -> bool:
        return self._root is None

    # -- insertion -----------------------------------------------
    def insert(self, key: Any) -> bool:
        """Insère key. Retourne False si déjà présent."""
        if self._root is None:
            self._root = _Node(key)
            self._size = 1
            return True
        cur = self._root
        while True:
            if key == cur.key:
                return False
            if key < cur.key:
                if cur.left is None:
                    cur.left = _Node(key)
                    self._size += 1
                    return True
                cur = cur.left
            else:
                if cur.right is None:
                    cur.right = _Node(key)
                    self._size += 1
                    return True
                cur = cur.right

    # -- recherche -----------------------------------------------
    def find(self, key: Any) -> Optional[_Node]:
        cur = self._root
        while cur is not None:
            if key == cur.key:
                return cur
            cur = cur.left if key < cur.key else cur.right
        return None

    def min(self) -> Any:
        if self._root is None:
            raise ValueError("arbre vide")
        cur = self._root
        while cur.left is not None:
            cur = cur.left
        return cur.key

    def max(self) -> Any:
        if self._root is None:
            raise ValueError("arbre vide")
        cur = self._root
        while cur.right is not None:
            cur = cur.right
        return cur.key

    def successor(self, key: Any) -> Optional[Any]:
        """Plus petit élément strictement supérieur à key."""
        cur, succ = self._root, None
        while cur is not None:
            if key < cur.key:
                succ, cur = cur.key, cur.left
            elif key > cur.key:
                cur = cur.right
            else:
                if cur.right is not None:
                    m = cur.right
                    while m.left is not None:
                        m = m.left
                    return m.key
                return succ
        return succ

    def predecessor(self, key: Any) -> Optional[Any]:
        cur, pred = self._root, None
        while cur is not None:
            if key > cur.key:
                pred, cur = cur.key, cur.right
            elif key < cur.key:
                cur = cur.left
            else:
                if cur.left is not None:
                    m = cur.left
                    while m.right is not None:
                        m = m.right
                    return m.key
                return pred
        return pred

    # -- suppression ---------------------------------------------
    def delete(self, key: Any) -> bool:
        before = self._size
        self._root = self._delete(self._root, key)
        return self._size < before

    def _delete(self, node: Optional[_Node], key: Any) -> Optional[_Node]:
        if node is None:
            return None
        if key < node.key:
            node.left = self._delete(node.left, key)
        elif key > node.key:
            node.right = self._delete(node.right, key)
        else:
            self._size -= 1
            if node.left is None:
                return node.right
            if node.right is None:
                return node.left
            nxt = node.right
            while nxt.left is not None:
                nxt = nxt.left
            node.key = nxt.key
            node.right = self._delete(node.right, nxt.key)
            self._size += 1  # annule le décrément du sous-appel
        return node

    def clear(self) -> None:
        self._root = None
        self._size = 0

    # -- parcours ------------------------------------------------
    def inorder(self) -> List[Any]:
        """Ordre croissant."""
        out: List[Any] = []
        stack, cur = [], self._root
        while stack or cur is not None:
            while cur is not None:
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            out.append(cur.key)
            cur = cur.right
        return out

    def preorder(self) -> List[Any]:
        out: List[Any] = []
        stack = [self._root] if self._root else []
        while stack:
            n = stack.pop()
            out.append(n.key)
            if n.right is not None:
                stack.append(n.right)
            if n.left is not None:
                stack.append(n.left)
        return out

    def postorder(self) -> List[Any]:
        out: List[Any] = []
        stack = [self._root] if self._root else []
        while stack:
            n = stack.pop()
            out.append(n.key)
            if n.left is not None:
                stack.append(n.left)
            if n.right is not None:
                stack.append(n.right)
        return out[::-1]

    def levelorder(self) -> List[Any]:
        out: List[Any] = []
        queue = [self._root] if self._root else []
        i = 0
        while i < len(queue):
            n = queue[i]
            i += 1
            out.append(n.key)
            if n.left is not None:
                queue.append(n.left)
            if n.right is not None:
                queue.append(n.right)
        return out

    # -- mesures -------------------------------------------------
    @property
    def height(self) -> int:
        """Hauteur : arbre vide = -1, un nœud = 0."""
        if self._root is None:
            return -1
        best, stack = 0, [(self._root, 0)]
        while stack:
            n, d = stack.pop()
            best = max(best, d)
            if n.left is not None:
                stack.append((n.left, d + 1))
            if n.right is not None:
                stack.append((n.right, d + 1))
        return best

    @property
    def is_degenerate(self) -> bool:
        """Vrai si l'arbre est une liste chaînée (hauteur == size-1)."""
        return self._size > 0 and self.height == self._size - 1

    # -- vérification --------------------------------------------
    def validate(self) -> None:
        """Lève ValueError si la propriété ABR est violée."""
        stack = [(self._root, None, None)]
        while stack:
            n, lo, hi = stack.pop()
            if n is None:
                continue
            if lo is not None and not (lo < n.key):
                raise ValueError(f"ABR violé : {n.key!r} <= {lo!r}")
            if hi is not None and not (n.key < hi):
                raise ValueError(f"ABR violé : {n.key!r} >= {hi!r}")
            stack.append((n.left, lo, n.key))
            stack.append((n.right, n.key, hi))

    # -- affichage ------------------------------------------------
    def __str__(self) -> str:
        lines: List[str] = []

        def rec(n: Optional[_Node], prefix: str, is_last: bool) -> None:
            if n is None:
                return
            lines.append(prefix + ("└── " if is_last else "├── ") + str(n.key))
            child_prefix = prefix + ("    " if is_last else "│   ")
            kids = [c for c in (n.left, n.right) if c is not None]
            for i, c in enumerate(kids):
                rec(c, child_prefix, i == len(kids) - 1)

        rec(self._root, "", True)
        return "\n".join(lines)

    def __repr__(self) -> str:
        return f"BST(size={self._size}, height={self.height})"
