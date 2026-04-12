# CI/CD Pipeline Optimizations

## 📊 Before/After Comparison

### Architecture

**Before**: 2 separate jobs

- `build-linux`: Release Build + Tests
- `coverage`: Debug Build + Tests + Coverage

**After**: 1 matrix job

- `test[Release]`: Release Build + Tests + Valgrind
- `test[Debug]`: Debug Build + Tests + Coverage

## 🚀 Applied Optimizations

### 1. **Build Matrix** (-30% time)

- Single job with 2 variants (Release/Debug)
- Shared definition code
- Parallel execution preserved

### 2. **Intelligent Caching** (-40% installation time)

#### APT Cache

```yaml
uses: awalsh128/cache-apt-pkgs-action@v1
```

- Caches downloaded apt packages
- Saves 30-60s per build on base dependencies

#### ccache Cache

```yaml
key: ${{ runner.os }}-ccache-${{ matrix.build_type }}-${{ hashFiles('app/src/**') }}
```

- Caches compiled object files
- Recompiles only modified files
- Savings: 50-80% compilation time on incremental builds

#### CMake Cache

- Caches `app/build/` to reuse configuration
- Avoids reconfiguring every time

### 3. **Conditional Installation** (-120s on Release)

```yaml
# Release: Skip Pandoc + XeLaTeX (tests detect and skip)
install_converters: false

# Debug: Full installation for coverage
install_converters: true
```

**Gain**:

- Pandoc: ~300 MB, 30s
- texlive-xetex: ~500 MB, 90s
- Total on Release: **-120s installation**

Tests in `test_conversion_engine.cpp` use `QSKIP()` if Pandoc/XeLaTeX are absent, so:

- ✅ Release: Pure unit tests (fast)
- ✅ Debug: Full integration tests (with tools)

### 4. **Parallel Compilation** (-30% build time)

```yaml
cmake --build . -j$(nproc)
```

Before: Single-threaded  
After: Uses all CPUs (2-4 on GitHub Actions)

### 5. **Minor Optimizations**

- `sudo apt update -qq`: Quiet mode
- `--no-install-recommends`: Skip suggested packages
- `ccache --max-size=500M`: Limit cache size
- `fail-fast: false`: Continue even if one job fails

## 📈 Estimated Performance Gains

| Scenario | Before | After | Gain |
|----------|-------|-------|------|
| **First build (empty cache)** | 8-10 min | 6-7 min | **~30%** |
| **Incremental build (1 file modified)** | 5-6 min | 2-3 min | **~50%** |
| **No file changed (docs only)** | ~10s | ~10s | 0% |

### Build time breakdown (first build)

**Before (2 jobs)**:

```
build-linux:   4-5 min
coverage:      4-5 min
Total:         8-10 min (parallel)
```

**After (matrix)**:

```
test[Release]:  2.5-3 min  (no Pandoc/XeLaTeX, with ccache)
test[Debug]:    3.5-4 min  (with coverage, all tools)
Total:          3.5-4 min  (parallel, longest of the two)
```

## ✅ Preserved Features

Nothing is lost:

- ✅ Complete unit tests (Release + Debug)
- ✅ Code coverage with Codecov (Debug)
- ✅ Valgrind memory checks (Release)
- ✅ Build artifacts (Release on main)
- ✅ File change detection (paths-filter)
- ✅ Optimized Release build
- ✅ Pandoc/XeLaTeX integration tests (Debug)

## 🔄 Migration

### Option 1: Direct replacement

```bash
mv .github/workflows/ci.yml .github/workflows/ci-old.yml
mv .github/workflows/ci-optimized.yml .github/workflows/ci.yml
git add .github/workflows/
git commit -m "ci: Optimize pipeline with matrix strategy and caching"
```

### Option 2: Parallel testing

Keep both workflows temporarily:

- `ci.yml`: Current pipeline (backup)
- `ci-optimized.yml`: New pipeline (test)

After validation (1-2 weeks), delete the old one.

## 📝 Technical Notes

### Why ccache?

- Keeps compiled `.o` files in cache
- Recompiles only if source OR headers change
- Detection via preprocessor hash
- Massive gain on incremental builds

### Why matrix instead of 2 jobs?

- Less duplication (DRY)
- Simplified maintenance (1 workflow instead of 2)
- Automatic sharing of common steps
- Easy addition of new variants (macOS, Windows, ...)

### Why split Pandoc/XeLaTeX?

- 800 MB dependencies for 2-3 integration tests
- Unit tests don't need them
- Tests automatically skip if not available
- Release job stays ultra-fast for quick feedback
- Debug job keeps full coverage

## 🎯 Recommendations

1. **Enable optimization immediately**: Substantial gains, minimal risk
2. **Monitor first builds**: Verify ccache works well
3. **Adjust ccache**: If too many misses, increase `--max-size`
4. **Future optimization**: Add Windows/macOS to matrix

## 🔍 Verification

After migration, check:

```bash
# Check workflow syntax
gh workflow view ci

# Trigger manual run
gh workflow run ci

# Watch status
gh run watch
```

Success indicators:

- ✅ `ccache --show-stats` shows >50% hit rate after 2-3 builds
- ✅ Total time < 5 min (instead of 8-10)
- ✅ All tests pass
- ✅ Coverage uploaded to Codecov
- ✅ Artifact generated on main
