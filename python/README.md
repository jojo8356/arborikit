# arborikit (Python)

> Arbres généralistes, ABR non équilibrés et ARN — **zéro dépendance**.

```python
from arborikit import GeneralTree, BST, RBTree

# Arbre généraliste
t = GeneralTree("CEO")
t.add("CEO", "VP1")
t.add("CEO", "VP2")
t.add("VP1", "GM1")
print(t)                 # affichage arborescent
print(t.preorder())      # ['CEO', 'VP1', 'GM1', 'VP2']

# ABR non équilibré
b = BST([50, 30, 70, 20, 40])
b.insert(60)
print(b.inorder())       # [20, 30, 40, 50, 60, 70]
print(b.height, b.is_degenerate)

# ARN (rouge-noir), observable
r = RBTree([50, 30, 70, 20, 40, 60, 80])
r.validate()             # vérifie les 5 invariants
print(r)                 # 50(N), 30(R), ...
print(r.inorder(), r.black_height)
```

## API

| Structure | Méthodes |
|---|---|
| `GeneralTree` | `add`, `prune`, `find`, `preorder`, `postorder`, `levelorder`, `leaves`, `size`, `height` |
| `BST` | `insert`, `delete`, `find`, `min`, `max`, `successor`, `predecessor`, `inorder`, `preorder`, `postorder`, `levelorder`, `height`, `is_degenerate`, `validate` |
| `RBTree` | idem `BST` + `root`, `nodes`, `black_height`, `validate`, couleurs `Color.RED/BLACK` |

## Installation

```bash
pip install arborikit
```

## Licence

MIT.
