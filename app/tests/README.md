# TexLoom Test Suite

This directory contains unit and integration tests for TexLoom components.

## Testing Strategy

### Objectives

- **Correctness**: Verify each component behaves as specified
- **Error handling**: Test failure scenarios (missing files, corrupted data, invalid inputs)
- **Edge cases**: Empty files, large files, special characters, Unicode
- **Signals**: Verify Qt signals are emitted correctly
- **Integration**: Test components with real external tools (Pandoc, XeLaTeX)

### Test Framework

- **Qt Test**: Native Qt testing framework
- **QSignalSpy**: Verify signal emissions
- **QTemporaryDir**: Isolated test environments
- **Skip capability**: Skip tests when dependencies unavailable (e.g., Pandoc)

## Test Suites

### 1. Core Components

#### `test_project_model.cpp` (12 tests)

Tests **ProjectModel** (project management, JSON serialization)

**Success scenarios** (✅):

- Create valid project
- Save and load project with all metadata
- Add/remove files with signal emissions
- Close project properly
- Save As to new location
- Track modification state

**Failure scenarios** (❌):

- Create project with invalid path
- Create project with empty name
- Load non-existent project file
- Load corrupted JSON file

**Edge cases** (⚠️):

- Add duplicate files (should ignore)
- Modification tracking across save/load

**Signals tested**:

- `projectOpened`, `projectClosed`
- `projectModified`, `projectSaved`
- `fileAdded`, `fileRemoved`
- `errorOccurred`

---

#### `test_conversion_engine.cpp` (9 tests)

Tests **ConversionEngine** (Markdown → LaTeX → PDF pipeline)

**Success scenarios** (✅):

- Initial state (Idle, not busy)
- Set Pandoc/XeLaTeX options
- Convert Markdown to LaTeX (requires Pandoc)
- Sequential conversions
- Busy state detection

**Failure scenarios** (❌):

- Convert non-existent file
- Convert with invalid output path
- Handle Pandoc errors

**Integration** (🔗):

- Actual Pandoc invocation (skipped if not installed)
- Verify generated LaTeX content

**Signals tested**:

- `conversionStarted`, `conversionProgress`
- `conversionCompleted`, `conversionFailed`
- `latexGenerated`, `pdfGenerated`

---

### 2. UI Components

#### `test_editor_widget.cpp` (11 tests)

Tests **EditorWidget** (Markdown text editor)

**Success scenarios** (✅):

- Load existing file
- Save to new file
- Save to current file
- Editor mode switching (Code/WYSIWYG)
- File modification signal

**Failure scenarios** (❌):

- Load non-existent file
- Save without current file set

**Edge cases** (⚠️):

- Empty file handling
- Large file handling (10MB+)
- UTF-8 encoding (French, Greek, emojis)

**Signals tested**:

- `fileModified`
- `modeChanged`

---

### 3. Example Tests

#### `test_example.cpp` (2 tests)

Basic Qt Test framework demonstration.

---

## Running Tests

### All Tests

```bash
cd app/build
cmake --build . --target test
# or
ctest --output-on-failure
```

### Specific Test Suite

```bash
./bin/test_project_model
./bin/test_editor_widget
./bin/test_conversion_engine
```

### With Verbose Output

```bash
ctest --verbose
# or
./bin/test_project_model -v2
```

### In CI (Headless with xvfb)

```bash
xvfb-run -a ctest --output-on-failure
```

## Test Coverage

### Current Coverage (Phase 0)

| Component | Tests | Coverage |
|-----------|-------|----------|
| ProjectModel | 12 | ✅ High |
| ConversionEngine | 9 | ✅ High |
| EditorWidget | 11 | ✅ High |
| PreviewWidget | 0 | ❌ None |
| ProjectTreeWidget | 0 | ❌ None |
| MainWindow | 0 | ❌ None |

**Total**: 32 tests (3 suites)

### TODO: Phase 1 Tests

- [ ] `test_preview_widget.cpp` - PDF loading, zoom, navigation
- [ ] `test_project_tree_widget.cpp` - File tree, selection, double-click
- [ ] `test_main_window.cpp` - Integration tests (signals between components)
- [ ] `test_markdown_highlighter.cpp` - Syntax highlighting rules
- [ ] `test_new_project_dialog.cpp` - Dialog validation, template selection

