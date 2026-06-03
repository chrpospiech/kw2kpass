#
# Copyright 2026 Christoph Pospiech
#
"""Helpers for opening and closing KeePass KDBX databases."""

from keepass import Database, KdbxFile, Key


def open_database(Kinfile: str | None) -> Database:
    """Open an existing KDBX database, or return a new empty database."""
    if Kinfile is None:
        return Database()

    return KdbxFile().Import(Kinfile, Key())


def close_database(database: Database, Koutfile: str) -> None:
    """Export a database to KDBX format."""
    KdbxFile().Export(Koutfile, database, Key())
