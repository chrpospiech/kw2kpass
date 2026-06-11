/*
 * readKwallet.cpp
 *
 *      Author: Dr. Christoph Pospiech
 *   Copyright: Lenovo Deutschland GmbH, 2016, 2021
 *     Contact: cpospiech@lenovo.com
 *  Created on: Feb  3, 2016
 * Modified on: Dec 25, 2021
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

#include <cstdlib>
#include <iostream>
#include <string>

extern "C" {
#include <getopt.h>
}

#include <WalletHandle>

int main(int argc, char *argv[]) {

    int verbose = 0;
    int c;
    std::string w_name;
    std::string f_name;
    std::string k_name;

    while ((c = getopt(argc, argv, "vf:w:")) != -1) {
        switch (c) {
        case 'v':
            verbose = 1;
            break;
        case 'w':
            w_name = std::string(optarg);
            break;
        case 'f':
            f_name = std::string(optarg);
            break;
        default:
            std::cout << "Unknown option." << std::endl;
            std::abort();
        }
    }

    if (verbose) {
        std::cout << "Starting readKwallet" << std::endl;
        if (argc > 0) {
            for (int i = 0; i < argc; i++) {
                std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
            }
        }
        std::cout << "Got name of wallet: " << w_name << "." << std::endl;
    }

    WalletHandle wh(w_name);

    if (wh.wallet_is_open()) {
        if (verbose) {
            std::cout << "Got wallet " << wh.wallet_name() << "." << std::endl;
        }
    } else {
        std::cerr << "Could not open wallet " << wh.wallet_name() << ". Exiting." << std::endl;
        return 1;
    }
    if (wh.hasFolder(f_name)) {
        if (verbose) {
            std::cout << "Folder " << f_name << " was found." << std::endl;
        }
    } else {
        std::cerr << "Could not find folder " << f_name << ". Exiting." << std::endl;
        return 1;
    }
    if (wh.setFolder(f_name)) {
        if (verbose) {
            std::cout << "Set folder to " << f_name << "." << std::endl;
        }
    } else {
        std::cerr << "Could not switch to folder " << f_name << " . Exiting." << std::endl;
        return 1;
    }
    std::cout << "\nList of entries in folder " << f_name << "\n" << std::endl;
    int i = 0;
    for (WalletIterator wi(wh); !wi.isLast(); ++wi) {
        std::cout << (k_name = wi.name()) << "\tpassword\t" << wh.password(k_name) << std::endl;
        i++;
    }
    std::cout << "\nTotal number is " << i << std::endl;

    return 0;
}
