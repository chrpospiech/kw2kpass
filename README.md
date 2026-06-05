# kw2kpass

kw2kpass reads entries from KDE KWallet folders and writes them into a KeePass
KDBX database.

## Features

- Read credentials from a selected KWallet (default: `kdewallet`).
- Map one or more KWallet folders to KeePass groups.
- Filter entries with a regex against `username + hostname`.
- Create a new KeePass database or update an existing one.
- Build includes a local `kwallet` Python extension (pybind11 + Qt5 + KF5).

## Requirements

### Python dependencies

- Python 3.12+
- `pykeepass`
- `pybind11`
- `PyYAML`
- `rich`

Install with:

```sh
uv sync
```

### System dependencies (Linux/KDE)

Build requires CMake, Qt5 and KDE Framework 5 development packages.

Example on Debian/Ubuntu:

```sh
sudo apt-get install \
   cmake \
   qtbase5-dev \
   libkf5coreaddons-dev \
   libkf5solid-dev \
   libkf5wallet-dev
```

## Build

Configure and compile as follows.

```sh
cmake -S . -B build
cmake --build build
```

After build, the `kwallet` extension module is copied to the project root so
the Python code can import it.

## Usage

Run either

```sh
source <project_root>/.venv/bin/activate
python kw2kpass.py [OPTIONS]
```

or

```sh
uv run kw2kpass.py [OPTIONS]
```

### Options

- `-w, --wallet NAME`
   KWallet name (default: `kdewallet`).
- `-m, --map FOLDER[:GROUP]`
   Map a KWallet folder to a KeePass group. Can be repeated.
   If `GROUP` is omitted, folder name is used.
- `-F, --filter REGEX`
   Only process entries where `username + hostname` matches regex
   (default: `.*`).
- `-i, --infile FILE`
   Existing input KDBX file. If omitted, a new database is created.
- `-o, --outfile FILE`
   Output KDBX path (default: `keepass.kdbx`).
- `-p, --passwd PASSWD`
   KeePass password.
- `-V, --verbose`
   Enable INFO logging.
- `-D, --debug`
   Enable DEBUG logging.

### Examples

Create a new database from one folder:

```sh
python kw2kpass.py \
   -m Chromium\ Form\ Data:Browser \
   -o keepass.kdbx \
   -p 'change-me'
```

Import multiple folders into an existing database and filter hosts:

```sh
python kw2kpass.py \
   -i existing.kdbx \
   -o updated.kdbx \
   -p 'change-me' \
   -m Chromium\ Form\ Data:Browser \
   -m Network\ Management:WiFi \
   -F 'github|gitlab|example\\.com'
```

## Notes

- If a wallet folder does not exist, execution exits with status 2.
- Entries with password `n/a` are skipped.
- Existing KeePass entries are matched by `(group, title)` and updated in place.

## Developer notes

Run linting/format checks with Ruff as configured in `pyproject.toml`.

```sh
uv run ruff check .
uv run ruff check --fix .
uv run ruff format .
```

Alternatively, `pre-commit` can be installed like this.

```sh
uv run pre-commit install
```
