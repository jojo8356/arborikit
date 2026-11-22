"""Arbre rouge-noir (ARN / Red-Black Tree) — implémentation CLRS complète.

Contrairement à un dictionnaire trié opaque, cette implémentation expose
les nœuds, les couleurs, les rotations et permet de vérifier les 5 invariants.

Invariants :
1. chaque nœud est rouge ou noir
2. la racine est noire
3. toute feuille (NIL) est noire
4. un nœud rouge a deux enfants noirs (pas de rouge-rouge)
5. tout chemin racine→feuille a le même nombre de nœuds noirs
"""

from __future__ import annotations

from enum import Enum
from typing import Any, Iterator, List, Optional


class Color(Enum):
    RED = "R"
    BLACK = "N"

    def __str__(self) -> str:  # pragma: no cover - cosmétique
        return self.value


class RBNode:
    """Nœud rouge-noir exposé (couleur lisible, parent accessible)."""

    __slots__ = ("key", "left", "right", "parent", "color")

    def __init__(self, key: Any = None, color: Color = Color.BLACK) -> None:
        self.key = key
        self.left: RBNode = self
        self.right: RBNode = self
        self.parent: Optional[RBNode] = None
        self.color = color

    def __repr__(self) -> str:
        return f"{self.key!r}({self.color.value})"


