#
# Copyright 2026 Christoph Pospiech
#
"""Command-line argument parsing for kw2kpass."""

import argparse
import logging
import re

logger = logging.getLogger("kw2pwsafe")


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="kw2kpass",
        description="Read Kwallet folders and translate into KeePassXC databases.",
    )
    parser.add_argument(
        "-w", "--wallet",
        dest="wallet",
        default="kdewallet",
        metavar="NAME",
        help="Kwallet name (default: kdewallet)",
    )
    parser.add_argument(
        "-m", "--map",
        dest="maps",
        action="append",
        default=[],
        metavar="FOLDER[:GROUP]",
        help=(
            "Kwallet folder to KeePassXC group mapping. "
            "FOLDER is the Kwallet folder name; GROUP is the target group "
            "(defaults to FOLDER when omitted). "
            "May be specified multiple times."
        ),
    )
    parser.add_argument(
        "-F", "--filter",
        dest="filter",
        default=".*",
        metavar="REGEX",
        help="Only process entries whose uid+host matches REGEX (default: .*).",
    )
    verbosity = parser.add_mutually_exclusive_group()
    verbosity.add_argument(
        "-V", "--verbose",
        dest="verbose",
        action="store_true",
        default=False,
        help="Verbose mode (INFO logging).",
    )
    verbosity.add_argument(
        "-D", "--debug",
        dest="debug",
        action="store_true",
        default=False,
        help="Debug mode (DEBUG logging, very verbose).",
    )
    return parser


def getOptionsAndDefaults(argv=None):
    """Parse command-line options and return runtime configuration.

    @summary: Parses argv (defaults to sys.argv[1:]) and returns
              the wallet name, compiled filter regex, and folder→group map.
    @param argv: Argument list to parse (None → sys.argv[1:]).
    @return: Tuple (wallet_name: str, filter: re.Pattern, map: dict[str, str]).
    """
    parser = _build_parser()
    args = parser.parse_args(argv)

    # Determine log level
    if args.debug:
        loglevel = "DEBUG"
    elif args.verbose:
        loglevel = "INFO"
    else:
        loglevel = "WARNING"
    logger.setLevel(getattr(logging, loglevel))
    logger.info(f"Logging level changed to {loglevel}")

    # Compile filter regex
    try:
        wfilter = re.compile(args.filter)
    except re.error as exc:
        parser.error(f"Invalid filter regex: {exc}")

    # Build folder→group map
    wsafe_map: dict[str, str] = {}
    for entry in args.maps:
        parts = entry.split(":", maxsplit=1)
        if len(parts) == 1:
            wsafe_map[parts[0]] = parts[0]
        else:
            wsafe_map[parts[0]] = parts[1]

    return (args.wallet, wfilter, wsafe_map)
