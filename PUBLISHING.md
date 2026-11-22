# Publier Arborikit — commandes exactes

Quatre bibliothèques, plusieurs registres. Le dépôt est
`github.com/jojo8356/arborikit` et le groupe Maven prévu est `io.github.jojo8356`.
La publication n'est pas encore effectuée : elle nécessite les comptes et
configurations ci-dessous. Vérifier que ces noms sont disponibles et que le
namespace Maven est bien validé avant toute publication.

Le workflow de release se déclenche uniquement sur un tag `vMAJOR.MINOR.PATCH`.
Avant de créer le tag sur le commit à publier, synchroniser les versions dans
`python/pyproject.toml`, `python/src/arborikit/__init__.py`, `java/pom.xml`,
les deux `CMakeLists.txt`, `c/include/arborikit/version.h` (y compris les trois
composants numériques), les deux recettes Conan et les deux `vcpkg.json`.
Vérifier avec `python scripts/check_release_version.py v0.1.0`. Le workflow
refuse les versions incohérentes, mais ne remplace pas les tests préalables.

Les publications externes sont irréversibles pour une version donnée : lancer
les tests et la répétition TestPyPI avant de pousser le tag. Configurer d'abord
l'environnement PyPI et les secrets Sonatype décrits plus bas. Ensuite :

```bash
git tag v0.1.0
git push origin v0.1.0
```

Les jobs PyPI et Maven s'exécutent indépendamment : vérifier chacun dans Actions.
La release GitHub contient les archives natives même si une publication échoue.
Maven Central exige ensuite une validation manuelle sur le Central Portal.

---

## 1. Python → PyPI

### 1.1 Construire et vérifier

```bash
cd python
python -m pip install --upgrade pip build twine
rm -rf dist build
python -m build                      # sdist + wheel dans dist/
python -m twine check dist/*         # vérifie le README, les métadonnées
```

### 1.2 Répéter sur TestPyPI (obligatoire la première fois)

Crée un token sur <https://test.pypi.org/manage/account/token/> :

```bash
python -m twine upload --repository testpypi dist/*
# username: __token__
# password: pypi-AgEIcHlwaS5vcmc...

# Test d'installation réelle depuis TestPyPI
python -m venv /tmp/venvtest && /tmp/venvtest/bin/pip install \
  --index-url https://test.pypi.org/simple/ \
  --extra-index-url https://pypi.org/simple/ arborikit
/tmp/venvtest/bin/python -c "import arborikit; print(arborikit.__version__)"
```

### 1.3 Publier sur PyPI

```bash
python -m twine upload dist/*
```

Ou, recommandé, avec un fichier `~/.pypirc` (ne jamais mettre le token dans un dépôt) :

```ini
[distutils]
index-servers =
    pypi
    testpypi

[pypi]
username = __token__
password = pypi-...

[testpypi]
repository = https://test.pypi.org/legacy/
username = __token__
password = pypi-...
```

> Une version publiée sur PyPI ne peut **jamais** être remplacée : il faut
> incrémenter `version` dans `pyproject.toml` (et `__version__`).

### 1.4 Automatique : GitHub Actions + trusted publishing

Dans PyPI → *Publishing* → *Add a new pending publisher* :

| Champ | Valeur |
|---|---|
| PyPI Project Name | `arborikit` |
| Owner | `jojo8356` |
| Repository name | `arborikit` |
| Workflow name | `release.yml` |
| Environment name | `pypi` |

Puis :

```bash
git tag v0.1.0 && git push origin v0.1.0
```

Le job `python` de `.github/workflows/release.yml` publie sans aucune clé
stockée (OIDC). C'est la méthode que PyPI recommande aujourd'hui.

---

## 2. Java → Maven Central

### 2.1 Préparer l'identité

Le `groupId` est `io.github.jojo8356` et les URLs pointent vers ce dépôt.
Le namespace doit être validé par son propriétaire dans Central Portal.
### 2.2 Compte Sonatype (Central Portal)

1. <https://central.sonatype.com/> → *Sign in with GitHub*
2. *Add namespace* → `io.github.jojo8356` → vérification automatique via GitHub
3. *Account* → *Generate User Token* → tu obtiens un `username` / `password`

