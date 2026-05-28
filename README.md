# kw2kpass

## Introduction

## Features

## Installation

### Prerequisites

A couple of prerequisites need to be installed for
this tool.

- Python, Qt and pybind
  This now handled via pyproject.toml and uv[.lock].

   - rm -rf ~/.venv
   - uv sync

- KDE 5 Framework (kf5)

   - sudo apt-get install libkf5coreaddons-dev libkf5coreaddons-doc
   - sudo apt-get install libkf5solid-dev libkf5solid-doc
   - sudo apt-get install libkf5wallet-dev

- OpenSSL Crypto development headers

   - sudo apt-get install libssl-dev

### Building the libraries

Initialize the `libkeepass` submodule before configuring the build:

```sh
git submodule update --init
```

Configure and build the project with CMake:

```sh
cmake -S . -B build
cmake --build build
```

## Usage

Options:

- tbd
- `-D, --dry-run`: Do not execute DB writes.
- `-v, --verbose`: Verbose output.

## License

See LICENSE for details.

## Hints for Developers

The following needs to be reworked!

The repo contains linter configuration files `rustfmt.toml`,
`.codespell.dictionary`, and `.pre-commit-config.yaml`.
The developer is strongly encouraged to use `pre-commit`
with these configuration files to maintain code quality.

To enable `pre-commit` using `uv`:

- `uv init`
- `rm .python-version main.py`
- `uv add pre-commit`
- `source .venv/bin/activate`
- `pre-commit run --all`
- `pre-commit install`
