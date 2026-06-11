/*
 * test_read_wallet.cpp
 *
 * Integration tests for WalletHandle against a live kwalletd5 daemon.
 *
 * Wallet setup (initTestCase):
 *
 *   IMPORT PATH  – preferred, provides a deterministic fixture
 *     tests/test_data/kw2kpass_test is a tar archive produced by
 *     kwalletmanager5's "Export Wallet" function.  It contains:
 *       kw2kpass_test.kwl   – Blowfish-encrypted wallet file
 *       kw2kpass_test.salt  – PBKDF2 salt used by kwalletd
 *     The wallet's password is "Only4Test".
 *     initTestCase() extracts both files into the kwalletd wallet
 *     directory so the daemon can open the wallet by name.
 *
 *   SCRATCH PATH  – automatic fallback when the tar archive is absent
 *     The wallet "kw2kpass_test" is created from scratch via kwalletd's
 *     pamOpen DBus method and populated with the entries documented in
 *     tests/test_data/kw2kpass_test.xml.
 *
 *   Either way the wallet is opened non-interactively via kwalletd5's
 *   pamOpen DBus method (SHA-512 of "Only4Test" as the key material,
 *   matching what pam_kwallet5 sends to the daemon) without any GUI
 *   dialog.
 *
 *   QSKIP is used throughout initTestCase() so all test methods are
 *   reported as "Skipped" – not "Failed" – when:
 *     - kwalletd5 is not registered on the session bus,
 *     - tar extraction of the wallet archive fails,
 *     - pamOpen does not open the wallet within 3 s (daemon policy
 *       restriction or mismatched password on the imported file), or
 *     - any other setup step cannot complete.
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

#include <set>
#include <string>

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

#include <WalletHandle>

// ── Constants ────────────────────────────────────────────────────────────────

namespace {
const char WALLET_NAME[] = "kw2kpass_test";
const char WALLET_APPID[] = "kw2kpass_unit_test";
const char WALLET_PASSWORD[] = "Only4Test";
const char FOLDER_PASSWORDS[] = "Passwords";
const char FOLDER_FORM_DATA[] = "Form Data";
const char FOLDER_TEST[] = "test_folder";
} // namespace

// ── Test class ───────────────────────────────────────────────────────────────

class TestReadWallet : public QObject {
    Q_OBJECT

  private:
    int m_handle = -1;
    QDBusInterface *m_iface = nullptr;
    bool m_imported = false; // true when .kwl/.salt were extracted

    // SHA-512 of the wallet password.
    // pam_kwallet5 hashes the login password with SHA-512 and passes the
    // result to kwalletd's pamOpen method as the key material.
    static QByteArray passwordHash() { return QCryptographicHash::hash(WALLET_PASSWORD, QCryptographicHash::Sha512); }

    // Directory where kwalletd stores .kwl files on this system.
    static QString walletDir() {
        return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kwalletd");
    }

    // Path to the test-data tar archive in the source tree.
    // KWALLET_TEST_DATA_DIR is set by CMake to tests/test_data.
    static QString testDataArchive() { return QLatin1String(KWALLET_TEST_DATA_DIR) + QLatin1String("/kw2kpass_test"); }

    // Encode a QMap<QString,QString> as the QDataStream byte array that
    // kwalletd's writeMap DBus method expects.
    static QByteArray encodeMap(const QMap<QString, QString> &m) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << m;
        return buf;
    }

    // Create folders and entries matching tests/test_data/kw2kpass_test.xml.
    void setupTestFixture();

    // Remove the .kwl and .salt files placed in the wallet directory.
    void removeImportedFiles() {
        QFile::remove(walletDir() + QLatin1String("/kw2kpass_test.kwl"));
        QFile::remove(walletDir() + QLatin1String("/kw2kpass_test.salt"));
    }

  private slots:
    void initTestCase();
    void cleanupTestCase();

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

// ── Setup helpers ─────────────────────────────────────────────────────────────

void TestReadWallet::setupTestFixture() {
    auto skipOnDbusError = [](const QDBusMessage &reply, const char *method) {
        if (reply.type() == QDBusMessage::ErrorMessage) {
            const QByteArray msg = QStringLiteral("kwalletd5 %1 failed while populating scratch fixture: %2")
                                       .arg(QString::fromLatin1(method), reply.errorMessage())
                                       .toUtf8();
            QSKIP(msg.constData());
        }
    };

    // Folder: Form Data – empty.
    skipOnDbusError(m_iface->call("createFolder", m_handle, QString(FOLDER_FORM_DATA), QString(WALLET_APPID)),
                    "createFolder");

    // Folder: Passwords – two Password-type entries and one Map-type entry.
    skipOnDbusError(m_iface->call("createFolder", m_handle, QString(FOLDER_PASSWORDS), QString(WALLET_APPID)),
                    "createFolder");
    skipOnDbusError(m_iface->call("writePassword", m_handle, QString(FOLDER_PASSWORDS), QString("example1"),
                                  QString(""), QString(WALLET_APPID)),
                    "writePassword");
    skipOnDbusError(m_iface->call("writePassword", m_handle, QString(FOLDER_PASSWORDS), QString("example2"),
                                  QString(""), QString(WALLET_APPID)),
                    "writePassword");

    QMap<QString, QString> testMap;
    testMap[QStringLiteral("hostname")] = QStringLiteral("https://www.bahn.de/");
    testMap[QStringLiteral("password")] = QStringLiteral("trivial_and_not_valid");
    testMap[QStringLiteral("username")] = QStringLiteral("not@existing");
    skipOnDbusError(m_iface->call("writeMap", m_handle, QString(FOLDER_PASSWORDS), QString("test_map"),
                                  encodeMap(testMap), QString(WALLET_APPID)),
                    "writeMap");

    // Folder: test_folder – one Password-type entry.
    skipOnDbusError(m_iface->call("createFolder", m_handle, QString(FOLDER_TEST), QString(WALLET_APPID)),
                    "createFolder");
    skipOnDbusError(m_iface->call("writePassword", m_handle, QString(FOLDER_TEST), QString("example3"), QString(""),
                                  QString(WALLET_APPID)),
                    "writePassword");

    skipOnDbusError(m_iface->call("sync", m_handle, QString(WALLET_APPID)), "sync");
}

// ── initTestCase ─────────────────────────────────────────────────────────────

void TestReadWallet::initTestCase() {
    // ── Step 1: verify kwalletd5 is reachable ────────────────────────────────
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || !bus.interface()->isServiceRegistered("org.kde.kwalletd5")) {
        QSKIP("kwalletd5 is not registered on the session bus");
    }

    m_iface = new QDBusInterface("org.kde.kwalletd5", "/modules/kwalletd5", "org.kde.KWallet", bus, this);
    if (!m_iface->isValid()) {
        QSKIP("DBus interface to kwalletd5 is not valid");
    }

    // ── Step 2: import wallet archive if present ─────────────────────────────
    // The archive tests/test_data/kw2kpass_test is produced by kwalletmanager5's
    // "Export Wallet" and contains kw2kpass_test.kwl and kw2kpass_test.salt.
    // Extract both files directly into the kwalletd wallet directory so the
    // daemon can find the wallet by name.

    // Safety: never overwrite or delete a real user wallet that happens to have
    // the same name as this test fixture.
    if (QFile::exists(walletDir() + QLatin1String("/kw2kpass_test.kwl")) ||
        QFile::exists(walletDir() + QLatin1String("/kw2kpass_test.salt"))) {
        QSKIP("A wallet named kw2kpass_test already exists in the kwalletd directory; refusing to overwrite/delete it");
    }

    const QString archive = testDataArchive();
    if (QFile::exists(archive)) {
        QDir().mkpath(walletDir());
        QProcess tar;
        tar.start("tar", {"-xf", archive, "-C", walletDir(), "kw2kpass_test.kwl", "kw2kpass_test.salt"});
        if (!tar.waitForFinished(10000) || tar.exitCode() != 0) {
            QSKIP("Failed to extract the test wallet archive into the kwalletd directory");
        }
        m_imported = true;
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
        if (m_imported)
            removeImportedFiles();
        QSKIP("pamOpen did not open the test wallet within 3 s "
              "(daemon policy restriction or wrong password on the imported file)");
    }

    // ── Step 4: acquire a per-session DBus handle ────────────────────────────
    // The wallet is already open in the daemon, so open() returns immediately
    // without showing any dialog.
    QDBusReply<int> openReply = m_iface->call("open", QString(WALLET_NAME), qlonglong(0), QString(WALLET_APPID));
    if (!openReply.isValid() || openReply.value() < 0) {
        if (m_imported)
            removeImportedFiles();
        QSKIP("Could not obtain a kwalletd handle for the test wallet");
    }
    m_handle = openReply.value();

    // ── Step 5: populate test data (scratch path only) ───────────────────────
    // When importing a pre-built archive the entries are already present.
    if (!m_imported) {
        setupTestFixture();
    }
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

QTEST_GUILESS_MAIN(TestReadWallet)
#include "test_read_wallet.moc"
