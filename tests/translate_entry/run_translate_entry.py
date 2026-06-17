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
"""Unit tests for translate_entry() in entry_data.py."""

import unittest

from entry_data import translate_entry


class TestTranslateEntryMissingKey(unittest.TestCase):
    """translate_entry() returns an empty dict when a required key is absent."""

    def test_missing_title(self):
        """Returns an empty dict when 'title' is absent."""
        result = translate_entry({"username": "user", "hostname": "host", "password": "pw"})
        self.assertEqual(result, {})

    def test_missing_username(self):
        """Returns an empty dict when 'username' is absent."""
        result = translate_entry({"title": "t", "hostname": "host", "password": "pw"})
        self.assertEqual(result, {})

    def test_missing_hostname(self):
        """Returns an empty dict when 'hostname' is absent."""
        result = translate_entry({"title": "t", "username": "user", "password": "pw"})
        self.assertEqual(result, {})

    def test_missing_password(self):
        """Returns an empty dict when 'password' is absent."""
        result = translate_entry({"title": "t", "username": "user", "hostname": "host"})
        self.assertEqual(result, {})

    def test_empty_dict(self):
        """Returns an empty dict when the input dict is empty."""
        result = translate_entry({})
        self.assertEqual(result, {})


class TestTranslateEntryNoPassword(unittest.TestCase):
    """translate_entry() returns an empty dict when the password is 'n/a'."""

    def test_password_na(self):
        """Returns an empty dict when the password value is 'n/a'."""
        result = translate_entry(
            {"title": "example.com", "username": "user", "hostname": "example.com", "password": "n/a"}
        )
        self.assertEqual(result, {})


class TestTranslateEntryRedundantHostname(unittest.TestCase):
    """translate_entry() strips the redundant hostname from the title."""

    def test_double_hostname_http(self):
        """Strips ',,http://…' suffix from the title."""
        result = translate_entry(
            {
                "title": "user@example.com,,http://example.com",
                "username": "user",
                "hostname": "example.com",
                "password": "secret",
            }
        )
        self.assertEqual(result["title"], "user@example.com")

    def test_no_redundant_hostname_unchanged(self):
        """Leaves the title unchanged when no redundant hostname is present."""
        result = translate_entry(
            {"title": "example.com", "username": "user", "hostname": "example.com", "password": "secret"}
        )
        self.assertEqual(result["title"], "example.com")


class TestTranslateEntryPasswordEntry(unittest.TestCase):
    """translate_entry() handles password entries (empty username) correctly."""

    def test_empty_username_with_at_sign(self):
        """Title 'user@host' is split into uid and host when username is empty."""
        result = translate_entry({"title": "alice@example.com", "username": "", "hostname": "", "password": "secret"})
        self.assertEqual(result["username"], "alice")
        self.assertEqual(result["hostname"], "example.com")
        self.assertEqual(result["title"], "alice@example.com")
        self.assertEqual(result["password"], "secret")

    def test_empty_username_without_at_sign(self):
        """Title without '@' leaves username empty and sets hostname to title."""
        result = translate_entry({"title": "example.com", "username": "", "hostname": "", "password": "secret"})
        self.assertEqual(result["username"], "")
        self.assertEqual(result["hostname"], "example.com")
        self.assertEqual(result["title"], "example.com")

    def test_empty_username_redundant_hostname_then_split(self):
        """Redundant hostname is removed first, then '@' split is applied."""
        result = translate_entry(
            {
                "title": "alice@example.com,,https://example.com",
                "username": "",
                "hostname": "",
                "password": "secret",
            }
        )
        self.assertEqual(result["title"], "alice@example.com")
        self.assertEqual(result["username"], "alice")
        self.assertEqual(result["hostname"], "example.com")


class TestTranslateEntryMapEntry(unittest.TestCase):
    """translate_entry() passes map entries (non-empty username) through unchanged."""

    def test_map_entry_username_and_hostname_preserved(self):
        """All fields are returned unchanged for a map entry with a non-empty username."""
        result = translate_entry(
            {"title": "My Service", "username": "bob", "hostname": "service.example.com", "password": "s3cr3t"}
        )
        self.assertEqual(result["title"], "My Service")
        self.assertEqual(result["username"], "bob")
        self.assertEqual(result["hostname"], "service.example.com")
        self.assertEqual(result["password"], "s3cr3t")

    def test_map_entry_returns_all_keys(self):
        """Result contains all four expected keys for a valid map entry."""
        result = translate_entry({"title": "t", "username": "u", "hostname": "h", "password": "p"})
        self.assertIn("title", result)
        self.assertIn("username", result)
        self.assertIn("hostname", result)
        self.assertIn("password", result)


if __name__ == "__main__":
    unittest.main()
