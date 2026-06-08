#!/usr/bin/env python3
#
# Copyright 2026 Christoph Pospiech
#
"""Unit tests for get_options_and_defaults() in cli.py."""

import os
import re
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from cli import get_options_and_defaults


class TestGetOptionsDefaults(unittest.TestCase):
    """Tests for get_options_and_defaults() with no arguments (defaults)."""

    def test_default_wallet(self):
        """Wallet name defaults to 'kdewallet' when no argument is given."""
        wallet, *_ = get_options_and_defaults([])
        self.assertEqual(wallet, "kdewallet")

    def test_default_filter(self):
        """Filter defaults to the compiled pattern '.*' (match everything)."""
        _, flt, *_ = get_options_and_defaults([])
        self.assertIsInstance(flt, re.Pattern)
        self.assertEqual(flt.pattern, ".*")

    def test_default_map(self):
        """Folder-to-group map defaults to an empty dict when no --map is given."""
        _, _, mp, *_ = get_options_and_defaults([])
        self.assertEqual(mp, {})

    def test_default_infile(self):
        """Input file defaults to None when --infile is not specified."""
        _, _, _, infile, *_ = get_options_and_defaults([])
        self.assertIsNone(infile)

    def test_default_outfile(self):
        """Output file defaults to 'keepass.kdbx' when --outfile is not specified."""
        _, _, _, _, outfile, _ = get_options_and_defaults([])
        self.assertEqual(outfile, "keepass.kdbx")

    def test_default_passwd(self):
        """Password defaults to None when --passwd is not specified."""
        *_, passwd = get_options_and_defaults([])
        self.assertIsNone(passwd)


class TestGetOptionsWallet(unittest.TestCase):
    """Tests for the --wallet / -w option."""

    def test_long_option(self):
        """--wallet NAME sets the wallet name."""
        wallet, *_ = get_options_and_defaults(["--wallet", "mywallet"])
        self.assertEqual(wallet, "mywallet")

    def test_short_option(self):
        """-w NAME sets the wallet name via the short form."""
        wallet, *_ = get_options_and_defaults(["-w", "otherwallet"])
        self.assertEqual(wallet, "otherwallet")


class TestGetOptionsFilter(unittest.TestCase):
    """Tests for the --filter / -F option."""

    def test_long_option(self):
        """--filter REGEX compiles and stores the given regex pattern."""
        _, flt, *_ = get_options_and_defaults(["--filter", r"^user@host"])
        self.assertIsInstance(flt, re.Pattern)
        self.assertEqual(flt.pattern, r"^user@host")

    def test_short_option(self):
        """-F REGEX sets the filter via the short form."""
        _, flt, *_ = get_options_and_defaults(["-F", r"admin"])
        self.assertEqual(flt.pattern, r"admin")

    def test_invalid_regex_exits(self):
        """An invalid regex pattern causes the parser to exit with an error."""
        with self.assertRaises(SystemExit):
            get_options_and_defaults(["--filter", "[invalid"])


class TestGetOptionsMap(unittest.TestCase):
    """Tests for the --map / -m option."""

    def test_single_folder_only(self):
        """A bare FOLDER name maps the folder to a group of the same name."""
        _, _, mp, *_ = get_options_and_defaults(["--map", "Passwords"])
        self.assertEqual(mp, {"Passwords": "Passwords"})

    def test_folder_colon_group(self):
        """FOLDER:GROUP syntax maps the folder to the specified group name."""
        _, _, mp, *_ = get_options_and_defaults(["--map", "Passwords:MyGroup"])
        self.assertEqual(mp, {"Passwords": "MyGroup"})

    def test_multiple_maps(self):
        """Multiple -m flags accumulate into a single folder-to-group dict."""
        _, _, mp, *_ = get_options_and_defaults(["-m", "FolderA:GroupA", "-m", "FolderB"])
        self.assertEqual(mp, {"FolderA": "GroupA", "FolderB": "FolderB"})

    def test_no_map(self):
        """When no --map flag is given the map is an empty dict."""
        _, _, mp, *_ = get_options_and_defaults([])
        self.assertEqual(mp, {})


class TestGetOptionsInfile(unittest.TestCase):
    """Tests for the --infile / -i option."""

    def test_long_option(self):
        """--infile FILE sets the input file path."""
        _, _, _, infile, *_ = get_options_and_defaults(["--infile", "data.xml"])
        self.assertEqual(infile, "data.xml")

    def test_short_option(self):
        """-i FILE sets the input file path via the short form."""
        _, _, _, infile, *_ = get_options_and_defaults(["-i", "wallet.xml"])
        self.assertEqual(infile, "wallet.xml")


class TestGetOptionsOutfile(unittest.TestCase):
    """Tests for the --outfile / -o option."""

    def test_long_option(self):
        """--outfile FILE sets the output file path."""
        _, _, _, _, outfile, _ = get_options_and_defaults(["--outfile", "out.kdbx"])
        self.assertEqual(outfile, "out.kdbx")

    def test_short_option(self):
        """-o FILE sets the output file path via the short form."""
        _, _, _, _, outfile, _ = get_options_and_defaults(["-o", "result.kdbx"])
        self.assertEqual(outfile, "result.kdbx")


class TestGetOptionsPasswd(unittest.TestCase):
    """Tests for the --passwd / -p option."""

    def test_long_option(self):
        """--passwd PASSWD sets the KeePass password."""
        *_, passwd = get_options_and_defaults(["--passwd", "s3cret"])
        self.assertEqual(passwd, "s3cret")

    def test_short_option(self):
        """-p PASSWD sets the KeePass password via the short form."""
        *_, passwd = get_options_and_defaults(["-p", "hunter2"])
        self.assertEqual(passwd, "hunter2")


class TestGetOptionsVerbosity(unittest.TestCase):
    """Tests for --verbose / -V and --debug / -D options."""

    def test_verbose_does_not_raise(self):
        """--verbose is accepted and the function returns a 6-tuple."""
        result = get_options_and_defaults(["--verbose"])
        self.assertEqual(len(result), 6)

    def test_debug_does_not_raise(self):
        """--debug is accepted and the function returns a 6-tuple."""
        result = get_options_and_defaults(["--debug"])
        self.assertEqual(len(result), 6)

    def test_verbose_and_debug_are_mutually_exclusive(self):
        """Combining --verbose and --debug causes the parser to exit with an error."""
        with self.assertRaises(SystemExit):
            get_options_and_defaults(["--verbose", "--debug"])


class TestGetOptionsReturnType(unittest.TestCase):
    """Tests for the return type and types of each element."""

    def test_returns_tuple_of_six(self):
        """The function always returns a tuple with exactly 6 elements."""
        result = get_options_and_defaults([])
        self.assertIsInstance(result, tuple)
        self.assertEqual(len(result), 6)

    def test_wallet_is_str(self):
        """The wallet element (index 0) is a str."""
        wallet, *_ = get_options_and_defaults([])
        self.assertIsInstance(wallet, str)

    def test_filter_is_pattern(self):
        """The filter element (index 1) is a compiled re.Pattern."""
        _, flt, *_ = get_options_and_defaults([])
        self.assertIsInstance(flt, re.Pattern)

    def test_map_is_dict(self):
        """The map element (index 2) is a dict."""
        _, _, mp, *_ = get_options_and_defaults([])
        self.assertIsInstance(mp, dict)

    def test_outfile_is_str(self):
        """The outfile element (index 4) is a str."""
        _, _, _, _, outfile, _ = get_options_and_defaults([])
        self.assertIsInstance(outfile, str)


if __name__ == "__main__":
    unittest.main()
