# texloom

![CI](https://github.com/ElenaFri/texloom/workflows/CI/badge.svg)
[![codecov](https://codecov.io/gh/ElenaFri/texloom/graph/badge.svg?token=IWDA5T9OR7)](https://codecov.io/gh/ElenaFri/texloom)

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
cmake -B build
cmake --build build -j$(nproc)
```

## Run

Launch the application:

```bash
./app/build/bin/texloom
```

## Tests

Run the test suite:

```bash
ctest --test-dir app/build --output-on-failure
```