### TODO: Phase 2 Tests

- [ ] `test_auto_conversion.cpp` - Debounce timer, auto-save, live preview
- [ ] `test_file_operations.cpp` - Open multiple files, tab management
- [ ] `test_template_system.cpp` - Template loading, metadata parsing
- [ ] `test_bibliography.cpp` - BibTeX integration
- [ ] Performance tests (startup time, large projects)

## Writing New Tests

### Best Practices

1. **Isolation**: Use `QTemporaryDir` for file operations
2. **Cleanup**: Use `init()`/`cleanup()` for setup/teardown
3. **Signals**: Use `QSignalSpy` to verify signal emissions
4. **Skip when needed**: Use `QSKIP()` for missing dependencies
5. **Descriptive names**: `testLoadNonExistentFile` > `testLoad2`
6. **Verify failures**: Test error paths, not just success paths

### Test Template

```cpp
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include "../src/path/to/Component.h"

class TestComponent : public QObject
{
    Q_OBJECT

private slots:
    void init() {
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
        m_component = new Component();
    }

    void cleanup() {
        delete m_component;
        delete m_tempDir;
    }

    void testSuccessScenario() {
        QSignalSpy spy(m_component, &Component::someSignal);
        
        bool result = m_component->doSomething();
        
        QVERIFY(result);
        QCOMPARE(spy.count(), 1);
    }

    void testFailureScenario() {
        bool result = m_component->doInvalidOperation();
        
        QVERIFY(!result);
    }

private:
    QTemporaryDir* m_tempDir;
    Component* m_component;
};

QTEST_MAIN(TestComponent)
#include "test_component.moc"
```

### Adding to CMakeLists.txt

```cmake
add_executable(test_component tests/test_component.cpp)
target_link_libraries(test_component PRIVATE texloom_core Qt6::Test)
add_test(NAME test_component COMMAND test_component)
```

## CI Integration

Tests run automatically on GitHub Actions:

- **Trigger**: Every push, pull request
- **Environment**: Ubuntu 24.04, xvfb for headless Qt
- **Skip optimization**: Tests skipped if only docs changed
- **Artifacts**: Test reports on failure

See [.github/workflows/ci.yml](../../.github/workflows/ci.yml) for configuration.

## Debugging Failed Tests

### Local Debugging

```bash
# Run single test with verbose output
./bin/test_project_model testLoadCorruptedProject -v2

# Run with debugger
gdb --args ./bin/test_project_model
```

### CI Debugging

Check GitHub Actions logs:

1. Go to Actions tab
2. Click failed workflow
3. Expand "Run tests" step
4. Look for `FAIL!` messages with assertions

### Memory Leak Detection (Valgrind)

**Integrated with CTest** - Run memory checks locally:

```bash
cd app/build

# Run all tests with valgrind (slow but thorough)
ctest -T memcheck

# View results
cat Testing/Temporary/MemoryChecker.*.log
```

**Manual valgrind** for specific tests:

```bash
# Single test with full leak check
valgrind --leak-check=full --show-leak-kinds=definite,possible \
    ./bin/test_project_model

# All tests with valgrind
for test in ./bin/test_*; do
    echo "Running $test with valgrind..."
    valgrind --leak-check=full --show-leak-kinds=definite \
        --error-exitcode=1 "$test"
done
```

**Expected results**:

- `definitely lost: 0 bytes` ✅
- `indirectly lost: 0 bytes` ✅
- `possibly lost`: Small amounts in glib/Qt are acceptable
- `still reachable`: Qt caches (normal, not a leak)

**Note**: Qt/GTK plugins may show "Invalid read" warnings - these are false positives from theme engines, not our code. Valgrind is NOT run in CI (too slow), only regular tests are automated.

## Success Criteria

✅ **All tests pass** on Linux (Ubuntu/Debian)  
✅ **No Qt widgets leak** (valgrind clean)  
✅ **Tests complete in < 30 seconds** (without Pandoc integration tests)  
✅ **Code coverage > 80%** for core components (future: lcov integration)

## References

- [Qt Test Documentation](https://doc.qt.io/qt-6/qtest-overview.html)
- [QSignalSpy](https://doc.qt.io/qt-6/qsignalspy.html)
- [Core C++ Guidelines - Testing](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-testing)
