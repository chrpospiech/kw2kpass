#
# Copyright 2026 Christoph Pospiech
#
"""Helpers for opening and closing KeePass KDBX databases."""

import logging

from pykeepass import PyKeePass, create_database

logger = logging.getLogger("kw2kpass")


def open_database(Kinfile: str | None, Koutfile: str, Kpasswd: str | None) -> PyKeePass:
    """Open an existing KDBX database, or return a new empty database."""
    if Kinfile is None:
        logger.info("Creating new empty KeePass database at %s", Koutfile)
        # pykeepass requires a path when creating a database.
        return create_database(Koutfile, password=(Kpasswd or ""))

    logger.info("Opening KeePass database from %s", Kinfile)
    return PyKeePass(Kinfile, password=Kpasswd)


def close_database(database: PyKeePass, Koutfile: str, Kpasswd: str | None) -> None:
    """Export a database to KDBX format."""
    if Kpasswd is not None:
        database.password = Kpasswd
    logger.info("Exporting KeePass database to %s", Koutfile)
    database.save(Koutfile)
