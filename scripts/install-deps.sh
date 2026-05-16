#!/bin/bash

# TexLoom Dependencies Installation Script for Linux
# This script installs all required dependencies to build and run TexLoom

set -e  # Exit on error

echo "==================================="
echo "TexLoom Dependencies Installer"
echo "==================================="
echo ""

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
else
    echo "Error: Cannot detect Linux distribution"
    exit 1
fi

echo "Detected distribution: $DISTRO"
echo ""

# Install dependencies based on distribution
case $DISTRO in
    ubuntu|debian|linuxmint)
        echo "Installing dependencies for Debian/Ubuntu..."
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            git \
            qt6-base-dev \
            qt6-base-dev-tools \
            qt6-pdf-dev \
            qgnomeplatform-qt6 \
            pandoc \
            texlive-xetex \
            texlive-latex-base \
            texlive-latex-recommended \
            texlive-fonts-recommended \
            lmodern \
            xvfb \
            valgrind \
            lcov
        ;;
    
    fedora|rhel|centos)
        echo "Installing dependencies for Fedora/RHEL..."
        sudo dnf install -y \
            cmake \
            gcc-c++ \
            git \
            qt6-qtbase-devel \
            qt6-qtpdf-devel \
            qgnomeplatform-qt6 \
            pandoc \
            texlive-xetex \
            texlive-collection-fontsrecommended \
            texlive-lm \
            xorg-x11-server-Xvfb \
            valgrind \
            lcov
        ;;
    
    arch|manjaro)
        echo "Installing dependencies for Arch Linux..."
        sudo pacman -S --needed --noconfirm \
            base-devel \
            cmake \
            git \
            qt6-base \
            qt6-pdf \
            qgnomeplatform-qt6 \
            pandoc \
            texlive-xetex \
            texlive-fontsrecommended \
            xorg-server-xvfb \
            valgrind \
            lcov
        ;;
    
    *)
        echo "Error: Unsupported distribution: $DISTRO"
        echo "Please install dependencies manually."
        echo "See docs/dependencies.md for the list of required packages."
        exit 1
        ;;
esac

echo ""
echo "==================================="
echo "Verifying installation..."
echo "==================================="
echo ""

# Verify installations
check_command() {
    if command -v $1 &> /dev/null; then
        version=$($1 --version 2>&1 | head -n 1)
        echo "✓ $1: $version"
    else
        echo "✗ $1: NOT FOUND"
        return 1
    fi
}

ALL_GOOD=true

check_command cmake || ALL_GOOD=false
check_command qmake6 || ALL_GOOD=false
check_command pandoc || ALL_GOOD=false
check_command xelatex || ALL_GOOD=false

echo ""
if [ "$ALL_GOOD" = true ]; then
    echo "==================================="
    echo "✓ All dependencies installed successfully!"
    echo "==================================="
    echo ""
    echo "You can now build TexLoom:"
    echo "  cmake -B app/build -S app"
    echo "  cmake --build app/build -j\$(nproc)"
else
    echo "==================================="
    echo "✗ Some dependencies are missing."
    echo "==================================="
    echo ""
    echo "Please check the errors above and install missing packages manually."
    echo "See docs/dependencies.md for more information."
    exit 1
fi
