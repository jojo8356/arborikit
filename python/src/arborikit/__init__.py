"""
arborikit — arbres équilibrés et non équilibrés, sans aucune dépendance.

Trois structures, une API cohérente :
- GeneralTree : arbre généraliste (n-aire), nœuds dynamiques
- BST         : arbre binaire de recherche NON équilibré
- RBTree      : arbre rouge-noir (ARN), équilibré, avec invariants vérifiables

Zéro dépendance d'exécution. Python 3.8+.
"""

from __future__ import annotations

from .general_tree import GeneralTree, GeneralNode
from .bst import BST
from .rbtree import RBTree, Color

__version__ = "0.1.0"
__all__ = [
    "GeneralTree",
    "GeneralNode",
    "BST",
    "RBTree",
    "Color",
    "__version__",
]
