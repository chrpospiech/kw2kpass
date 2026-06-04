#! /usr/bin/env python3
#
# Copyright 2026 Christoph Pospiech
#
"""Read Kwallet folders and translate into KeePassXC databases.

@author:  Christoph Pospiech
@copyright: Christoph Pospiech 2026
@contact: chrpospiech@magenta.de
@version: 0.1.0
"""

# Standard python modules
# logger for verbose mode
import logging
import re

from cli import get_options_and_defaults
from keepass_utils import close_database, find_or_create_entry, open_database
from kwallet import WalletIterator
from wallet_utils import open_wallet, set_wallet_folder

logging.basicConfig(
    level=logging.WARNING,
    format="%(asctime)s %(levelname)s %(message)s",
)
logger = logging.getLogger("kw2kpass")


def copy_wallet_folder(wallet, Wfolder, Wfilter, dbase, group):
    """Copy filtered entries from Wname:Wfolder into KeePass group.

    @param wallet: Kwallet handle
    @param Wfolder: Kwallet folder
    @param Wfilter: Filter to filter entries
    @param dbase: KeePassXC database handle
    @param group: KeePassXC group name (created under root if absent)
    """
    set_wallet_folder(wallet, Wfolder)
    for e in WalletIterator(wallet):
        title = str(e)
        uid = wallet.username(e)
        host = wallet.hostname(e)
        passwd = wallet.password(e)
        # Skip the entry that lists the URLs without password saving
        if passwd == "n/a":
            logger.debug(f"entry {title}  has no password")
            continue
        """
		Entries in Kwallet can be password or map entries;
		password entries return empty username and hostname.
		"""
        if not str(uid):
            logger.debug(f"entry {title} is an ordinary password entry")
            titleParts = re.split(r"\@", title)
            if len(titleParts) > 1:
                uid = titleParts[0]
                host = titleParts[1]
            else:
                uid = ""
                host = titleParts[0]
        else:
            logger.debug(f"entry {title} is a map entry")
        # Skip the entry if it does not match the filter
        if not Wfilter.search(f"{uid}{host}"):
            logger.info(f"entry {title} has been filtered out")
            continue
        logger.info(f"Copying record for {title}")
        logger.debug(f"On host {host}:")
        logger.debug(f"u = {uid} (password redacted)")
        entry = find_or_create_entry(dbase, group, title)
        entry.url = str(host)
        entry.username = str(uid)
        entry.password = str(passwd)


def main():
    """Main loop of extracting from Kwallet and adding to KeePassXC database."""
    Wname, Wfilter, WsafeMap, Kinfile, Koutfile, Kpasswd = get_options_and_defaults()
    database = open_database(Kinfile, Koutfile, Kpasswd)
    wallet = open_wallet(Wname)
    for Wfolder in WsafeMap:
        group = WsafeMap[Wfolder]
        copy_wallet_folder(wallet, Wfolder, Wfilter, database, group)
    close_database(database, Koutfile, Kpasswd)


if __name__ == "__main__":
    main()
