# TexLoom LaTeX Templates

This directory contains Pandoc-compatible LaTeX templates for TexLoom.

## Available Templates

### 1. Article (`article.latex`)

**Purpose**: Short scientific articles, conference papers, journal submissions

**Features**:

- Single-column layout
- Simple structure (title, abstract, body, bibliography)
- Suitable for 5–20 page documents
- Built-in code highlighting
- Hyperlinks support

**Recommended for**: Academic papers, technical reports, short documentation

---

### 2. Report (`report.latex`)

**Purpose**: Technical reports, project documentation, medium-length documents

**Features**:

- Chapter-based structure
- Headers and footers (fancy style)
- Table of contents, list of figures, list of tables
- Code listings with line numbers
- Larger margins for binding

**Recommended for**: University reports, technical documentation, project deliverables

---

### 3. Thesis (`thesis.latex`)

**Purpose**: Master's thesis, PhD dissertation, books

**Features**:

- Book-class layout with frontmatter/mainmatter/backmatter
- Double-sided printing support (can be changed to oneside)
- 1.5 line spacing for readability
- Custom chapter styling
- Larger left margin for binding
- Abstract, acknowledgments, table of contents

**Recommended for**: Master's/PhD theses, books, long-form academic writing

---

## Usage with Pandoc

All templates support Pandoc's metadata variables:

```yaml
---
title: "Your Document Title"
author: 
  - "Author Name"
  - "Co-Author Name"
date: "April 2026"
lang: english  # or french
abstract: "Your abstract text here"
toc: true  # Include table of contents
lof: true  # Include list of figures (report/thesis)
lot: true  # Include list of tables (report/thesis)
bibliography: references.bib  # BibTeX file
---
```

## XeLaTeX Compatibility

All templates use:

- `fontspec` for Unicode font support
- `polyglossia` for multilingual documents (English/French)
- `unicode-math` for mathematical symbols
- Latin Modern fonts (included in TeX Live)

## Customization

Templates can be customized by users:

1. Copy template to project directory
2. Modify LaTeX preamble (packages, settings)
3. Specify custom template in TexLoom project settings

## Testing

Test templates with:

```bash
pandoc input.md -o output.pdf --template=article.latex --pdf-engine=xelatex
```

## Future Enhancements

- [ ] IEEE conference template
- [ ] APA style article template
- [ ] Beamer presentation template
- [ ] Letter/CV template
