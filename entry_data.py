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
"""Helper for setting up KeePass entries."""

import logging
import re

from kwallet import WalletHandle

logger = logging.getLogger("kw2kpass")


def get_entry_data(wallet: WalletHandle, key: str) -> dict[str, str]:
    """Extract and process the data for a given entry key.

    Args:
        wallet: The wallet handle to access the entry data.
        key: The key of the entry to retrieve.

    Returns:
        A dictionary containing the entry data, or an empty dictionary if an error occurs.
    """
    try:
        wallet_title = str(key)
        logger.debug(f"Processing entry '{wallet_title}' with key '{key}'")
        uid = wallet.username(key)
        host = wallet.hostname(key)
        passwd = wallet.password(key)
    except RuntimeError as e:
        logger.error(f"Failed to get data for entry '{key}': {e}")
        return {}
    # Skip the entry that lists the URLs without password saving
    if passwd == "n/a":
        logger.debug(f"entry {wallet_title}  has no password")
        return {}
    # Entries in Kwallet can be password or map entries;
    # password entries return empty username and hostname.
    if not str(uid):
        logger.debug(f"entry {wallet_title} is an ordinary password entry")
        titleParts = re.split(r"\@", wallet_title)
        if len(titleParts) > 1:
            uid = titleParts[0]
            host = titleParts[1]
        else:
            uid = ""
            host = titleParts[0]
    else:
        logger.debug(f"entry {wallet_title} is a map entry")
    return {
        "title": wallet_title,
        "username": uid,
        "hostname": host,
        "password": passwd,
    }
