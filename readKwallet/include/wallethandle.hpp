/*
 * wallethandle.hpp
 *
 *      Author: Dr. Christoph Pospiech
 *   Copyright: Lenovo Deutschland GmbH, 2016, 2021
 *     Contact: cpospiech@lenovo.com
 *  Created on: Feb  6, 2016
 * Modified on: Dec 25, 2021
 *
 * Wrapper class around KWallet::Wallet.
 * The constructor attempts to open the wallet given
 * by name. Access functions provide Wallet contents.
 * String values are returned as std::String.
 *
 * The destructor closes the wallet.
 *
 */

#ifndef WALLETWRAPPER_READKWALLET_INCLUDE_WALLETHANDLE_HPP_
#define WALLETWRAPPER_READKWALLET_INCLUDE_WALLETHANDLE_HPP_

#include <stdbool.h>
#include <string>

#include <QString>
#include <QStringList>
#include <kwallet.h>

#define QT_NAMESPACE readKwallet

#include <qglobal.h>

QT_BEGIN_NAMESPACE

class WalletHandle {
  private:
    KWallet::Wallet *wallet;
    // intermediate buffers to get rid of
    // erroneous move constructors.
    std::string w_name;

  public:
    WalletHandle(std::string wallet_name) : w_name(wallet_name) {
        const QString &QT_name = QString(w_name.c_str());
        wallet = KWallet::Wallet::openWallet(QT_name, 0);
    }
    ~WalletHandle() { delete wallet; }
    bool wallet_is_open(void) { return (bool)wallet; }
    std::string wallet_name(void) { return w_name; }
    bool hasFolder(std::string fname) {
        const QString &QT_name = QString(fname.c_str());
        return (wallet_is_open() ? wallet->hasFolder(QT_name) : false);
    }
    bool setFolder(std::string fname) {
        const QString &QT_name = QString(fname.c_str());
        return (wallet_is_open() ? wallet->setFolder(QT_name) : false);
    }
    std::string currentFolder(void) { return (wallet_is_open() ? wallet->currentFolder().toStdString() : ""); }
    bool hasEntry(std::string kname) {
        const QString &tmp_str = QString(kname.c_str());
        return (wallet_is_open() ? wallet->hasEntry(tmp_str) : false);
    }
    QStringList entryList(void) { return (wallet_is_open() ? wallet->entryList() : QStringList()); }
    // return the password for the current entry.
    std::string password(std::string entry);
    // return the user name for the current entry.
    std::string username(std::string entry);
    // return the user name for the current entry.
    std::string hostname(std::string entry);
};

/*
 * WalletIterator is essentially a wrapper around a
 * list of pointers to std::string. The strings are
 * initiated by the list of entries of a KWallet folder.
 * This is an STL-type iterator. Only this type of iterators
 * can be exposed to Python via pybind11.
 * For a distinction of Java-type vs. STL-type iterators
 * see https://doc.qt.io/qt-5/qlistiterator.html
 */
class WalletIterator {
  private:
    std::string *entry_list;
    std::string *current;
    std::string *last;

  public:
    WalletIterator(WalletHandle &wh);
    ~WalletIterator() { delete[] entry_list; }
    WalletIterator &__iter__() { return *this; }
    std::string *begin(void) { return entry_list; }
    std::string *end(void) { return last; }
    WalletIterator &next(void) {
        ++current;
        return *this;
    }
    WalletIterator &operator++() {
        ++current;
        return *this;
    }
    bool isLast(void) { return (!current || (current == last)); }
    std::string name(void) { return *current; }
    std::string __str__(void) { return *current; }
};

QT_END_NAMESPACE

#endif /* WALLETWRAPPER_READKWALLET_INCLUDE_WALLETHANDLE_HPP_ */
