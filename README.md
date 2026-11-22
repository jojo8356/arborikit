# Arborikit

**Trois structures d'arbres, quatre langages, zéro dépendance.**

| | GeneralTree (n-aire) | BST (ABR non équilibré) | RBTree (ARN) |
|---|:---:|:---:|:---:|
| **Python** | `arborikit.GeneralTree` | `arborikit.BST` | `arborikit.RBTree` |
| **Java** | `io.github.arborikit.GeneralTree` | `io.github.arborikit.BST` | `io.github.arborikit.RBTree` |
| **C** | `gt_*` (`general_tree.h`) | `bst_*` (`bst.h`) | `rb_*` (`rbtree.h`) |
| **C++** | `arborikit::GeneralTree<T>` | `arborikit::BST<T>` | `arborikit::RBTree<T>` |

Chaque implémentation est **autonome** : aucune dépendance d'exécution, aucune
dépendance de build hors la bibliothèque standard du langage. Pas de GLib, pas de
Boost, pas de Guava, pas de sortedcontainers.

## Pourquoi ces quatre structures ?

La bibliothèque standard de chaque langage a des trous bien connus :

| | Arbre généraliste | ABR non équilibré | ARN observable |
|---|---|---|---|
| **Python** | ❌ (anytree, treelib à installer) | ❌ | ❌ (pas de dict trié en stdlib) |
| **Java** | ⚠️ `javax.swing.tree` (couplé à l'UI, non générique) | ❌ | ⚠️ `TreeMap` : existe mais opaque |
| **C** | ❌ (GLib `GNode` = dépendance externe) | ❌ (libavl, libtree externes) | ❌ (GLib `GTree` externe) |
| **C++** | ❌ (`tree.hh` = GPL, Boost = lourd) | ❌ (`std::map` est équilibré) | ⚠️ `std::map` : existe mais opaque |

Arborikit comble ces trous, avec la même API d'un langage à l'autre.

## Ce qui distingue Arborikit

- **Observable** : couleurs, rotations, nœuds et structure sont accessibles.
  `TreeMap` et `std::map` cachent tout.
- **Vérifiable** : `validate()` vérifie les 5 invariants de l'ARN. Aucun
  conteneur standard ne le propose.
- **Pédagogique** : `BST.height()` et `isDegenerate()` rendent visible la
  dégénérescence en O(n) sur une insertion triée.
- **Cohérent** : mêmes noms de méthodes (`insert`, `erase`/`delete`, `find`,
  `successor`, `predecessor`, `inorder`, `levelorder`, `height`, `validate`)
  dans les quatre langages.
- **Zéro dépendance** : c'est une contrainte de conception, pas un hasard.

## Installation

### Python

```bash
pip install arborikit
```

```python
from arborikit import GeneralTree, BST, RBTree

tree = GeneralTree("CEO")
tree.add("CEO", "VP1"); tree.add("VP1", "GM1")
print(tree.preorder())            # ['CEO', 'VP1', 'GM1']

bst = BST([50, 30, 70, 20])
bst.insert(60)
print(bst.inorder(), bst.height)  # [20, 30, 50, 60, 70] 2

rb = RBTree([50, 30, 70, 20, 40, 60, 80])
rb.validate()                     # 5 invariants vérifiés
print(rb)                         # 50(N) / 30(R) / ...
```

### Java

```xml
<dependency>
  <groupId>io.github.VOTRE_COMPTE</groupId>
  <artifactId>arborikit</artifactId>
  <version>0.1.0</version>
</dependency>
```

```java
import io.github.arborikit.*;

RBTree<Integer> rb = new RBTree<>(50, 30, 70, 20);
rb.validate();
System.out.println(rb.inorder());      // [20, 30, 50, 70]
System.out.println(rb.toTreeString()); // arbre avec couleurs
```

### C

```bash
cmake -S c -B build && cmake --build build -j && sudo cmake --install build
```

```c
#include <arborikit/rbtree.h>

rb_tree *t = rb_create(NULL);
rb_insert(t, (void *)50);
rb_insert(t, (void *)30);
rb_validate(t, NULL, 0);
printf("min = %ld\n", (long)rb_min(t));
rb_destroy(t);
```

```bash
cc app.c $(pkg-config --cflags --libs arborikit) -o app
```

### C++ (header-only)

```cmake
find_package(arborikit_cpp 0.1 REQUIRED)
target_link_libraries(app PRIVATE arborikit::cpp)
```

```cpp
#include <arborikit/arborikit.hpp>

arborikit::RBTree<int> rb{50, 30, 70, 20};
rb.validate();
std::cout << rb.toString();
```

## API commune

Les quatre libs exposent les mêmes opérations, avec les idiomes du langage :

| Opération | Python | Java | C | C++ |
|---|---|---|---|---|
| insérer | `insert(x)` | `insert(x)` | `bst_insert` / `rb_insert` | `insert(x)` |
| supprimer | `delete(x)` | `delete(x)` / `erase` | `bst_delete` / `rb_delete` | `erase(x)` |
| chercher | `find(x)` | `find(x)` | `*_find` | `find(x)` |
| contient | `x in t` | `contains(x)` | `*_contains` | `contains(x)` |
| trié | `inorder()` | `inorder()` | `*_inorder` | `inorder()` |
| largeur | `levelorder()` | `levelorder()` | `*_levelorder` | `levelorder()` |
| invariants | `validate()` | `validate()` | `rb_validate()` | `validate()` |
| afficher | `print(t)` | `toTreeString()` | `*_print_int` | `toString()` |

## Tests

| Langage | Commande | Tests |
|---|---|---|
| Python | `cd python && pytest tests -q` | 22 |
| Java | `cd java && mvn test` | 23 |
| C | `cd c && cc -Iinclude tests/test_arborikit.c src/*.c -o t && ./t` | asserts |
| C++ | `cd cpp && c++ -Iinclude tests/test_arborikit.cpp -o t && ./t` | asserts |

Les quatre suites sont aussi exécutées en ASan/UBSan dans la CI, et l'ARN est
testé en stress contre `TreeSet` / `std::set` / les collections Python.

## Structure du dépôt

```
arborikit/
├── python/     # paquet PyPI (src layout, pyproject.toml)
├── java/       # projet Maven (pom.xml prêt pour Maven Central)
├── c/          # lib C99 (CMake, pkg-config, Conan, vcpkg)
├── cpp/        # header-only C++11 (CMake, Conan, vcpkg)
├── .github/
│   └── workflows/
│       ├── ci.yml       # tests des 4 langages
│       └── release.yml  # publication PyPI + Maven Central + archives
├── PUBLISHING.md        # commandes exactes de publication
└── LICENSE              # MIT
```

## Licence

MIT. Contrairement à GNU libavl (GPL) ou `tree.hh` (GPLv3), Arborikit est
utilisable en contexte commercial sans contamination.
