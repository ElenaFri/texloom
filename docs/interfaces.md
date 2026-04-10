# Component Interfaces & Communication Flow

## Architecture Overview

```text
ProjectTreeWidget  ←→  MainWindow  ←→  EditorWidget
                          ↕
                   ConversionEngine
                          ↕
                    PreviewWidget
```

**MainWindow** acts as the orchestrator - it connects all components via signals/slots.
Components do NOT communicate directly with each other.

## Component Responsibilities

### EditorWidget (UI)

**Purpose**: Markdown text editing

- Displays and edits `.md` files
- Tracks modification state
- Supports Code/WYSIWYG modes (WYSIWYG future)

**Public Interface**:

```cpp
// File operations
bool loadFile(const QString& filePath);
bool saveFile(const QString& filePath);
bool saveFile(); // Save to current file

// State queries
QString currentFile() const noexcept;
bool isModified() const;
Mode editorMode() const noexcept;
void setEditorMode(Mode mode);
```

**Signals Emitted**:

```cpp
void fileModified(bool modified);  // When content changes
void modeChanged(Mode mode);       // When switching Code ↔ WYSIWYG
```

### ConversionEngine (Core)

**Purpose**: Markdown → LaTeX → PDF pipeline

- Runs Pandoc (Markdown → LaTeX)
- Runs XeLaTeX (LaTeX → PDF)
- Manages temporary files
- Async operations with QProcess

**Public Interface**:

```cpp
// Conversion operations
void convertToLatex(const QString& markdownFile, const QString& outputLatex);
void compileToPdf(const QString& latexFile, const QString& outputPdf);
void convertAll(const QString& markdownFile, const QString& outputPdf);

// Configuration
void setPandocOptions(const QStringList& options);
void setXelatexOptions(const QStringList& options);
void setTemplatePath(const QString& path);

// State queries
Stage currentStage() const noexcept;
bool isBusy() const noexcept;
```

**Signals Emitted**:

```cpp
void conversionStarted(Stage stage);            // Starting conversion
void conversionProgress(const QString& message); // Progress updates
void conversionCompleted(const QString& outputFile); // Success
void conversionFailed(const QString& error);     // Error

void latexGenerated(const QString& latexFile);  // .tex ready
void pdfGenerated(const QString& pdfFile);      // .pdf ready
```

### PreviewWidget (UI)

**Purpose**: Display compiled PDF

- Shows PDF output
- TODO: Integrate QPdfView for actual rendering

**Public Interface**:

```cpp
// Preview operations
bool loadPdf(const QString& pdfPath);
void clear();

// State queries
QString currentPdf() const noexcept;
```

**Signals Emitted**:

```cpp
void pdfLoaded(const QString& path);       // PDF loaded successfully
void pdfLoadFailed(const QString& error);  // Load error
```

## Communication Flows

### Flow 1: User Edits File → Auto-Convert → Live Preview

```text
User types in EditorWidget
    ↓
EditorWidget::fileModified(true) signal
    ↓
MainWindow::onEditorModified() slot
    ↓
[Debounce timer: 1-2 seconds]
    ↓
EditorWidget::saveFile() to temp location
    ↓
ConversionEngine::convertAll(tempMd, tempPdf)
    ↓
ConversionEngine::conversionStarted(Stage::ConvertingToLatex)
    ↓ [Pandoc running]
ConversionEngine::latexGenerated(latexFile)
    ↓
ConversionEngine::conversionStarted(Stage::CompilingToPdf)
    ↓ [XeLaTeX running]
ConversionEngine::pdfGenerated(pdfFile)
    ↓
ConversionEngine::conversionCompleted(pdfFile)
    ↓
MainWindow::onConversionCompleted(pdfFile) slot
    ↓
PreviewWidget::loadPdf(pdfFile)
    ↓
PreviewWidget::pdfLoaded(pdfFile)
    ↓
MainWindow updates status bar: "Preview updated"
```

**Implementation Notes**:

- Use `QTimer::singleShot()` for debouncing (avoid converting on every keystroke)
- Save to temporary file to avoid overwriting user's source
- Show progress in status bar: "Converting..." → "Compiling..." → "Ready"

