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
"""Type stubs for the kwallet pybind11 extension module.

Generated to suppress false PyLance import errors.
"""

from collections.abc import Iterator

class WalletHandle:
    def __init__(self, name: str) -> None: ...
    def wallet_is_open(self) -> bool: ...
    def wallet_name(self) -> str: ...
    def hasFolder(self, folder: str) -> bool: ...
    def setFolder(self, folder: str) -> bool: ...
    def currentFolder(self) -> str: ...
    def password(self, key: str) -> str: ...
    def username(self, key: str) -> str: ...
    def hostname(self, key: str) -> str: ...

class WalletIterator:
    def __init__(self, wallet: WalletHandle) -> None: ...
    def __iter__(self) -> Iterator[str]: ...
