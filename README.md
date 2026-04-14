# texloom

![CI](https://github.com/ElenaFri/texloom/workflows/CI/badge.svg)
[![codecov](https://codecov.io/gh/ElenaFri/texloom/graph/badge.svg)](https://codecov.io/gh/ElenaFri/texloom)

A simple Markdown-to-LaTeX converter and text editor. With (useful) GUI.

## Dependencies

Install all required dependencies (Linux only):

```bash
./scripts/install-deps.sh
```

Or see [docs/dependencies.md](docs/dependencies.md) for manual installation.

## Build

Compile the application with CMake:

```bash
cd app
mkdir build && cd build
cmake ..
cmake --build .
```

## Run

Launch the application:

```bash
# From app/build/
./bin/texloom
```

## Tests

Run the test suite:

```bash
# From app/build/
ctest
```