class RBTree:
    """ARN avec sentinelle NIL, doublons ignorés."""

    __slots__ = ("_nil", "_root", "_size")

    def __init__(self, items: Optional[Any] = None) -> None:
        self._nil: RBNode = RBNode(None, Color.BLACK)
        self._nil.left = self._nil
        self._nil.right = self._nil
        self._nil.parent = None
        self._root = self._nil
        self._size = 0
        if items is not None:
            for x in items:
                self.insert(x)

    # -- état ----------------------------------------------------
    def __len__(self) -> int:
        return self._size

    def __bool__(self) -> bool:
        return self._root is not self._nil

    def __contains__(self, key: Any) -> bool:
        return self.find(key) is not None

    def __iter__(self) -> Iterator[Any]:
        return iter(self.inorder())

    @property
    def size(self) -> int:
        return self._size

    @property
    def is_empty(self) -> bool:
        return self._root is self._nil

    @property
    def root(self) -> Optional[RBNode]:
        """Racine exposée (None si vide)."""
        return None if self._root is self._nil else self._root

    # -- rotations -----------------------------------------------
    def _left_rotate(self, x: RBNode) -> None:
        y = x.right
        x.right = y.left
        if y.left is not self._nil:
            y.left.parent = x
        y.parent = x.parent
        if x.parent is None:
            self._root = y
        elif x is x.parent.left:
            x.parent.left = y
        else:
            x.parent.right = y
        y.left = x
        x.parent = y

    def _right_rotate(self, x: RBNode) -> None:
        y = x.left
        x.left = y.right
        if y.right is not self._nil:
            y.right.parent = x
        y.parent = x.parent
        if x.parent is None:
            self._root = y
        elif x is x.parent.right:
            x.parent.right = y
        else:
            x.parent.left = y
        y.right = x
        x.parent = y

    # -- insertion -----------------------------------------------
    def insert(self, key: Any) -> bool:
        """Insère key (nœud rouge + fixup). False si doublon."""
        nil = self._nil
        y: Optional[RBNode] = None
        x = self._root
        while x is not nil:
            y = x
            if key == x.key:
                return False
            x = x.left if key < x.key else x.right
        z = RBNode(key, Color.RED)
        z.left = z.right = nil
        z.parent = y
        if y is None:
            self._root = z
        elif key < y.key:
            y.left = z
        else:
            y.right = z
        self._size += 1
        self._insert_fixup(z)
        return True

    def _insert_fixup(self, z: RBNode) -> None:
        red, black = Color.RED, Color.BLACK
        while z.parent is not None and z.parent.color is red:
            gp = z.parent.parent
            if z.parent is gp.left:
                y = gp.right  # oncle
                if y.color is red:                     # cas 1
                    z.parent.color = black
                    y.color = black
                    gp.color = red
                    z = gp
                else:
                    if z is z.parent.right:            # cas 2 : zigzag
                        z = z.parent
                        self._left_rotate(z)
                    z.parent.color = black             # cas 3 : ligne
                    z.parent.parent.color = red
                    self._right_rotate(z.parent.parent)
            else:
                y = gp.left
                if y.color is red:
                    z.parent.color = black
                    y.color = black
                    gp.color = red
                    z = gp
                else:
                    if z is z.parent.left:
                        z = z.parent
                        self._right_rotate(z)
                    z.parent.color = black
                    z.parent.parent.color = red
                    self._left_rotate(z.parent.parent)
        self._root.color = black

    # -- recherche -----------------------------------------------
    def find(self, key: Any) -> Optional[RBNode]:
        nil = self._nil
        x = self._root
        while x is not nil:
            if key == x.key:
                return x
            x = x.left if key < x.key else x.right
        return None

    def min(self) -> Any:
        if self.is_empty:
            raise ValueError("arbre vide")
        return self._minimum(self._root).key

    def max(self) -> Any:
        if self.is_empty:
            raise ValueError("arbre vide")
        return self._maximum(self._root).key

    def _minimum(self, x: RBNode) -> RBNode:
        while x.left is not self._nil:
            x = x.left
        return x

    def _maximum(self, x: RBNode) -> RBNode:
        while x.right is not self._nil:
            x = x.right
        return x

    def successor(self, key: Any) -> Optional[Any]:
        nil = self._nil
        x = self.find(key)
        if x is None:
            return None
        if x.right is not nil:
            return self._minimum(x.right).key
        y = x.parent
        while y is not None and x is y.right:
            x, y = y, y.parent
        return None if y is None else y.key

    def predecessor(self, key: Any) -> Optional[Any]:
        nil = self._nil
        x = self.find(key)
        if x is None:
            return None
        if x.left is not nil:
            return self._maximum(x.left).key
        y = x.parent
        while y is not None and x is y.left:
            x, y = y, y.parent
        return None if y is None else y.key

    # -- suppression ---------------------------------------------
    def delete(self, key: Any) -> bool:
        z = self.find(key)
        if z is None:
            return False
        self._delete_node(z)
        self._size -= 1
        return True

    def _transplant(self, u: RBNode, v: RBNode) -> None:
        if u.parent is None:
            self._root = v
        elif u is u.parent.left:
            u.parent.left = v
        else:
            u.parent.right = v
        v.parent = u.parent

    def _delete_node(self, z: RBNode) -> None:
        nil = self._nil
        y = z
        y_original_color = y.color
        if z.left is nil:
            x = z.right
            self._transplant(z, z.right)
        elif z.right is nil:
            x = z.left
            self._transplant(z, z.left)
        else:
            y = self._minimum(z.right)
            y_original_color = y.color
            x = y.right
            if y.parent is z:
                x.parent = y
            else:
                self._transplant(y, y.right)
                y.right = z.right
                y.right.parent = y
            self._transplant(z, y)
            y.left = z.left
            y.left.parent = y
            y.color = z.color
        if y_original_color is Color.BLACK:
            self._delete_fixup(x)

    def _delete_fixup(self, x: RBNode) -> None:
        red, black = Color.RED, Color.BLACK
        nil = self._nil
        while x is not self._root and x.color is black:
            if x is x.parent.left:
                w = x.parent.right
                if w.color is red:                                  # cas 1
                    w.color = black
                    x.parent.color = red
                    self._left_rotate(x.parent)
                    w = x.parent.right
                if w.left.color is black and w.right.color is black:  # cas 2
                    w.color = red
                    x = x.parent
                else:
                    if w.right.color is black:                      # cas 3
                        w.left.color = black
                        w.color = red
                        self._right_rotate(w)
                        w = x.parent.right
                    w.color = x.parent.color                           # cas 4
                    x.parent.color = black
                    w.right.color = black
                    self._left_rotate(x.parent)
                    x = self._root
            else:
                w = x.parent.left
                if w.color is red:
                    w.color = black
                    x.parent.color = red
                    self._right_rotate(x.parent)
                    w = x.parent.left
                if w.right.color is black and w.left.color is black:
                    w.color = red
                    x = x.parent
                else:
                    if w.left.color is black:
                        w.right.color = black
                        w.color = red
                        self._left_rotate(w)
                        w = x.parent.left
                    w.color = x.parent.color
                    x.parent.color = black
                    w.left.color = black
                    self._right_rotate(x.parent)
                    x = self._root
        x.color = black

    def clear(self) -> None:
        self._root = self._nil
        self._size = 0

    # -- parcours ------------------------------------------------
    def inorder(self) -> List[Any]:
        """Ordre croissant."""
        out: List[Any] = []
        nil = self._nil
        stack, cur = [], self._root
        while stack or cur is not nil:
            while cur is not nil:
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            out.append(cur.key)
            cur = cur.right
        return out

    def preorder(self) -> List[Any]:
        out: List[Any] = []
        nil = self._nil
        stack = [self._root] if self._root is not nil else []
        while stack:
            n = stack.pop()
            out.append(n.key)
            if n.right is not nil:
                stack.append(n.right)
            if n.left is not nil:
                stack.append(n.left)
        return out

    def levelorder(self) -> List[Any]:
        out: List[Any] = []
        nil = self._nil
        queue = [self._root] if self._root is not nil else []
        i = 0
        while i < len(queue):
            n = queue[i]
            i += 1
            out.append(n.key)
            if n.left is not nil:
                queue.append(n.left)
            if n.right is not nil:
                queue.append(n.right)
        return out

    def nodes(self) -> List[RBNode]:
        """Tous les nœuds réels, en préordre (sentinelle exclue)."""
        out: List[RBNode] = []
        nil = self._nil
        stack = [self._root] if self._root is not nil else []
        while stack:
            n = stack.pop()
            out.append(n)
            if n.right is not nil:
                stack.append(n.right)
            if n.left is not nil:
                stack.append(n.left)
        return out

    # -- mesures -------------------------------------------------
    @property
    def height(self) -> int:
        """Hauteur : arbre vide = -1 ; nœuds noirs comptés."""
        nil = self._nil
        if self._root is nil:
            return -1
        best, stack = 0, [(self._root, 0)]
        while stack:
            n, d = stack.pop()
            best = max(best, d)
            if n.left is not nil:
                stack.append((n.left, d + 1))
            if n.right is not nil:
                stack.append((n.right, d + 1))
        return best

    @property
    def black_height(self) -> int:
        """Nombre de nœuds noirs sur un chemin racine→feuille (NIL inclus)."""
        return self._black_height(self._root)

    def _black_height(self, n: RBNode) -> int:
        if n is self._nil:
            return 1
        return self._black_height(n.left) + (1 if n.color is Color.BLACK else 0)

    # -- vérification des invariants ------------------------------
    def validate(self) -> None:
        """Lève ValueError si l'un des 5 invariants est violé."""
        nil = self._nil
        if self._root is nil:
            return
        if self._root.color is Color.RED:
            raise ValueError("invariant 2 violé : racine rouge")
        if self._root.parent is not None:
            raise ValueError("la racine a un parent")
        self._check(self._root, None, None)
        if self._check_black_height(nil, self._root) < 0:
            raise ValueError("invariant 5 violé : hauteurs noires différentes")

    def _check(self, n: RBNode, lo: Any, hi: Any) -> None:
        if n is self._nil:
            return
        if lo is not None and not (lo < n.key):
            raise ValueError(f"propriété ABR violée : {n.key!r} <= {lo!r}")
        if hi is not None and not (n.key < hi):
            raise ValueError(f"propriété ABR violée : {n.key!r} >= {hi!r}")
        if n.color is Color.RED:
            if n.left.color is Color.RED:
                raise ValueError(f"invariant 4 violé (rouge-rouge) : {n.key!r}/{n.left.key!r}")
            if n.right.color is Color.RED:
                raise ValueError(f"invariant 4 violé (rouge-rouge) : {n.key!r}/{n.right.key!r}")
        self._check(n.left, lo, n.key)
        self._check(n.right, n.key, hi)

    def _check_black_height(self, nil: RBNode, n: RBNode) -> int:
        if n is nil:
            return 1
        l = self._check_black_height(nil, n.left)
        r = self._check_black_height(nil, n.right)
        if l < 0 or r < 0 or l != r:
            return -1
        return l + (1 if n.color is Color.BLACK else 0)

    # -- affichage ------------------------------------------------
    def __str__(self) -> str:
        lines: List[str] = []
        nil = self._nil

        def rec(n: RBNode, prefix: str, is_last: bool) -> None:
            if n is nil:
                return
            lines.append(prefix + ("└── " if is_last else "├── ") + repr(n))
            child_prefix = prefix + ("    " if is_last else "│   ")
            kids = [c for c in (n.left, n.right) if c is not nil]
            for i, c in enumerate(kids):
                rec(c, child_prefix, i == len(kids) - 1)

        rec(self._root, "", True)
        return "\n".join(lines)

    def __repr__(self) -> str:
        return f"RBTree(size={self._size}, height={self.height}, black_height={self.black_height})"
