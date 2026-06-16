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
"""Type stubs for the test_wallet pybind11 extension module.

The module exposes TestReadWallet.initTestCase() and
TestReadWallet.cleanupTestCase() so they can be used as setUp / tearDown
in Python's unittest framework::

    import unittest
    import test_wallet

    class MyWalletTest(unittest.TestCase):
        def setUp(self) -> None:
            self._fixture = test_wallet.TestReadWallet()
            self._fixture.initTestCase()

        def tearDown(self) -> None:
            self._fixture.cleanupTestCase()
"""

class TestReadWallet:
    """Fixture that opens and closes the kw2kpass_test wallet via kwalletd5.

    initTestCase() exits the process with code 222 (CTest skip sentinel)
    when prerequisites are not met so that Python unittest suites called
    via CTest are marked *Skipped* rather than *Failed*.
    """

    def __init__(self) -> None: ...
    def initTestCase(self) -> None:
        """Open the kw2kpass_test wallet (setUp).

        Connects to kwalletd5 over D-Bus, extracts the test wallet
        archive into the kwalletd directory, and acquires a per-session
        wallet handle.

        Exits with code 222 when any prerequisite is missing so CTest
        reports the test as *Skipped*.
        """
        ...

    def cleanupTestCase(self) -> None:
        """Delete the test wallet and remove imported files (tearDown).

        Calls kwalletd5's deleteWallet and removes the .kwl / .salt
        files that initTestCase placed in the kwalletd directory.
        """