### 2.3 Clé GPG (obligatoire sur Central)

```bash
gpg --full-generate-key                   # RSA 4096, expiration 2 ans
gpg --list-secret-keys --keyid-format LONG
# repérer l'ID : sec   rsa4096/ABCDEF1234567890

gpg --keyserver keyserver.ubuntu.com --send-keys ABCDEF1234567890
```

### 2.4 `~/.m2/settings.xml`

```xml
<settings>
  <servers>
    <server>
      <id>central</id>
      <username>TON_TOKEN_USERNAME</username>
      <password>TON_TOKEN_PASSWORD</password>
    </server>
  </servers>
</settings>
```

### 2.5 Publier

```bash
cd java
mvn -B clean verify                       # tests d'abord
mvn -B -P release deploy                  # signe (GPG) + upload vers Central Portal

# Si la publication n'est pas automatique (autoPublish=false dans le pom) :
# aller sur https://central.sonatype.com/publishing et cliquer "Publish"
```

Consommation ensuite :

```xml
<dependency>
  <groupId>io.github.jojo8356</groupId>
  <artifactId>arborikit</artifactId>
  <version>0.1.0</version>
</dependency>
```

> Délai : quelques minutes à quelques heures avant apparition dans Maven Central.

### 2.6 Version snapshot pour tester sans publier

```bash
cd java
mvn -B versions:set -DnewVersion=0.2.0-SNAPSHOT
mvn -B clean install                     # dispo dans ~/.m2 pour tes autres projets
```

---

## 3. C et C++ → GitHub Releases + gestionnaires de paquets

Il n'existe pas de « npm du C ». La distribution passe par :

1. une archive GitHub Release (fiable, universelle) ;
2. des recettes pour les gestionnaires (Conan, vcpkg) — plus long, en PR.

### 3.1 Archive GitHub Release (automatique)

```bash
git tag v0.1.0 && git push origin v0.1.0
```

Le job `native` de `release.yml` produit `arborikit-c-0.1.0.tar.gz` et
`arborikit-cpp-0.1.0.tar.gz`, attachés à la Release.

### 3.2 Installation « système » pour les utilisateurs Unix

```bash
# C
cmake -S c -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build --prefix /usr/local
# -> /usr/local/lib/libarborikit.a, /usr/local/include/arborikit/*.h,
#    /usr/local/lib/cmake/arborikit/*.cmake, /usr/local/lib/pkgconfig/arborikit.pc

# C++ (header-only : rien à compiler)
cmake -S cpp -B build_cpp -DCMAKE_BUILD_TYPE=Release
cmake --build build_cpp -j
sudo cmake --install build_cpp --prefix /usr/local
# -> /usr/local/include/arborikit/*.hpp
```

Côté consommateur :

```cmake
find_package(arborikit 0.1 REQUIRED)         # lib C
target_link_libraries(mon_app PRIVATE arborikit::arborikit)

find_package(arborikit_cpp 0.1 REQUIRED)     # lib C++
target_link_libraries(mon_app PRIVATE arborikit::cpp)
```

```bash
# ou via pkg-config, sans CMake
cc mon_app.c $(pkg-config --cflags --libs arborikit) -o mon_app
```

### 3.3 Utilisation sans installation : `FetchContent` (C++)

```cmake
include(FetchContent)
FetchContent_Declare(arborikit
  GIT_REPOSITORY https://github.com/jojo8356/arborikit.git
  GIT_TAG        v0.1.0
  SOURCE_SUBDIR  cpp)
FetchContent_MakeAvailable(arborikit)
target_link_libraries(mon_app PRIVATE arborikit::cpp)
```

### 3.4 Conan 2

Test local de la recette (dépôt local, pas besoin de ConanCenter) :

```bash
pip install conan
conan profile detect --force
conan create c/   --version=0.1.0      # lib C
conan create cpp/ --version=0.1.0      # lib C++, header-only
```

Consommation :

