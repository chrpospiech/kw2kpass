/*
 * test_wallethandle.cpp
 *
 * Unit tests for WalletHandle and WalletIterator.
 *
 * All tests exercise the "wallet not open" code paths, which are
 * deterministic regardless of whether a KWallet daemon is running.
 * WalletHandle::wallet_is_open() returns false whenever
 * KWallet::Wallet::openWallet() cannot open the named wallet
 * (e.g. the wallet does not exist, or no daemon is available),
 * and every public method guards against this state.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// Include Qt headers before WalletHandle to avoid QT_NAMESPACE
// side-effects on the Qt moc-generated code.
#include <QTest>

#include <WalletHandle>

class TestWalletHandle : public QObject {
    Q_OBJECT

  private slots:
    // WalletHandle – basic state
    void walletIsClosedForNonexistentWallet();
    void walletNameIsPreserved();

    // WalletHandle – accessor guards when wallet is closed
    void passwordReturnsEmptyWhenClosed();
    void usernameReturnsEmptyWhenClosed();
    void hostnameReturnsEmptyWhenClosed();
    void hasFolderReturnsFalseWhenClosed();
    void setFolderReturnsFalseWhenClosed();
    void currentFolderReturnsEmptyWhenClosed();
    void hasEntryReturnsFalseWhenClosed();
    void entryListIsEmptyWhenClosed();

    // WalletIterator – behaviour on a closed wallet
    void walletIteratorIsAtEndForClosedWallet();
    void walletIteratorBeginEqualsEndForClosedWallet();
};

// ---------------------------------------------------------------------------
// WalletHandle – basic state
// ---------------------------------------------------------------------------

void TestWalletHandle::walletIsClosedForNonexistentWallet() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QVERIFY(!wh.wallet_is_open());
}

void TestWalletHandle::walletNameIsPreserved() {
    WalletHandle wh("my_test_wallet");
    QCOMPARE(wh.wallet_name(), std::string("my_test_wallet"));
}

// ---------------------------------------------------------------------------
// WalletHandle – accessor guards when wallet is closed
// ---------------------------------------------------------------------------

void TestWalletHandle::passwordReturnsEmptyWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QCOMPARE(wh.password("entry"), std::string(""));
}

void TestWalletHandle::usernameReturnsEmptyWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QCOMPARE(wh.username("entry"), std::string(""));
}

void TestWalletHandle::hostnameReturnsEmptyWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QCOMPARE(wh.hostname("entry"), std::string(""));
}

void TestWalletHandle::hasFolderReturnsFalseWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QVERIFY(!wh.hasFolder("Network"));
}

void TestWalletHandle::setFolderReturnsFalseWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QVERIFY(!wh.setFolder("Network"));
}

void TestWalletHandle::currentFolderReturnsEmptyWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QCOMPARE(wh.currentFolder(), std::string(""));
}

void TestWalletHandle::hasEntryReturnsFalseWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QVERIFY(!wh.hasEntry("some_key"));
}

void TestWalletHandle::entryListIsEmptyWhenClosed() {
    WalletHandle wh("__test_nonexistent_wallet__");
    QVERIFY(wh.entryList().isEmpty());
}

// ---------------------------------------------------------------------------
// WalletIterator – behaviour on a closed wallet
// ---------------------------------------------------------------------------

void TestWalletHandle::walletIteratorIsAtEndForClosedWallet() {
    WalletHandle wh("__test_nonexistent_wallet__");
    WalletIterator it(wh);
    QVERIFY(it.isLast());
}

void TestWalletHandle::walletIteratorBeginEqualsEndForClosedWallet() {
    WalletHandle wh("__test_nonexistent_wallet__");
    WalletIterator it(wh);
    QCOMPARE(it.begin(), it.end());
}

QTEST_GUILESS_MAIN(TestWalletHandle)
#include "test_closed_wallet.moc"
