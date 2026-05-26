#
# Copyright 2026 Christoph Pospiech
#
"""Utility functions for accessing KWallet."""

import logging
import sys

from kwallet import WalletHandle

logger = logging.getLogger("kw2pwsafe")


def OpenWallet(Wname):
    """Open Kwallet "Wname".

    @summary: Opens Kwallet "Wname"
    @return: Handle to the open wallet.
    """
    wh = WalletHandle(Wname)
    if not wh.wallet_is_open():
        print(f"Wallet {Wname} not found")
        sys.exit(2)
    else:
        logger.info(f"Wallet {Wname} opened")
    return wh


def SetWalletFolder(wallet, Wfolder):
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
        sys.exit(2)
