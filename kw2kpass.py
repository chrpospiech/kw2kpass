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
import getopt

# logger for verbose mode
import logging
import re
import sys

from kwallet import WalletHandle, WalletIterator

logging.basicConfig(
    level=logging.WARNING,
    format="%(asctime)s %(levelname)s %(message)s",
)
logger = logging.getLogger("kw2pwsafe")

# Python KF5/Qt5 interface ** access to Kwallet
# Python interface to Password Safe V3 files
# from pwsafe import PWSrecordHandle
# from pwsafe import PWSfileHandle
# This script allows to filter the entries with
# a given regular expression.
# This requires the module re


def Usage():
    """Print usage information."""
    usageString = """
    kw2pkpass.py *** Read Kwallet folders and translate into
				 	KeePassXC databases

    kw2pkpass.py [options] [arguments]
    where
    * only argument is the file name of the
	  KeePassXC database - if not specified via option -f
	* options are
	-h, --help		Print this help text.
	-v, --version	Print version number and exit.
	-w, --wallet	Kwallet name (default kdewallet).
	-m, --map		format: folder_name[:group]
					Kwallet folder folder_name
					(default folder_name Firefox).
					Entries in this folder are stored in
					group "group" in KeePassXC database.
					Several --map option entries possible.
	-f, --filename	Name of the KeePassXC database.
	-p, --password	Password of the KeePassXC database.
					*** This is a required option. ***
	-F, --filter	Allow only records containing a substring
					that matches a given regular expression
	-a, --append	Append to an existing file
					(temporarily disabled)
	-V, --verbose	Verbose mode
	-D, --debug		Debug mode (very verbose logging)
	"""
    print(usageString)


def getOptionsAndDefaults():
    """Sets defaults and interprets options.

    @summary: Sets defaults and interprets options
    @return: Kwallet, Kwallet folder, filter,
    @return: safe name, safe password, safe group.
    """
    Wname = "kdewallet"
    WsafeMap = {}
    Wfilter = re.compile(r".*")
    loglevel = "WARNING"
    # append	  = False (not yet implemented)
    try:
        opts, args = getopt.getopt(
            sys.argv[1:],
            "hw:f:p:m:F:vVD",
            ["help", "verbose", "debug", "version", "wallet=", "map=", "filename=", "password=", "filter="],
        )
    except getopt.GetoptError as err:
        # print help information and exit:
        print(str(err))  # will print something like "option -a not recognized"
        Usage()
        sys.exit(2)
    # The file name can be either passed as argument
    # or with option -f
    if args:
        args[0]
    for o, a in opts:
        if o in {"-V", "--verbose"}:
            loglevel = "INFO"
        elif o in {"-D", "--debug"}:
            loglevel = "DEBUG"
        #        elif o in {"-v", "--version"}:
        #            print("kw2pwsafe version {}.".format("2.0"))
        #            sys.exit()
        #        elif o in {"-h", "--help"}:
        #            Usage()
        #           sys.exit()
        elif o in {"-w", "--wallet"}:
            Wname = a
        #       elif o in {"-f", "--filename"}:
        #            safeName = a
        #       elif o in {"-p", "--password"}:
        #            safePword = a
        #       elif o in {"-g", "--group"}:
        #            pass
        elif o in {"-F", "--filter"}:
            Wfilter = re.compile(a)
        elif o in {"-m", "--map"}:
            StrList = a.split(":")
            one = 1
            two = 2
            if len(StrList) == one:
                WsafeMap[StrList[0]] = StrList[0]
            elif len(StrList) == two:
                WsafeMap[StrList[0]] = StrList[1]
            else:
                raise AssertionError("wrong map entry")
        else:
            raise AssertionError(f"unhandled option {o}")
    #    if not safePword:
    #        print("No password specified")
    #        Usage()
    #        sys.exit(2)
    #    if len(WsafeMap) == 0:
    #        WsafeMap["Firefox"] = "Firefox"
    logger.setLevel(getattr(logging, loglevel))
    logger.info(f"Logging level changed to {loglevel}")
    return (Wname, Wfilter, WsafeMap)


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


# def OpenSafe(safeName, safePword):
#    """
#    @summary: Opens KeePassXC database "safeName"
#    @return: Handle to the open database
#    """
#    pws = PWSfileHandle(safeName, safePword)
#    if not pws.statusOK():
#        print("KeePassXC database {0} not found".format(safeName))
#        sys.exit(2)
#    else:
#       logger.info("KeePassXC database {0} opened".format(safeName))
#    return pws


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
    if wallet.hasFolder(Wfolder):
        logger.info(f"folder {Wfolder} found")
    else:
        print(f"No folder {Wfolder} found")
        sys.exit(2)
    if wallet.setFolder(Wfolder):
        logger.info(f"folder {Wfolder} set")
    else:
        print(f"No folder {Wfolder} could not be set")
        sys.exit(2)
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
        # if not Wfilter.search(str(uid) + str(host)):
        #    logger.info("host " + str(host) + " has been filtered out")
        #    continue
        logger.info("Printing record for " + title)
        print("On host " + str(host) + ":")
        print("u = " + str(uid) + " pw = " + str(passwd))


def main():
    """Main loop of extracting from Kwallet and adding to KeePassXC database.

    Output is temporarily written to stdout for debugging purposes.
    The final version will write directly to the database.
    """
    Wname, Wfilter, WsafeMap = getOptionsAndDefaults()
    # psafe = OpenSafe(safeName, safePword)
    wallet = OpenWallet(Wname)
    # for Wfolder in WsafeMap:
    # pgroup = WsafeMap[Wfolder]
    # CopyWalletFolder(wallet, Wfolder, Wfilter, psafe, pgroup)
    # psafe.Close()
    for Wfolder in WsafeMap:
        PrintWalletFolder(wallet, Wfolder, Wfilter)


if __name__ == "__main__":
    main()
