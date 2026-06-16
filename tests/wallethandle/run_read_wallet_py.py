#!/usr/bin/env python3
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
"""Integration tests for WalletHandle against a live kwalletd5 daemon.

Mirrors the test cases defined in tests/wallethandle/src/test_read_wallet.cpp.

The setUp / tearDown lifecycle is provided by the test_wallet pybind11 module
which opens the kw2kpass_test wallet non-interactively via pamOpen and closes
it again afterwards.  When prerequisites are not met (kwalletd5 absent, wallet
archive missing, etc.) initTestCase() exits the process with code 222 so that
CTest reports the test as *Skipped* rather than *Failed*.

Expected wallet contents (see tests/test_data/kw2kpass_test.xml):
  Folder "Form Data"   – empty
  Folder "Passwords"   – entries: example1 (Password), example2 (Password),
                                   test_map (Map)
  Folder "test_folder" – entries: example3 (Password)
"""

import unittest

import kwallet
import test_wallet

WALLET_NAME = "kw2kpass_test"
FOLDER_PASSWORDS = "Passwords"
FOLDER_FORM_DATA = "Form Data"
FOLDER_TEST = "test_folder"


def _entry_names(wh: kwallet.WalletHandle) -> set[str]:
    """Return the set of entry names in the wallet's current folder."""
    return {name for name in kwallet.WalletIterator(wh)}


class PythonWalletTest(unittest.TestCase):
    """Integration tests for WalletHandle via the test_wallet pybind11 module."""

    def setUp(self) -> None:
        """Open the test wallet via pamOpen before each test."""
        self._fixture = test_wallet.TestReadWallet()
        self._fixture.initTestCase()

    def tearDown(self) -> None:
        """Close the test wallet after each test."""
        self._fixture.cleanupTestCase()

    # ── Folder existence ──────────────────────────────────────────────────────

    def test_hasFolderPasswords(self) -> None:
        """Wallet contains the Passwords folder."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.wallet_is_open())
        self.assertTrue(wh.hasFolder(FOLDER_PASSWORDS))

    def test_hasFolderFormData(self) -> None:
        """Wallet contains the Form Data folder."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.wallet_is_open())
        self.assertTrue(wh.hasFolder(FOLDER_FORM_DATA))

    def test_hasFolderTestFolder(self) -> None:
        """Wallet contains the test_folder folder."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.wallet_is_open())
        self.assertTrue(wh.hasFolder(FOLDER_TEST))

    # ── Password-type entries (Passwords folder) ──────────────────────────────

    def test_hasEntryExample1(self) -> None:
        """Passwords folder contains the example1 entry."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertIn("example1", _entry_names(wh))

    def test_passwordExample1IsEmpty(self) -> None:
        """example1 password field is empty."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertEqual(wh.password("example1"), "")

    def test_usernameExample1IsEmpty(self) -> None:
        """example1 username field is empty (Password-type entries carry no username)."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        # Password-type entries carry no username field.
        self.assertEqual(wh.username("example1"), "")

    def test_hasEntryExample2(self) -> None:
        """Passwords folder contains the example2 entry."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertIn("example2", _entry_names(wh))

    def test_passwordExample2IsEmpty(self) -> None:
        """example2 password field is empty."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertEqual(wh.password("example2"), "")

    # ── Map-type entry "test_map" (Passwords folder) ──────────────────────────

    def test_hasEntryTestMap(self) -> None:
        """Passwords folder contains the test_map entry."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertIn("test_map", _entry_names(wh))

    def test_passwordTestMap(self) -> None:
        """test_map password field matches the Map entry value."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertEqual(wh.password("test_map"), "trivial_and_not_valid")

    def test_usernameTestMap(self) -> None:
        """test_map username field matches the Map entry value."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertEqual(wh.username("test_map"), "not@existing")

    def test_hostnameTestMap(self) -> None:
        """test_map hostname field matches the Map entry value."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        self.assertEqual(wh.hostname("test_map"), "https://www.bahn.de/")

    # ── Password-type entry in test_folder ────────────────────────────────────

    def test_hasEntryExample3(self) -> None:
        """test_folder contains the example3 entry."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_TEST))
        self.assertIn("example3", _entry_names(wh))

    def test_passwordExample3IsEmpty(self) -> None:
        """example3 password field is empty."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_TEST))
        self.assertEqual(wh.password("example3"), "")

    # ── WalletIterator ────────────────────────────────────────────────────────

    def test_iteratorPasswordsFolder(self) -> None:
        """WalletIterator yields all three entries in the Passwords folder."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_PASSWORDS))
        found = _entry_names(wh)
        self.assertIn("example1", found)
        self.assertIn("example2", found)
        self.assertIn("test_map", found)
        self.assertEqual(len(found), 3)

    def test_iteratorTestFolder(self) -> None:
        """WalletIterator yields exactly one entry in test_folder."""
        wh = kwallet.WalletHandle(WALLET_NAME)
        self.assertTrue(wh.setFolder(FOLDER_TEST))
        found = _entry_names(wh)
        self.assertIn("example3", found)
        self.assertEqual(len(found), 1)


if __name__ == "__main__":
    unittest.main()
