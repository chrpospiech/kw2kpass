#
# Copyright 2026 Christoph Pospiech
#
"""Helpers for opening and closing KeePass KDBX databases."""

from keepass import Database, KdbxFile, Key


def _build_key(Kpasswd: str | None) -> Key:
    """Create a KeePass key from an optional password."""
    key = Key()
    if Kpasswd is not None:
        key.SetPassword(Kpasswd)
    return key


def open_database(Kinfile: str | None, Kpasswd: str | None) -> Database:
    """Open an existing KDBX database, or return a new empty database."""
    if Kinfile is None:
        return Database()

    return KdbxFile().Import(Kinfile, _build_key(Kpasswd))


def close_database(database: Database, Koutfile: str, Kpasswd: str | None) -> None:
    """Export a database to KDBX format."""
    KdbxFile().Export(Koutfile, database, _build_key(Kpasswd))
