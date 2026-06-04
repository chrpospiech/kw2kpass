#
# Copyright 2026 Christoph Pospiech
#
"""Helpers for opening and closing KeePass KDBX databases."""

import logging
from typing import cast

from pykeepass import PyKeePass, create_database
from pykeepass.entry import Entry

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


def find_or_create_entry(database: PyKeePass, group: str, title: str) -> Entry:
    """Find an entry by group name and title, or create it if absent.

    Searches for a group whose name matches *group*, then looks for an entry
    in that group whose title matches *title*. If the entry is found it is
    returned unchanged. If the group is missing it is created under the root
    group. In either missing case, a new Entry with the given title is
    appended to the group and returned.

    @param database: Open KeePass database.
    @param group: Name of the target group (created under root if absent).
    @param title: Title of the entry to find or create.
    @return: The existing or newly created Entry.
    """
    target_group = database.find_groups(name=group, first=True)

    if target_group is None:
        logger.info(f"Group '{group}' not found, creating it")
        target_group = database.add_group(database.root_group, group)

    # Search for an existing entry with the given title.
    entry = database.find_entries(title=title, group=target_group, first=True)
    if entry is not None:
        logger.debug(f"Entry '{title}' found in group '{group}'")
        return cast(Entry, entry)

    # Create a new entry.
    logger.info(f"Entry '{title}' not found in group '{group}', creating it")
    return cast(
        Entry,
        database.add_entry(target_group, title, username="", password="", url=""),
    )
