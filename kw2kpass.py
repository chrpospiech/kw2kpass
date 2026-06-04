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

from cli import getOptionsAndDefaults
from keepass_utils import close_database, open_database
from kwallet import WalletIterator
from wallet_utils import OpenWallet, SetWalletFolder

logging.basicConfig(
    level=logging.WARNING,
    format="%(asctime)s %(levelname)s %(message)s",
)
logger = logging.getLogger("kw2kpass")


# def CopyWalletFolder(wallet, Wfolder, Wfilter, psafe, pgroup):
#     """
#     @summary: Copy filtered entries from Wname:Wfolder
#                       into psafe:pgroup
#     @param wallet: Kwallet handle
#     @param Wfolder: Kwallet folder
#     @param Wfilter: Filter to filter entries
#     @param psafe: Password Safe handle
#     @param pgroup: Password Safe group
#     """
#     if wallet.hasFolder(Wfolder):
#         logger.info("folder {0} found".format(Wfolder))
#     else:
#         print("No folder {0} found".format(Wfolder))
#         sys.exit(2)
#     if wallet.setFolder(Wfolder):
#        logger.info("folder {0} set".format(Wfolder))
#    else:
#        print("No folder {0} could not be set".format(Wfolder))
#        sys.exit(2)
# [...]
# newRecord = PWSrecordHandle()
# newRecord.Clear()
# newRecord.SetTitle(title)
# newRecord.SetGroup(pgroup)
# newRecord.SetURL(host)
# newRecord.SetUser(uid)
# newRecord.SetEmail(email)
# newRecord.SetPassword(passwd)
# newRecord.CreateUUID()
# psafe.WriteRecord(newRecord)


def PrintWalletFolder(wallet, Wfolder, Wfilter):
    """Print filtered entries from Wname:Wfolder.

    @summary: Print filtered entries from Wname:Wfolder
    @param wallet: Kwallet handle
    @param Wfolder: Kwallet folder
    @param Wfilter: Filter to filter entries.
    """
    SetWalletFolder(wallet, Wfolder)
    for e in WalletIterator(wallet):
        uidFilter = re.compile(r"\@")
        title = str(e)
        uid = wallet.username(e)
        host = wallet.hostname(e)
        passwd = wallet.password(e)
        """
		Entries in Kwallet can be password or map entries;
		password entries return empty username and hostname.
		"""
        if not str(uid):
            logger.debug("entry " + title + " is an ordinary password entry")
            titleParts = re.split(r"\@", title)
            if len(titleParts) > 1:
                uid = titleParts[0]
                host = titleParts[1]
            else:
                uid = ""
                host = titleParts[0]
        else:
            logger.debug("entry " + title + " is a map entry")
        # Skip the entry that lists the URLs without
        # password saving
        if passwd == "n/a":
            logger.debug("entry " + title + "  has no password")
            continue
        logger.debug("On host " + str(host) + ":")
        logger.debug("u = " + str(uid) + " pw = " + str(passwd))
        if uidFilter.search(str(uid)):
            logger.debug("uid " + str(uid) + " is an email address")
            str(uid)
        else:
            logger.debug("uid " + str(uid) + " is not an email address")
        if not Wfilter.search(str(uid) + str(host)):
            logger.info("host " + str(host) + " has been filtered out")
            continue
        logger.info("Printing record for " + title)
        print("On host " + str(host) + ":")
        print("u = " + str(uid) + " pw = " + str(passwd))


def main():
    """Main loop of extracting from Kwallet and adding to KeePassXC database.

    Output is temporarily written to stdout for debugging purposes.
    The final version will write directly to the database.
    """
    Wname, Wfilter, WsafeMap, Kinfile, Koutfile, Kpasswd = getOptionsAndDefaults()
    database = open_database(Kinfile, Kpasswd)
    wallet = OpenWallet(Wname)
    # for Wfolder in WsafeMap:
    # pgroup = WsafeMap[Wfolder]
    # CopyWalletFolder(wallet, Wfolder, Wfilter, psafe, pgroup)
    close_database(database, Koutfile, Kpasswd)
    for Wfolder in WsafeMap:
        PrintWalletFolder(wallet, Wfolder, Wfilter)


if __name__ == "__main__":
    main()
