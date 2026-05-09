/*
 * wallethandle.cpp
 *
 *      Author: Dr. Christoph Pospiech
 *   Copyright: Lenovo Deutschland GmbH, 2016, 2021
 *     Contact: cpospiech@lenovo.com
 *  Created on: Feb  6, 2016
 * Modified on: Dec 25, 2021
 *
 * Wrapper class around KWallet::Wallet.
 * The constructor attempts to the wallet given
 * by name. Access functions provide Wallet contents.
 * String values are returned as std::String.
 *
 * The destructor closes the wallet.
 *
 */

#include <iostream>
#include <string>

#include <QString>
#include <QStringList>
#include <QListIterator>
#include <QMap>

#include <WalletHandle>

QT_BEGIN_NAMESPACE

std::string WalletHandle::password(std::string entry) {
	const QString& key = QString(entry.c_str());
	QString passwd;

	if (!wallet_is_open()) return("");

	switch (wallet->entryType(key)) {
		case (KWallet::Wallet::EntryType::Password) : {
			int ret = wallet->readPassword(key, passwd);
			if (ret != 0) {
				std::cerr << "Could not read password for key "
				<< entry
				<< " in folder "
				<< currentFolder()
				<< ". Exiting."
				<< std::endl;
				delete wallet;
				exit(1);
			}
			return(passwd.toStdString());
			break;
		}
		case (KWallet::Wallet::EntryType::Map) : {
			QMap<QString,QString> wmap;
			int ret = wallet->readMap(key,wmap);
			if (ret != 0) {
				std::cerr << "Could not read map for key "
				<< entry
				<< " in folder "
				<< currentFolder()
				<< ". Exiting."
				<< std::endl;
				delete wallet;
				exit(1);
			}
			passwd = wmap.value(
					QString("password"),
					QString("n/a"));
			return(passwd.toStdString());
			break;
		}
		default:
		std::cerr << "case not implemented yet." << std::endl;
		return("n/a");
	}
}

std::string WalletHandle::username(std::string entry) {
	const QString& key = QString(entry.c_str());

	if (!wallet_is_open()) return("");

	switch (wallet->entryType(key)) {
		case (KWallet::Wallet::EntryType::Password) : {
			return "";
			break;
		}
		case (KWallet::Wallet::EntryType::Map) : {
			QMap<QString,QString> wmap;
			int ret = wallet->readMap(key,wmap);
			if (ret != 0) {
				std::cerr << "Could not read map for key "
				<< entry
				<< " in folder "
				<< currentFolder()
				<< ". Exiting."
				<< std::endl;
				delete wallet;
				exit(1);
			}
			QString uname = wmap.value(
					QString("username"),
					QString(""));
			return(uname.toStdString());
			break;
		}
		default:
		std::cerr << "case not implemented yet." << std::endl;
		return("n/a");
	}
}

std::string WalletHandle::hostname(std::string entry) {
	const QString& key = QString(entry.c_str());

	if (!wallet_is_open()) return("");

	switch (wallet->entryType(key)) {
		case (KWallet::Wallet::EntryType::Password) : {
			return "";
			break;
		}
		case (KWallet::Wallet::EntryType::Map) : {
			QMap<QString,QString> wmap;
			int ret = wallet->readMap(key,wmap);
			if (ret != 0) {
				std::cerr << "Could not read map for key "
				<< entry
				<< " in folder "
				<< currentFolder()
				<< ". Exiting."
				<< std::endl;
				delete wallet;
				exit(1);
			}
			QString hname = wmap.value(
					QString("hostname"),
					QString(""));
			return(hname.toStdString());
			break;
		}
		default:
		std::cerr << "case not implemented yet." << std::endl;
		return("n/a");
	}
}

WalletIterator::WalletIterator(WalletHandle& wh) :
		entry_list(NULL),
		current(NULL),
		last(NULL) {
	if ( wh.currentFolder().length() == 0 ) return;
	const QStringList& entries = wh.entryList();
	int list_size = entries.size();
	entry_list = new std::string[list_size];
	for (int i = 0; i < list_size; i++) {
		entry_list[i] = entries[i].toStdString();
	}
	current = entry_list;
	last = entry_list + list_size;
}

QT_END_NAMESPACE