### Flow 2: User Clicks "Build → Convert to LaTeX"

```text
User clicks menu action
    ↓
MainWindow::onConvertToLatex() slot
    ↓
Get current EditorWidget from m_editorTabs
    ↓
QString mdFile = editor->currentFile()
    ↓
QString outputLatex = mdFile.replace(".md", ".tex")
    ↓
ConversionEngine::convertToLatex(mdFile, outputLatex)
    ↓
ConversionEngine::latexGenerated(outputLatex)
    ↓
MainWindow::onLatexGenerated(outputLatex) slot
    ↓
StatusBar: "LaTeX generated: path/to/file.tex"
```

### Flow 3: User Clicks "Build → Compile to PDF"

```text
User clicks menu action
    ↓
MainWindow::onCompileToPdf() slot
    ↓
Get current EditorWidget
    ↓
QString mdFile = editor->currentFile()
    ↓
QString outputPdf = mdFile.replace(".md", ".pdf")
    ↓
ConversionEngine::convertAll(mdFile, outputPdf)
    ↓
[Same flow as Flow 1]
    ↓
PreviewWidget::loadPdf(outputPdf)
    ↓
StatusBar: "PDF compiled successfully"
```

### Flow 4: User Opens File from Project Tree

```text
User double-clicks file in ProjectTreeWidget
    ↓
ProjectTreeWidget::fileDoubleClicked(filePath) signal
    ↓
MainWindow::onFileDoubleClicked(filePath) slot
    ↓
Check if already open in m_editorTabs
    ↓ [If not open]
Create new EditorWidget
    ↓
editor->loadFile(filePath)
    ↓
m_editorTabs->addTab(editor, fileName)
    ↓
Connect editor->fileModified to auto-convert flow
    ↓ [If already open]
m_editorTabs->setCurrentWidget(existingEditor)
```

## Signal/Slot Connections in MainWindow

### ProjectModel Connections

```cpp
connect(m_projectModel, &ProjectModel::projectOpened,
        this, &MainWindow::onProjectOpened);
connect(m_projectModel, &ProjectModel::projectModified,
        this, &MainWindow::onProjectModified);
connect(m_projectModel, &ProjectModel::fileAdded,
        this, &MainWindow::onFileAdded);
```

### ConversionEngine Connections

```cpp
connect(m_conversionEngine, &ConversionEngine::conversionStarted,
        this, &MainWindow::onConversionStarted);
connect(m_conversionEngine, &ConversionEngine::conversionProgress,
        this, &MainWindow::onConversionProgress);
connect(m_conversionEngine, &ConversionEngine::conversionCompleted,
        this, &MainWindow::onConversionCompleted);
connect(m_conversionEngine, &ConversionEngine::conversionFailed,
        this, &MainWindow::onConversionFailed);
connect(m_conversionEngine, &ConversionEngine::pdfGenerated,
        this, &MainWindow::onPdfGenerated);
```

### EditorWidget Connections (per tab)

```cpp
// For each EditorWidget added to m_editorTabs:
connect(editor, &EditorWidget::fileModified,
        this, &MainWindow::onEditorModified);
connect(editor, &EditorWidget::modeChanged,
        this, &MainWindow::onEditorModeChanged);
```

### PreviewWidget Connections

```cpp
connect(m_previewWidget, &PreviewWidget::pdfLoaded,
        this, &MainWindow::onPdfLoaded);
connect(m_previewWidget, &PreviewWidget::pdfLoadFailed,
        this, &MainWindow::onPdfLoadFailed);
```

### ProjectTreeWidget Connections

```cpp
connect(m_projectTree, &ProjectTreeWidget::fileSelected,
        this, &MainWindow::onFileSelected);
connect(m_projectTree, &ProjectTreeWidget::fileDoubleClicked,
        this, &MainWindow::onFileDoubleClicked);
```

## Auto-Conversion Strategy

### Option 1: Aggressive (Recommended for Development)

