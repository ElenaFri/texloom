# TexLoom — Dependencies

_This is the initial version, may be edited anytime soon._

This document lists all dependencies required to develop and build TexLoom from source.

## Build-time Dependencies

These are required to compile TexLoom from source.

### Required

- **Qt 6.5+** — GUI framework
  - Required components: Widgets, Core, Concurrent
  - LTS version recommended (6.5 or 6.6)
  
- **CMake 3.25+** — Build system
  - Modern CMake features required
  
- **C++20 compiler**
  - GCC 11+ (Linux)
  - Clang 14+ (Linux/macOS)
  - MSVC 2019+ (Windows)

### Optional (for development)

- **Qt Test** — Unit testing framework (included with Qt)
- **Git** — Version control
- **xvfb** — Virtual framebuffer for headless testing (Linux)

## Runtime Dependencies

These must be installed on the user's system to run TexLoom.

### Required

- **Pandoc 3.1+** — Markdown to LaTeX converter
  - Download: <https://pandoc.org/installing.html>
  
- **TeX Live 2024+** (or equivalent)
  - Must include `xelatex` engine
  - Alternative: MiKTeX (Windows), MacTeX (macOS)
  - Packages required: `xetex`, `fontspec`, `unicode-math`

## Installation Instructions

### Ubuntu/Debian

```bash
# Build dependencies
sudo apt update
sudo apt install build-essential cmake git
sudo apt install qt6-base-dev qt6-base-dev-tools

# Runtime dependencies
sudo apt install pandoc texlive-xetex texlive-fonts-recommended

# Optional: for headless testing
sudo apt install xvfb
```

### Fedora/RHEL

```bash
# Build dependencies
sudo dnf install cmake gcc-c++ git
sudo dnf install qt6-qtbase-devel

# Runtime dependencies
sudo dnf install pandoc texlive-xetex texlive-collection-fontsrecommended

# Optional: for headless testing
sudo dnf install xorg-x11-server-Xvfb
```

### Arch Linux

```bash
# Build dependencies
sudo pacman -S base-devel cmake git qt6-base

# Runtime dependencies
sudo pacman -S pandoc texlive-xetex texlive-fontsrecommended

# Optional: for headless testing
sudo pacman -S xorg-server-xvfb
```

### macOS (Homebrew)

```bash
# Build dependencies
brew install cmake qt@6

# Runtime dependencies
brew install pandoc
brew install --cask mactex  # or mactex-no-gui for smaller install
```

### Windows

1. **Qt 6.5+**: Download from <https://www.qt.io/download-qt-installer>
2. **CMake**: Download from <https://cmake.org/download/>
3. **Pandoc**: Download from <https://pandoc.org/installing.html>
4. **MiKTeX**: Download from <https://miktex.org/download>

## Verifying Installation

After installation, verify all dependencies are available:

```bash
# Check versions
cmake --version        # Should be 3.25+
qmake6 --version       # Should show Qt 6.5+
pandoc --version       # Should be 3.1+
xelatex --version      # Should show TeX Live 2024+

# Check if executables are in PATH
which pandoc
which xelatex
```

## Minimum Disk Space

- **Build**: ~500 MB
- **TeX Live full**: ~6-7 GB
- **TexLoom installed**: ~50 MB

For a minimal installation, TeX Live can be reduced to ~1 GB by installing only required packages.

## Troubleshooting

### Qt 6 not found

```bash
# Set Qt path manually
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
```

### Pandoc not in PATH

Add Pandoc installation directory to your PATH environment variable.

### xelatex missing fonts

```bash
# Install additional font packages
sudo apt install texlive-fonts-extra  # Ubuntu/Debian
sudo dnf install texlive-collection-fontsextra  # Fedora
```