```ini
# conanfile.txt
[requires]
arborikit/0.1.0
arborikit-cpp/0.1.0

[generators]
CMakeDeps
CMakeToolchain
```

Pour ConanCenter, ces recettes locales ne suffisent pas : adapter les recettes
aux exigences de ConanCenter (téléchargement des sources par version et hash,
`conandata.yml`, tests et structure attendue), puis ouvrir une PR sur
<https://github.com/conan-io/conan-center-index>. Ne pas copier directement
ces recettes locales en supposant qu'elles fonctionneront sans les sources.

### 3.5 vcpkg

```bash
# 1. fork + clone microsoft/vcpkg
git clone https://github.com/<ton-pseudo>/vcpkg.git
cd vcpkg

# 2. créer le port
mkdir -p ports/arborikit
cp ../arborikit/c/vcpkg.json ports/arborikit/
cp ../arborikit/LICENSE ports/arborikit/
cat > ports/arborikit/portfile.cmake <<'CMAKE'
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO jojo8356/arborikit
    REF "v${VERSION}"
    SHA512 0   # remplacer par le hash affiché par vcpkg, il te le donne
    HEAD_REF main
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}/c")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME arborikit CONFIG_PATH lib/cmake/arborikit)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
CMAKE

# 3. version de référence
cat > ports/arborikit/vcpkg.json <<'JSON'
{
  "name": "arborikit",
  "version": "0.1.0",
  "description": "General trees, unbalanced BSTs and red-black trees for C",
  "homepage": "https://github.com/jojo8356/arborikit",
  "license": "MIT",
  "dependencies": [
    { "name": "vcpkg-cmake", "host": true },
    { "name": "vcpkg-cmake-config", "host": true }
  ]
}
JSON

# 4. tester
./bootstrap-vcpkg.sh
vcpkg install arborikit --triplet x64-linux
vcpkg x-add-version arborikit

# 5. PR
git checkout -b add-arborikit
git add ports/arborikit versions/
git commit -m "[arborikit] Add new port (0.1.0)"
git push origin add-arborikit
```

Puis PR sur <https://github.com/microsoft/vcpkg>.

---

## 4. Récapitulatif des commandes par langage

```bash
# ---------- PYTHON ----------
cd python && python -m build && python -m twine check dist/* \
  && python -m twine upload --repository testpypi dist/* \
  && python -m twine upload dist/*

# ---------- JAVA ----------
cd java && mvn -B clean verify && mvn -B -P release deploy

# ---------- C ----------
cd c && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  && cmake --build build -j && (cd build && ctest --output-on-failure) \
  && sudo cmake --install build --prefix /usr/local

# ---------- C++ ----------
cd cpp && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  && cmake --build build -j && (cd build && ctest --output-on-failure) \
  && sudo cmake --install build --prefix /usr/local

# ---------- TOUT EN UN COUP (après tag) ----------
git tag v0.1.0 && git push origin v0.1.0
# -> release.yml publie PyPI, envoie Java au Central Portal et crée les archives GitHub
# -> vérifier les jobs puis publier manuellement le bundle sur Central Portal
```

---

## 5. Secrets GitHub à configurer

| Secret | Où l'obtenir | Utilisé par |
|---|---|---|
| `SONATYPE_USERNAME` | Central Portal → Account → User Token | job `java` |
| `SONATYPE_PASSWORD` | idem | job `java` |
| `MAVEN_GPG_PRIVATE_KEY` | `gpg --armor --export-secret-keys ABCDEF1234567890` | job `java` |
| `MAVEN_GPG_PASSPHRASE` | la passphrase choisie à la création de la clé | job `java` |

PyPI : **aucun secret** (trusted publishing OIDC, plus sûr).

---

## 6. À ne jamais faire

- Publier un `-SNAPSHOT` sur Maven Central (rejeté).
- Réutiliser une version PyPI déjà publiée.
- Committer `~/.pypirc`, un token, ou une clé GPG privée.
- Publier sans `LICENSE` : PyPI et Central le vérifient.
- Oublier d'attacher `sources.jar` et `javadoc.jar` : Central refuse le bundle
  (déjà configuré dans `java/pom.xml`).
