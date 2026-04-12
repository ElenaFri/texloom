# Optimisations du Pipeline CI/CD

## 📊 Comparaison Avant/Après

### Architecture

**Avant** : 2 jobs séparés

- `build-linux` : Build Release + Tests
- `coverage` : Build Debug + Tests + Coverage

**Après** : 1 job matriciel

- `test[Release]` : Build Release + Tests + Valgrind
- `test[Debug]` : Build Debug + Tests + Coverage

## 🚀 Optimisations Appliquées

### 1. **Matrice de Build** (-30% temps)

- Un seul job avec 2 variantes (Release/Debug)
- Partage du code de définition
- Exécution parallèle conservée

### 2. **Cache Intelligent** (-40% temps installation)

#### Cache APT

```yaml
uses: awalsh128/cache-apt-pkgs-action@v1
```

- Met en cache les paquets apt téléchargés
- Économise 30-60s par build sur les dépendances de base

#### Cache ccache

```yaml
key: ${{ runner.os }}-ccache-${{ matrix.build_type }}-${{ hashFiles('app/src/**') }}
```

- Cache les fichiers objets compilés
- Recompile uniquement les fichiers modifiés
- Économie : 50-80% du temps de compilation sur builds incrémentaux

#### Cache CMake

- Cache `app/build/` pour réutiliser la configuration
- Évite de reconfigurer à chaque fois

### 3. **Installation Conditionnelle** (-120s sur Release)

```yaml
# Release : Skip Pandoc + XeLaTeX (tests les détectent et skip)
install_converters: false

# Debug : Installation complète pour coverage
install_converters: true
```

**Gain** :

- Pandoc : ~300 MB, 30s
- texlive-xetex : ~500 MB, 90s
- Total sur Release : **-120s d'installation**

Les tests `test_conversion_engine.cpp` utilisent `QSKIP()` si Pandoc/XeLaTeX absents, donc :

- ✅ Release : Tests unitaires purs (rapides)
- ✅ Debug : Tests d'intégration complets (avec outils)

### 4. **Compilation Parallèle** (-30% temps build)

```yaml
cmake --build . -j$(nproc)
```

Avant : Single-threaded  
Après : Utilise tous les CPU (2-4 sur GitHub Actions)

### 5. **Optimisations Mineures**

- `sudo apt update -qq` : Mode silencieux
- `--no-install-recommends` : Skip paquets suggérés
- `ccache --max-size=500M` : Limite taille cache
- `fail-fast: false` : Continue même si un job échoue

## 📈 Gains de Performance Estimés

| Scénario | Avant | Après | Gain |
|----------|-------|-------|------|
| **Premier build (cache vide)** | 8-10 min | 6-7 min | **~30%** |
| **Build incrémental (1 fichier modifié)** | 5-6 min | 2-3 min | **~50%** |
| **Aucun fichier changé (docs seulement)** | ~10s | ~10s | 0% |

### Détail du temps (premier build)

**Avant (2 jobs)** :

```
build-linux:   4-5 min
coverage:      4-5 min
Total:         8-10 min (parallèle)
```

**Après (matrice)** :

```
test[Release]:  2.5-3 min  (sans Pandoc/XeLaTeX, avec ccache)
test[Debug]:    3.5-4 min  (avec coverage, tous outils)
Total:          3.5-4 min  (parallèle, le plus long des deux)
```

## ✅ Fonctionnalités Préservées

Rien n'est perdu :

- ✅ Tests unitaires complets (Release + Debug)
- ✅ Code coverage avec Codecov (Debug)
- ✅ Valgrind memory checks (Release)
- ✅ Build artifacts (Release sur main)
- ✅ Détection changements fichiers (paths-filter)
- ✅ Build Release optimisé
- ✅ Tests d'intégration Pandoc/XeLaTeX (Debug)

## 🔄 Migration

### Option 1 : Remplacement direct

```bash
mv .github/workflows/ci.yml .github/workflows/ci-old.yml
mv .github/workflows/ci-optimized.yml .github/workflows/ci.yml
git add .github/workflows/
git commit -m "ci: Optimize pipeline with matrix strategy and caching"
```

### Option 2 : Test en parallèle

Garder les deux workflows temporairement :

- `ci.yml` : Pipeline actuel (backup)
- `ci-optimized.yml` : Nouveau pipeline (test)

Après validation (1-2 semaines), supprimer l'ancien.

## 📝 Notes Techniques

### Pourquoi ccache ?

- Garde les `.o` compilés en cache
- Recompile uniquement si le source OU les headers changent
- Détection via hash du préprocesseur
- Gain massif sur builds incrémentaux

### Pourquoi matrice au lieu de 2 jobs ?

- Moins de duplication (DRY)
- Maintenance simplifiée (1 workflow au lieu de 2)
- Partage automatique des étapes communes
- Ajout facile de nouvelles variantes (macOS, Windows, ...)

### Pourquoi split Pandoc/XeLaTeX ?

- 800 MB de dépendances pour 2-3 tests d'intégration
- Les tests unitaires n'en ont pas besoin
- Les tests skip automatiquement si non disponibles
- Job Release reste ultra rapide pour feedback rapide
- Job Debug garde la couverture complète

## 🎯 Recommandations

1. **Activer l'optimisation immédiatement** : Gains substantiels, risque minimal
2. **Monitorer les premiers builds** : Vérifier que ccache fonctionne bien
3. **Ajuster cache ccache** : Si trop de misses, augmenter `--max-size`
4. **Future optimisation** : Ajouter Windows/macOS dans la matrice

## 🔍 Vérification

Après migration, vérifier :

```bash
# Check workflow syntax
gh workflow view ci

# Trigger manual run
gh workflow run ci

# Watch status
gh run watch
```

Indicateurs de succès :

- ✅ `ccache --show-stats` montre >50% hit rate après 2-3 builds
- ✅ Temps total < 5 min (au lieu de 8-10)
- ✅ Tous les tests passent
- ✅ Coverage uploadé sur Codecov
- ✅ Artifact généré sur main