- Convert on every save (Ctrl+S)
- Debounce: 500ms after last keystroke
- Always show latest preview

**Pros**: Immediate feedback, true live preview
**Cons**: Higher CPU usage, more Pandoc/XeLaTeX invocations

### Option 2: Conservative

- Convert only on manual trigger (F5 or Build menu)
- User controls when to preview

**Pros**: Lower resource usage
**Cons**: Extra step for user

### Recommended Implementation

Start with **Option 1** (aggressive) with:

- 1-2 second debounce
- Option to disable auto-convert in settings (future)
- Visual indicator when conversion is running (spinner in status bar)

## Error Handling

### Conversion Errors

```cpp
void MainWindow::onConversionFailed(const QString& error) {
    // Show in status bar
    statusBar()->showMessage("Conversion failed: " + error, 5000);
    
    // Show message box for critical errors
    if (error.contains("Pandoc not found")) {
        QMessageBox::critical(this, "Conversion Error", 
                              "Pandoc not found. Please install Pandoc.");
    }
    
    // Keep showing old preview (don't clear)
}
```

### File Load Errors

```cpp
void MainWindow::onPdfLoadFailed(const QString& error) {
    statusBar()->showMessage("Preview error: " + error, 5000);
    // Show placeholder in PreviewWidget
    m_previewWidget->clear();
}
```

## Performance Considerations

### Debouncing

```cpp
// In MainWindow
QTimer* m_autoConvertTimer = nullptr;

void MainWindow::onEditorModified(bool modified) {
    if (!m_autoConvertEnabled) return;
    
    // Cancel previous timer
    if (m_autoConvertTimer) {
        m_autoConvertTimer->stop();
    }
    
    // Start new timer: convert after 1.5s of inactivity
    m_autoConvertTimer = new QTimer(this);
    m_autoConvertTimer->setSingleShot(true);
    connect(m_autoConvertTimer, &QTimer::timeout,
            this, &MainWindow::onAutoConvert);
    m_autoConvertTimer->start(1500); // 1.5 seconds
}
```

### Temporary Files

```cpp
// In ConversionEngine
QTemporaryDir m_tempDir; // Automatically cleaned up on destruction

void ConversionEngine::convertAll(const QString& mdFile, ...) {
    QString tempLatex = m_tempDir.filePath("output.tex");
    QString tempPdf = m_tempDir.filePath("output.pdf");
    // Use temp files, copy to final location on success
}
```

## Future Enhancements

### Multi-File Projects

- Editor tracks multiple open files
- Convert only the active file
- Or batch-convert all project files (Build → Build All)

### Incremental Compilation

- Cache intermediate LaTeX if Markdown unchanged
- Only re-run XeLaTeX if template/settings changed

### Real-Time Preview Sync

- Scroll preview to match editor cursor position
- Highlight corresponding PDF section (requires source maps)

## Implementation Checklist

### Phase 1: Basic Flow (Current)

- [x] Components exist with signals/slots
- [x] MainWindow connects ProjectModel signals
- [x] MainWindow connects ConversionEngine signals
- [ ] Connect EditorWidget signals (per tab)
- [ ] Connect PreviewWidget signals
- [ ] Connect ProjectTreeWidget signals
- [ ] Implement MainWindow slots for conversion flow

### Phase 2: File Opening

- [ ] Implement onFileDoubleClicked
- [ ] Track open editors in QMap<QString, EditorWidget*>
- [ ] Handle tab close (save prompt if modified)

### Phase 3: Auto-Convert

- [ ] Implement debounce timer
- [ ] Save to temp file on auto-convert
- [ ] Update PreviewWidget on conversion complete
- [ ] Show progress in status bar

### Phase 4: Error Handling

- [ ] Show conversion errors to user
- [ ] Handle missing Pandoc/XeLaTeX gracefully
- [ ] Validate LaTeX syntax before compiling (optional)

### Phase 5: Polish

- [ ] Add loading spinner during conversion
- [ ] Add "Stop Conversion" button (kill QProcess)
- [ ] Add conversion settings dialog (Pandoc options)
- [ ] Add QPdfView integration for real preview
