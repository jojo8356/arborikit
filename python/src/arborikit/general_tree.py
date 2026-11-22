"""Arbre généraliste (n-aire) : chaque nœud a 0..n enfants et un parent."""

from __future__ import annotations

from typing import Any, Iterator, List, Optional


class GeneralNode:
    """Nœud d'arbre généraliste. `children` est ordonné (insertion)."""

    __slots__ = ("data", "parent", "children")

    def __init__(self, data: Any, parent: Optional["GeneralNode"] = None) -> None:
        self.data = data
        self.parent = parent
        self.children: List[GeneralNode] = []

    def add_child(self, data: Any) -> "GeneralNode":
        child = GeneralNode(data, self)
        self.children.append(child)
        return child

    def add_node(self, node: "GeneralNode") -> "GeneralNode":
        """Rattache un nœud existant (détaché d'abord de son parent)."""
        if node.parent is not None:
            node.parent.children.remove(node)
        node.parent = self
        self.children.append(node)
        return node

    def remove_child(self, node: "GeneralNode") -> bool:
        try:
            self.children.remove(node)
        except ValueError:
            return False
        node.parent = None
        return True

    @property
    def is_leaf(self) -> bool:
        return not self.children

    @property
    def is_root(self) -> bool:
        return self.parent is None

    @property
    def depth(self) -> int:
        """Profondeur : racine = 0."""
        d, cur = 0, self
        while cur.parent is not None:
            d += 1
            cur = cur.parent
        return d

    @property
    def height(self) -> int:
        """Hauteur du sous-arbre : feuille = 0."""
        if not self.children:
            return 0
        return 1 + max(c.height for c in self.children)

    @property
    def subtree_size(self) -> int:
        return 1 + sum(c.subtree_size for c in self.children)

    def path_from_root(self) -> List[Any]:
        path, cur = [], self
        while cur is not None:
            path.append(cur.data)
            cur = cur.parent
        return path[::-1]

    def __iter__(self) -> Iterator["GeneralNode"]:
        return iter(self.children)

    def __repr__(self) -> str:
        return f"GeneralNode({self.data!r})"


class GeneralTree:
    """Arbre généraliste enveloppant un nœud racine."""

    def __init__(self, root_data: Any) -> None:
        self.root = GeneralNode(root_data)

    # -- mesures -------------------------------------------------
    def __len__(self) -> int:
        return self.root.subtree_size

    def __bool__(self) -> bool:
        return True

    @property
    def size(self) -> int:
        return len(self)

    @property
    def height(self) -> int:
        return self.root.height

    # -- parcours ------------------------------------------------
    def preorder(self) -> List[Any]:
        """Nœud puis enfants."""
        out: List[Any] = []
        stack = [self.root]
        while stack:
            n = stack.pop()
            out.append(n.data)
            stack.extend(reversed(n.children))
        return out

    def postorder(self) -> List[Any]:
        """Enfants puis nœud."""
        out: List[Any] = []
        root = self.root

        def rec(n: GeneralNode) -> None:
            for c in n.children:
                rec(c)
            out.append(n.data)

        rec(root)
        return out

    def levelorder(self) -> List[Any]:
        """BFS, niveau par niveau."""
        out: List[Any] = []
        queue = [self.root]
        i = 0
        while i < len(queue):
            n = queue[i]
            i += 1
            out.append(n.data)
            queue.extend(n.children)
        return out

    def leaves(self) -> List[Any]:
        return [n.data for n in self._walk() if n.is_leaf]

    def _walk(self) -> Iterator[GeneralNode]:
        stack = [self.root]
        while stack:
            n = stack.pop()
            yield n
            stack.extend(reversed(n.children))

    # -- recherche ----------------------------------------------
    def find(self, data: Any) -> Optional[GeneralNode]:
        for n in self._walk():
            if n.data == data:
                return n
        return None

    # -- édition -------------------------------------------------
    def add(self, parent_data: Any, data: Any) -> GeneralNode:
        """Raccourci : ajoute `data` sous le nœud contenant `parent_data`."""
        p = self.find(parent_data)
        if p is None:
            raise KeyError(f"nœud parent introuvable : {parent_data!r}")
        return p.add_child(data)

    def prune(self, data: Any) -> bool:
        """Supprime le nœud et tout son sous-arbre."""
        n = self.find(data)
        if n is None or n.parent is None:
            return False
        return n.parent.remove_child(n)

    # -- affichage ------------------------------------------------
    def __str__(self) -> str:
        lines: List[str] = []

        def rec(n: GeneralNode, prefix: str, is_last: bool) -> None:
            lines.append(prefix + ("└── " if is_last else "├── ") + str(n.data))
            child_prefix = prefix + ("    " if is_last else "│   ")
            for i, c in enumerate(n.children):
                rec(c, child_prefix, i == len(n.children) - 1)

        rec(self.root, "", True)
        return "\n".join(lines)

    def __repr__(self) -> str:
        return f"GeneralTree({self.root.data!r}, size={len(self)})"
