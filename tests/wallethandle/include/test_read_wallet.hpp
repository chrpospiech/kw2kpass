/*
 * test_read_wallet.hpp
 *
 * Declaration of the TestReadWallet Qt Test class.
 *
 * initTestCase() and cleanupTestCase() are public slots so that the
 * pybind11 module (test_wallet) can call them as setUp / tearDown
 * from Python's unittest framework.  All other test methods remain
 * private slots and are discovered only by Qt Test's meta-object
 * machinery.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TEST_READ_WALLET_HPP
#define TEST_READ_WALLET_HPP

#include <QByteArray>
#include <QObject>
#include <QString>

class QDBusInterface;

class TestReadWallet : public QObject {
    Q_OBJECT

  private:
    int m_handle = -1;
    QDBusInterface *m_iface = nullptr;

    static QByteArray passwordHash();
    static QString walletDir();
    static QString testDataArchive();
    void removeImportedFiles();

  public slots:
    // Public so that the pybind11 test_wallet module can invoke them
    // as setUp / tearDown from Python's unittest framework.
    void initTestCase();
    void cleanupTestCase();

  private slots:
    // ── Folder existence ──────────────────────────────────────────────────────
    void hasFolderPasswords();
    void hasFolderFormData();
    void hasFolderTestFolder();

    // ── Password-type entries (Passwords folder) ──────────────────────────────
    void hasEntryExample1();
    void passwordExample1IsEmpty();
    void usernameExample1IsEmpty();
    void hasEntryExample2();
    void passwordExample2IsEmpty();

    // ── Map-type entry "test_map" (Passwords folder) ──────────────────────────
    void hasEntryTestMap();
    void passwordTestMap();
    void usernameTestMap();
    void hostnameTestMap();

    // ── Password-type entry in test_folder ────────────────────────────────────
    void hasEntryExample3();
    void passwordExample3IsEmpty();

    // ── WalletIterator ────────────────────────────────────────────────────────
    void iteratorPasswordsFolder();
    void iteratorTestFolder();
};

#endif // TEST_READ_WALLET_HPP
