# Changelog

Format : [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/).
Versionnage : [SemVer](https://semver.org/lang/fr/).

## [Unreleased]

## [0.1.0] - 2026-11-22

### Ajouté

- **Python** (`arborikit`) : `GeneralTree`, `BST`, `RBTree` + `Color`.
  Aucune dépendance, Python 3.8+.
- **Java** (`io.github.arborikit`) : `GeneralTree<T>`, `BST<T>`, `RBTree<T>`
  compatibles Java 8. `validate()` vérifie les 5 invariants de l'ARN.
- **C** (C99) : API préfixée `gt_`, `bst_`, `rb_`, comparateurs génériques
  `void *`, zéro dépendance hors libc.
- **C++** (`arborikit::`) : header-only C++11, `GeneralTree<T>`, `BST<T>`,
  `RBTree<T>` avec comparateur personnalisable.
- Packaging : `pyproject.toml`, `pom.xml`, CMake + pkg-config, recettes Conan 2,
  manifestes vcpkg.
- CI : tests des 4 langages, ASan/UBSan, test de consommation `find_package`.
- Release : publication PyPI (OIDC), Maven Central, archives GitHub Releases.

### Notes

- L'ARN est volontairement observable (`colorOf`, `nodes()`, `toTreeString()`),
  ce que `TreeMap` et `std::map` ne permettent pas.
- L'ABR non équilibré est volontairement conservé non équilibré : c'est son
  intérêt pédagogique.
