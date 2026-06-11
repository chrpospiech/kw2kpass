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
"""Utility functions for accessing KWallet."""

import logging

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
