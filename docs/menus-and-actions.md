# Menus and Actions

This document defines the main menu structure and actions for TexLoom.

## Menu: File

| Action | Shortcut | Description |
|--------|----------|-------------|
| New Project... | Ctrl+N | Create a new TexLoom project |
| Open Project... | Ctrl+O | Open an existing .texloom project file |
| Save Project | Ctrl+S | Save current project |
| Save Project As... | Ctrl+Shift+S | Save project with a new name |
| Close Project | Ctrl+W | Close current project |
| --- | --- | --- |
| New Markdown File | Ctrl+Alt+N | Add a new markdown file to project |
| Add Existing File... | | Add existing markdown file to project |
| Remove File | | Remove selected file from project |
| --- | --- | --- |
| Recent Projects | | Submenu with recently opened projects |
| --- | --- | --- |
| Export Settings... | | Export project settings to share |
| Import Settings... | | Import project settings |
| --- | --- | --- |
| Quit | Ctrl+Q | Exit application |

## Menu: Edit

| Action | Shortcut | Description |
|--------|----------|-------------|
| Undo | Ctrl+Z | Undo last edit |
| Redo | Ctrl+Shift+Z | Redo last undone edit |
| --- | --- | --- |
| Cut | Ctrl+X | Cut selected text |
| Copy | Ctrl+C | Copy selected text |
| Paste | Ctrl+V | Paste from clipboard |
| Select All | Ctrl+A | Select all text in editor |
| --- | --- | --- |
| Find... | Ctrl+F | Find text in current file |
| Replace... | Ctrl+H | Find and replace text |
| Find in Project... | Ctrl+Shift+F | Search across all project files |
| --- | --- | --- |
| Preferences... | Ctrl+, | Open application settings |

## Menu: View

| Action | Shortcut | Description |
|--------|----------|-------------|
| Project Tree | Ctrl+1 | Toggle project tree panel |
| Preview Panel | Ctrl+2 | Toggle preview panel |
| Log Output | Ctrl+3 | Toggle compilation log panel |
| --- | --- | --- |
| Editor Mode: Code | Ctrl+Shift+M | Switch to code (raw Markdown) editor |
| Editor Mode: WYSIWYG | Ctrl+Shift+W | Switch to WYSIWYG Markdown editor |
| --- | --- | --- |
| Split Editor | Ctrl+\ | Split editor horizontally |
| Close Split | | Close current split |
| --- | --- | --- |
| Zoom In | Ctrl++ | Increase editor font size |
| Zoom Out | Ctrl+- | Decrease editor font size |
| Reset Zoom | Ctrl+0 | Reset editor font size |
| --- | --- | --- |
| Fullscreen | F11 | Toggle fullscreen mode |

## Menu: Build

| Action | Shortcut | Description |
|--------|----------|-------------|
| Convert to LaTeX | Ctrl+B | Generate LaTeX from current file |
| Convert All Files | Ctrl+Shift+B | Generate LaTeX from all project files |
| --- | --- | --- |
| Compile PDF | F5 | Compile LaTeX to PDF (XeLaTeX) |
| Compile and Preview | Ctrl+F5 | Compile and show PDF preview |
| --- | --- | --- |
| Clean Build Files | | Remove intermediate build files |
| --- | --- | --- |
| Build Settings... | | Configure build options |

## Menu: Tools

| Action | Shortcut | Description |
|--------|----------|-------------|
| Validate Markdown | | Check markdown syntax |
| Format Markdown | Ctrl+Shift+I | Auto-format markdown file |
| --- | --- | --- |
| Insert Template... | Ctrl+T | Insert LaTeX template snippet |
| Manage Templates... | | Open template manager |
| --- | --- | --- |
| Bibliography Manager... | | Manage BibTeX references |
| --- | --- | --- |
| Word Count | | Show document statistics |

## Menu: Help

| Action | Shortcut | Description |
|--------|----------|-------------|
| Documentation | F1 | Open user documentation |
| Markdown Syntax Reference | | Quick markdown syntax guide |
| LaTeX Commands Reference | | Common LaTeX commands |
| --- | --- | --- |
| Check for Updates... | | Check for application updates |
| Report Issue... | | Open GitHub issues page |
| --- | --- | --- |
| About TexLoom | | Show about dialog |

## Main Toolbar

Primary actions that should appear in the main toolbar:

