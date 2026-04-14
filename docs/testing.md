# Test Plan

## Philosophy

- Tests cover **observable behavior**, not implementation details.
- UI widget tests use Qt's `QTest` / `QSignalSpy` to verify signals and state changes — no mocking of Qt internals.
- Integration tests that require external tools (Pandoc, XeLaTeX) use `QSKIP` when the tool is not available.
- `MainWindow` is intentionally not unit-tested: it is an orchestrator with heavy GUI interactions that are better validated manually or via future end-to-end tests.

---

## Test suites

### `test_project_model` — Core / data layer

| Test | What it verifies |
| --- | --- |
| `testCreateValidProject` | creates project file on disk, emits `projectOpened` + `projectSaved` |
| `testSaveAndLoadProject` | full round-trip: create → add files/template/bib → save → close → reload |
| `testAddFiles` | correct count, `fileAdded` signal, `projectModified` emitted once |
| `testRemoveFile` | correct count, `fileRemoved` signal, `projectModified` |
| `testCloseProject` | `isOpen()` → false, `projectClosed` signal |
| `testDuplicateFileNotAdded` | adding the same path twice → still 1 entry |
| `testCreateProjectInvalidPath` | returns false, no file created |
| `testLoadInvalidFile` | returns false on non-existent/corrupt `.texloom` |
| `testSetTemplateName` | stored and retrievable after save/load |
| `testSetBibliographyFile` | stored and retrievable after save/load |

### `test_conversion_engine` — Core / conversion pipeline

| Test | What it verifies |
| --- | --- |
| `testInitialState` | `Stage::Idle`, `isBusy() == false` |
| `testSetPandocOptions` | no crash, state unchanged |
| `testSetXelatexOptions` | no crash, state unchanged |
| `testSetTemplatePath` | no crash, state unchanged |
| `testConvertToLatexWithPandoc` *(integration)* | full Markdown → LaTeX run, signals fired, output file exists |
| `testConvertFailsOnMissingInput` | emits `conversionFailed`, not busy after |
| `testConvertToPdfWithXelatex` *(integration)* | LaTeX → PDF run, PDF file exists |

### `test_editor_widget` — UI / editor

| Test | What it verifies |
| --- | --- |
| `testLoadExistingFile` | content displayed, `currentFile()` set, not modified |
| `testSaveToNewFile` | file created on disk, `isModified()` false |
| `testSaveToCurrentFile` | existing file overwritten |
| `testEditorModeSwitch` | `modeChanged` signal, correct mode value |
| `testFileModificationSignal` | `modificationChanged` signal on text edit |
| `testSaveFailsWithNoFile` | returns false when no path and no current file |
| `testLoadNonExistentFile` | returns false, state unchanged |

### `test_preview_widget` — UI / PDF preview

| Test | What it verifies |
| --- | --- |
| `testInitialState` | `currentPdf()` empty |
| `testLoadExistingPdf` | returns true, `currentPdf()` set, `pdfLoaded` emitted |
| `testLoadNonExistentPdf` | returns false, `pdfLoadFailed` emitted with message |
| `testClearAfterLoad` | `currentPdf()` empty after `clear()` |
| `testClearOnEmptyWidget` | no crash |
| `testReloadWithDifferentFile` | `currentPdf()` updated, `pdfLoaded` emitted again |

### `test_project_tree_widget` — UI / file tree

| Test | What it verifies |
| --- | --- |
| `testInitialState` | no items |
| `testSetProjectRoot` | one top-level item with correct name |
| `testSetProjectRootReplacesExisting` | previous root removed |
| `testAddFileAppearsUnderRoot` | child item with display name (filename only) |
| `testAddMultipleFiles` | correct child count |
| `testAddFileStoresFullPath` | `UserRole` data = full path |
| `testAddFileWithoutProjectRootDoesNotCrash` | silently ignored |
| `testRemoveExistingFile` | child count decremented, correct file removed |
| `testRemoveNonExistentFileDoesNotCrash` | no side effect |
| `testRemoveWithoutProjectRootDoesNotCrash` | silently ignored |
| `testClearRemovesAllItems` | `topLevelItemCount() == 0` |
| `testClearThenAddWorksAgain` | tree usable after clear + new root |
| `testFileSelectedSignalOnClick` | `fileSelected` emitted with full path |
| `testFileDoubleClickedSignal` | `fileDoubleClicked` emitted with full path |
| `testRootItemClickDoesNotEmitFileSelected` | root has no path → no signal |

---

## Not unit-tested (by design)

| Component | Reason |
| --- | --- |
| `MainWindow` | Orchestrator with complex GUI; covered by manual testing and future E2E tests |
| PDF rendering | No PDF engine integrated yet (placeholder label only) |
| Actual LaTeX compilation | Requires full TeX distribution; covered by integration tests with `QSKIP` guard |
