# Athena Package Manager

Athena Package Manager is an experimental package manager for Linux.

The project aims to combine:

- source-based package building inspired by Gentoo/Portage;
- dependency resolution;
- declarative system configuration inspired by Nix;
- immutable package storage;
- system generations;
- configuration rollback;
- integration with the Arch Linux ecosystem.

The initial implementation is written in C++20.

## Status

Early development — version 0.1.0.

At this stage the project contains the initial CMake-based C++ application.

## Build

Requirements:

- C++20 compiler
- CMake 3.20 or newer

Build:

```bash
cmake -S . -B build
cmake --build build

```

Run:

```bash
./build/athena
```

## Project goals

Athena aims to provide a package management system that combines the flexibility of source-based package management with the reproducibility and declarative model of functional package managers.

The project is initially developed and tested on Arch Linux.

## License

License to be decided.
