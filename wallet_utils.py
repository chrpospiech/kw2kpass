#
# Copyright 2026 Christoph Pospiech
#
"""Utility functions for accessing KWallet."""

import logging

from keepass import Database, Entry, Group, ProtectedString
from kwallet import WalletHandle

logger = logging.getLogger("kw2kpass")


def open_wallet(Wname):
    """Open Kwallet "Wname".

    @summary: Opens Kwallet "Wname"
    @return: Handle to the open wallet.
    """
    wh = WalletHandle(Wname)
    if not wh.wallet_is_open():
        logger.error(f"Wallet {Wname} not found")
        raise SystemExit(2)
    else:
        logger.info(f"Wallet {Wname} opened")
    return wh


def set_wallet_folder(wallet, Wfolder):
    """Validate and set the active folder in an open wallet.

    @summary: Checks that Wfolder exists in wallet and sets it as active.
    @param wallet: Kwallet handle
    @param Wfolder: Kwallet folder name
    """
    if wallet.hasFolder(Wfolder):
        logger.info(f"folder {Wfolder} found")
    else:
        logger.error(f"No folder {Wfolder} found")
        raise SystemExit(2)
    if wallet.setFolder(Wfolder):
        logger.info(f"folder {Wfolder} set")
    else:
        logger.error(f"Folder {Wfolder} could not be set")
        raise SystemExit(2)


def find_or_create_entry(database: Database, group: str, title: str) -> Entry:
    """Find an entry by group name and title, or create it if absent.

    Searches the root group's direct subgroups for one whose name matches
    *group*, then looks for an entry whose title matches *title*.  If the
    entry is found it is returned unchanged.  If only the group is missing
    it is created under root.  In either missing case a new Entry with the
    given title is appended to the group and returned.

    @param database: Open KeePass database.
    @param group: Name of the target group (created under root if absent).
    @param title: Title of the entry to find or create.
    @return: The existing or newly created Entry.
    """
    root = database.root()
    if root is None:
        root = Group()
        database.set_root(root)

    # Locate group under root.
    target_group: Group | None = None
    for g in root.Groups():
        if g.name() == group:
            target_group = g
            break

    if target_group is None:
        logger.info(f"Group '{group}' not found, creating it")
        target_group = Group()
        target_group.set_name(group)
        root.AddGroup(target_group)

    # Search for an existing entry with the given title.
    for entry in target_group.Entries():
        if entry.title().value() == title:
            logger.debug(f"Entry '{title}' found in group '{group}'")
            return entry

    # Create a new entry.
    logger.info(f"Entry '{title}' not found in group '{group}', creating it")
    new_entry = Entry()
    new_entry.set_title(ProtectedString(title))
    target_group.AddEntry(new_entry)
    return new_entry
