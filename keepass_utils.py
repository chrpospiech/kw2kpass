#
# Copyright 2026 Christoph Pospiech
#
"""Helpers for opening and closing KeePass KDBX databases."""

import logging

from keepass import Database, KdbxFile, Key

logger = logging.getLogger("kw2pwsafe")


def _build_key(Kpasswd: str | None) -> Key:
    """Create a KeePass key from an optional password."""
    logger.debug("Creating KeePass key (password provided: %s)", Kpasswd is not None)
    key = Key()
    if Kpasswd is not None:
        key.SetPassword(Kpasswd)
    return key


def open_database(Kinfile: str | None, Kpasswd: str | None) -> Database:
    """Open an existing KDBX database, or return a new empty database."""
    if Kinfile is None:
        logger.info("Creating new empty KeePass database")
        return Database()

    logger.info("Opening KeePass database from %s", Kinfile)
    return KdbxFile().Import(Kinfile, _build_key(Kpasswd))


def close_database(database: Database, Koutfile: str, Kpasswd: str | None) -> None:
    """Export a database to KDBX format."""
    logger.info("Exporting KeePass database to %s", Koutfile)
    KdbxFile().Export(Koutfile, database, _build_key(Kpasswd))
