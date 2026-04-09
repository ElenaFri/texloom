# TexLoom — Roadmap

## Core differentiators to implement

- [ ] Multi-file project support (book, thesis, report)
- [ ] Real-time PDF preview
- [ ] Integrated LaTeX templates (easy to configure)
- [ ] Simple bibliography management (BibTeX)
- [ ] Clean export for scientific articles and reports
- [ ] Fast, stable compilation pipeline
- [ ] Multi-language support (English and French, to begin with)

## Architecture

- [ ] GUI: Qt (C++)
- [ ] Editor: text component with Markdown syntax highlighting
- [ ] Conversion: Pandoc or custom Markdown => LaTeX => PDF pipeline
- [ ] Preview: embedded PDF viewer
- [ ] Project manager: `.md` file list/tree

## Open questions

- [ ] Pandoc dependency vs. custom Markdown parser?
- [ ] Which LaTeX engine? (pdflatex / xelatex / lualatex)
- [ ] Bundled templates: which formats to prioritize? (IEEE, APA, thesis…)
- [ ] Cross-platform scope: Linux only or Windows/macOS too?
