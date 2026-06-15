/*
 * test_read_wallet.cpp
 *
 * Integration tests for WalletHandle against a live kwalletd5 daemon.
 *
 * Wallet setup (initTestCase):
 *
 *   IMPORT PATH  – tests/test_data/kw2kpass_test is a tar archive
 *     containing kw2kpass_test.kwl (Blowfish-encrypted wallet) and a
 *     kw2kpass_test.salt placeholder.  The wallet was sealed with the
 *     first 56 bytes of SHA-512("Only4Test") as the Blowfish key.
 *     initTestCase() extracts both files into the kwalletd wallet
 *     directory so the daemon can open the wallet by name.
 *
 *   The wallet is opened non-interactively via kwalletd5's pamOpen DBus
 *   method (first 56 bytes of SHA-512("Only4Test") as the key material)
 *   without any GUI dialog.
 *
 *   initTestCase() prints a "SKIP   :" message and exits with code 222
 *   so CTest reports the integration test as "Skipped" when:
 *     - kwalletd5 is not registered on the session bus,
 *     - the DBus interface to kwalletd5 is not valid,
 *     - the test wallet archive is not found in the source tree,
 *     - tar extraction of the wallet archive fails,
 *     - pamOpen does not open the wallet within 3 s, or
 *     - a kwalletd handle cannot be obtained.
 *
 * Cleanup (cleanupTestCase):
 *   deleteWallet removes the wallet from the daemon and deletes the
 *   .kwl / .salt files it placed in the wallet directory.
 *
 * Expected wallet contents – see tests/test_data/kw2kpass_test.xml:
 *   Folder "Form Data"   – empty
 *   Folder "Passwords"   – entries: example1 (Password), example2 (Password),
 *                                    test_map (Map)
 *   Folder "test_folder" – entries: example3 (Password)
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

#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

#include <WalletHandle>

#include <TestReadWallet>

// ── Constants ────────────────────────────────────────────────────────────────

namespace {
const char WALLET_NAME[] = "kw2kpass_test";
const char WALLET_APPID[] = "kw2kpass_unit_test";
const char WALLET_PASSWORD[] = "Only4Test";
const char FOLDER_PASSWORDS[] = "Passwords";
const char FOLDER_FORM_DATA[] = "Form Data";
const char FOLDER_TEST[] = "test_folder";
} // namespace

// ── Private helpers ─────────────────────────────────────────────────────────

// Key material for pamOpen.
// kwalletd's openPreHashed() only accepts 20, 40, or 56 bytes (the latter
// being PBKDF2_SHA512_KEYSIZE).  A raw SHA-512 digest is 64 bytes and is
// therefore rejected with error -42.  We truncate to 56 bytes so the size
// check passes and the value is used directly as the Blowfish key.
// (pam_kwallet5 derives its 56-byte key via PBKDF2-HMAC-SHA512(password,
// salt-file, 50000 iters) — here we skip the PBKDF2 step and use the
// first 56 bytes of the raw SHA-512 digest as a deterministic test key.)
QByteArray TestReadWallet::passwordHash() {
    return QCryptographicHash::hash(WALLET_PASSWORD, QCryptographicHash::Sha512).left(56);
}

// Directory where kwalletd stores .kwl files on this system.
QString TestReadWallet::walletDir() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kwalletd");
}

// Path to the test-data tar archive in the source tree.
// KWALLET_TEST_DATA_DIR is set by CMake to tests/test_data.
QString TestReadWallet::testDataArchive() {
    return QLatin1String(KWALLET_TEST_DATA_DIR) + QLatin1String("/kw2kpass_test");
}

// Remove the .kwl and .salt files placed in the wallet directory.
void TestReadWallet::removeImportedFiles() {
    QFile::remove(walletDir() + QLatin1String("/kw2kpass_test.kwl"));
    QFile::remove(walletDir() + QLatin1String("/kw2kpass_test.salt"));
}

// ── initTestCase ─────────────────────────────────────────────────────────────

void TestReadWallet::initTestCase() {
    // ── Step 1: verify kwalletd5 is reachable ────────────────────────────────
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || !bus.interface()->isServiceRegistered("org.kde.kwalletd5")) {
        fprintf(stdout, "SKIP   : kwalletd5 is not registered on the session bus\n");
        std::exit(222);
    }

    m_iface = new QDBusInterface("org.kde.kwalletd5", "/modules/kwalletd5", "org.kde.KWallet", bus, this);
    if (!m_iface->isValid()) {
        fprintf(stdout, "SKIP   : DBus interface to kwalletd5 is not valid\n");
        std::exit(222);
    }

    // ── Step 2: import wallet archive ────────────────────────────────────────
    // Extract kw2kpass_test.kwl and kw2kpass_test.salt from the source-tree
    // archive into the kwalletd directory so the daemon can find the wallet.
    const QString archive = testDataArchive();
    if (!QFile::exists(archive)) {
        fprintf(stdout, "SKIP   : test wallet archive not found: %s\n", archive.toUtf8().constData());
        std::exit(222);
    }
    QDir().mkpath(walletDir());
    QProcess tar;
    tar.start("tar", {"-xf", archive, "-C", walletDir(), "kw2kpass_test.kwl", "kw2kpass_test.salt"});
    if (!tar.waitForFinished(10000) || tar.exitCode() != 0) {
        removeImportedFiles();
        fprintf(stdout, "SKIP   : Failed to extract the test wallet archive into the kwalletd directory\n");
        std::exit(222);
    }

    // ── Step 3: open the wallet non-interactively via pamOpen ────────────────
    // pamOpen takes SHA-512 of the user password (matching pam_kwallet5's
    // hash computation) and opens or creates the wallet without any GUI dialog.
    // The call is Q_NOREPLY, so we poll isOpen() for up to 3 s.
    m_iface->call(QDBus::NoBlock, "pamOpen", QString(WALLET_NAME), passwordHash(), 0);

    bool opened = false;
    for (int i = 0; i < 30; ++i) {
        QTest::qWait(100);
        QDBusReply<bool> reply = m_iface->call("isOpen", QString(WALLET_NAME));
        if (reply.isValid() && reply.value()) {
            opened = true;
            break;
        }
    }

    if (!opened) {
        removeImportedFiles();
        fprintf(stdout, "SKIP   : pamOpen did not open the test wallet within 3 s "
                        "(daemon policy restriction or wrong password on the imported file)\n");
        std::exit(222);
    }

    // ── Step 4: acquire a per-session DBus handle ────────────────────────────
    // The wallet is already open in the daemon, so open() returns immediately
    // without showing any dialog.
    QDBusReply<int> openReply = m_iface->call("open", QString(WALLET_NAME), qlonglong(0), QString(WALLET_APPID));
    if (!openReply.isValid() || openReply.value() < 0) {
        removeImportedFiles();
        fprintf(stdout, "SKIP   : Could not obtain a kwalletd handle for the test wallet\n");
        std::exit(222);
    }
    m_handle = openReply.value();
}

// ── cleanupTestCase ───────────────────────────────────────────────────────────

void TestReadWallet::cleanupTestCase() {
    if (m_handle >= 0 && m_iface) {
        // deleteWallet closes all handles and removes the .kwl file.
        m_iface->call("deleteWallet", QString(WALLET_NAME));
        m_handle = -1;
        // Remove the salt file that deleteWallet does not clean up.
        QFile::remove(walletDir() + QLatin1String("/kw2kpass_test.salt"));
    }
}

// ── Tests – folder existence ──────────────────────────────────────────────────

void TestReadWallet::hasFolderPasswords() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.wallet_is_open());
    QVERIFY(wh.hasFolder(FOLDER_PASSWORDS));
}

void TestReadWallet::hasFolderFormData() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.wallet_is_open());
    QVERIFY(wh.hasFolder(FOLDER_FORM_DATA));
}

void TestReadWallet::hasFolderTestFolder() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.wallet_is_open());
    QVERIFY(wh.hasFolder(FOLDER_TEST));
}

// ── Tests – Password-type entries ─────────────────────────────────────────────

void TestReadWallet::hasEntryExample1() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QVERIFY(wh.hasEntry("example1"));
}

void TestReadWallet::passwordExample1IsEmpty() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QCOMPARE(wh.password("example1"), std::string(""));
}

void TestReadWallet::usernameExample1IsEmpty() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    // Password-type entries carry no username field.
    QCOMPARE(wh.username("example1"), std::string(""));
}

void TestReadWallet::hasEntryExample2() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QVERIFY(wh.hasEntry("example2"));
}

void TestReadWallet::passwordExample2IsEmpty() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QCOMPARE(wh.password("example2"), std::string(""));
}

// ── Tests – Map-type entry "test_map" ─────────────────────────────────────────

void TestReadWallet::hasEntryTestMap() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QVERIFY(wh.hasEntry("test_map"));
}

void TestReadWallet::passwordTestMap() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QCOMPARE(wh.password("test_map"), std::string("trivial_and_not_valid"));
}

void TestReadWallet::usernameTestMap() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QCOMPARE(wh.username("test_map"), std::string("not@existing"));
}

void TestReadWallet::hostnameTestMap() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    QCOMPARE(wh.hostname("test_map"), std::string("https://www.bahn.de/"));
}

// ── Tests – test_folder entries ───────────────────────────────────────────────

void TestReadWallet::hasEntryExample3() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_TEST));
    QVERIFY(wh.hasEntry("example3"));
}

void TestReadWallet::passwordExample3IsEmpty() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_TEST));
    QCOMPARE(wh.password("example3"), std::string(""));
}

// ── Tests – WalletIterator ────────────────────────────────────────────────────

void TestReadWallet::iteratorPasswordsFolder() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_PASSWORDS));
    WalletIterator it(wh);
    std::set<std::string> found;
    for (const std::string &name : it)
        found.insert(name);
    QVERIFY(found.count("example1") == 1);
    QVERIFY(found.count("example2") == 1);
    QVERIFY(found.count("test_map") == 1);
    QCOMPARE(static_cast<int>(found.size()), 3);
}

void TestReadWallet::iteratorTestFolder() {
    WalletHandle wh(WALLET_NAME);
    QVERIFY(wh.setFolder(FOLDER_TEST));
    WalletIterator it(wh);
    std::set<std::string> found;
    for (const std::string &name : it)
        found.insert(name);
    QVERIFY(found.count("example3") == 1);
    QCOMPARE(static_cast<int>(found.size()), 1);
}

#ifndef BUILDING_PYBIND_MODULE
QTEST_GUILESS_MAIN(TestReadWallet)
#endif
