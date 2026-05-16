# TexLoom — Roadmap

## Core features to implement

- [x] Multi-file project support (book, thesis, report)
- [ ] Real-time PDF preview
- [x] Integrated LaTeX templates (easy to configure) — article, report, thesis
- [ ] Simple bibliography management (BibTeX)
- [ ] Clean export for scientific articles and reports
- [x] Fast, stable compilation pipeline (Pandoc + XeLaTeX, async, fully tested)
- [ ] Multi-language support (English and French, to begin with)

## Architecture

- [x] GUI: Qt (C++)
- [ ] Editor: text component with Markdown syntax highlighting
  - [x] Code mode (raw Markdown) — `EditorWidget` implemented
  - [ ] WYSIWYG mode (visual editing)
- [x] Conversion: Pandoc => LaTeX => PDF pipeline (ConversionEngine implemented, auto-save + template wiring done)
- [x] Preview: embedded PDF viewer (QPdfView integration with zoom controls and page navigation)
- [x] Project manager: `.md` file list/tree (`ProjectModel` + `ProjectTreeWidget`)

## GUI

See [images/mockups/](images/mockups/) for editor mode designs

### Code Mode (Raw Markdown Editor)

![Code Mode - Raw Markdown editor with syntax highlighting](images/mockups/code-mode.png)

### WYSIWYG Mode (Visual Markdown Editor)

![WYSIWYG Mode - Visual editing with formatting toolbar](images/mockups/wysiwyg-mode.png)

**Documentation**: See [docs/menus-and-actions.md](docs/menus-and-actions.md) for complete menu/action specifications

## Open questions

- [x] Pandoc dependency vs. custom Markdown parser? → **Pandoc 3.1+**
- [x] Which LaTeX engine? (pdflatex / xelatex / lualatex) → **XeLaTeX** (Unicode/multilingual)
- [x] Bundled templates: which formats to prioritize? → **article, report, thesis** (Pandoc LaTeX format)
- [x] Cross-platform scope: Linux only or Windows/macOS too? → **Linux-first, cross-platform ready**
