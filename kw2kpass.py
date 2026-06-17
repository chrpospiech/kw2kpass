#! /usr/bin/env python3
#
# Copyright 2026 Christoph Pospiech
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

from cli import get_options_and_defaults
from entry_data import get_entry_data
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

    Args:
        wallet: Kwallet handle
        Wfolder: Kwallet folder
        Wfilter: Filter to filter entries
        dbase: KeePassXC database handle
        group: KeePassXC group name (created under root if absent)

    Returns:
        None
    """
    set_wallet_folder(wallet, Wfolder)
    for e in WalletIterator(wallet):
        entry_data = get_entry_data(wallet, e)
        if not entry_data:
            continue
        title = entry_data["title"]
        uid = entry_data["username"]
        host = entry_data["hostname"]
        passwd = entry_data["password"]
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
    Wname, Wfilter, WsafeMap, Kinfile, Koutfile, Kinpasswd, Koutpasswd = get_options_and_defaults()
    database = open_database(Kinfile, Koutfile, Kinpasswd)
    wallet = open_wallet(Wname)
    for Wfolder in WsafeMap:
        group = WsafeMap[Wfolder]
        copy_wallet_folder(wallet, Wfolder, Wfilter, database, group)
    close_database(database, Koutfile, Koutpasswd)


if __name__ == "__main__":
    main()