| Icon | Action | Tooltip |
|------|--------|---------|
| 📄 | New Project | Create new project (Ctrl+N) |
| 📂 | Open Project | Open project (Ctrl+O) |
| 💾 | Save | Save project (Ctrl+S) |
| ➕ | New File | Add markdown file (Ctrl+Alt+N) |
| --- | --- | --- |
| ↩️ | Undo | Undo (Ctrl+Z) |
| ↪️ | Redo | Redo (Ctrl+Shift+Z) |
| --- | --- | --- |
| 🔄 | Convert to LaTeX | Convert current file (Ctrl+B) |
| ▶️ | Compile PDF | Compile to PDF (F5) |
| 👁️ | Preview | Compile and preview (Ctrl+F5) |
| --- | --- | --- |
| 🔍 | Find | Find in file (Ctrl+F) |
| ⚙️ | Settings | Application preferences |

## WYSIWYG Editor Toolbar

Formatting toolbar that appears when in WYSIWYG mode (Ctrl+Shift+W):

| Icon | Action | Tooltip | Shortcut |
|------|--------|---------|----------|
| **B** | Bold | Make text bold | Ctrl+B |
| *I* | Italic | Make text italic | Ctrl+I |
| ~~S~~ | Strikethrough | Strikethrough text | Ctrl+Shift+S |
| `<>` | Inline Code | Format as inline code | Ctrl+` |
| --- | --- | --- | --- |
| H1 | Heading 1 | Insert level 1 heading | Ctrl+1 |
| H2 | Heading 2 | Insert level 2 heading | Ctrl+2 |
| H3 | Heading 3 | Insert level 3 heading | Ctrl+3 |
| --- | --- | --- | --- |
| • | Unordered List | Insert bullet list | Ctrl+Shift+8 |
| 1. | Ordered List | Insert numbered list | Ctrl+Shift+7 |
| ☑ | Task List | Insert checkbox list | Ctrl+Shift+T |
| --- | --- | --- | --- |
| " | Blockquote | Insert blockquote | Ctrl+Shift+9 |
| {} | Code Block | Insert code block | Ctrl+Shift+K |
| 🔗 | Link | Insert hyperlink | Ctrl+K |
| 🖼️ | Image | Insert image | Ctrl+Shift+I |
| 📊 | Table | Insert table | Ctrl+Shift+T |
| --- | --- | --- | --- |
| ← | Decrease Indent | Outdent list item | Shift+Tab |
| → | Increase Indent | Indent list item | Tab |
| --- | --- | --- | --- |
| ⌨️ | Code Mode | Switch to code editor | Ctrl+Shift+M |

**Note**: WYSIWYG mode provides visual formatting while preserving raw Markdown. The editor shows formatted text with formatting controls appearing on selection/hover.

## Context Menus

### Project Tree Context Menu (right-click on file)

- Open in Editor
- Open in External Editor

- ---

- Rename File...
- Duplicate File...
- Remove from Project

- ---

- Show in File Manager
- Copy File Path

### Project Tree Context Menu (right-click on project root)

- New Markdown File...
- Add Existing File...

- ---

- Convert All Files
- Compile PDF

- ---

- Open Project Folder
- Project Settings...

### Editor Context Menu (right-click in editor)

- Cut
- Copy
- Paste

- ---

- Select All

- ---

- Insert Template...
- Format Markdown

- ---

- Convert to LaTeX
- Compile PDF

## Action Implementation Notes

### QAction Properties

Each action should be implemented with:

- **objectName**: Unique identifier (e.g., `actionNewProject`)
- **text**: Display text with mnemonic (e.g., `&New Project...`)
- **shortcut**: Keyboard shortcut (QKeySequence)
- **statusTip**: Brief description for status bar
- **icon**: QIcon for toolbar/menu (use Qt standard icons or custom)
- **enabled**: Initially enabled/disabled based on state

### Action Groups

Group mutually exclusive actions:

- **View modes**: Editor only / Split view / Preview only
- **Editor modes**: Code (raw Markdown) / WYSIWYG (visual Markdown)
- **Build targets**: Current file / All files / Selected files

### Dynamic Actions

Some actions should be dynamically enabled/disabled:

- Save: enabled only when project is modified
- Undo/Redo: enabled based on undo stack
- Close Project: enabled only when project is open
- Build actions: enabled only when project is open
- Remove File: enabled only when file is selected

### Signals and Slots

Actions trigger signals that components handle:

```cpp
// Example connections
connect(ui->actionNewProject, &QAction::triggered, 
        this, &MainWindow::onNewProject);
connect(ui->actionBuild, &QAction::triggered,
        m_conversionEngine, &ConversionEngine::convertToLatex);
```

## Future Enhancements

Actions to consider for later versions:

- Export to DOCX/HTML
- Git integration (commit, push, pull)
- Collaborative editing features
- Spell checker toggle
- Dark/Light theme switcher
- Custom keyboard shortcut editor
- Macro recording/playback
