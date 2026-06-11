/*
 * wrap_pybind.cpp
 *
 *      Author: Dr. Christoph Pospiech
 *   Copyright: Lenovo Deutschland GmbH, 2016, 2021, 2022
 *     Contact: cpospiech@lenovo.com
 *  Created on: Feb  8, 2016
 * Modified on: Jan  1, 2022
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

#include <Python.h>
#include <pybind11/pybind11.h>

#include <WalletHandle>

namespace py = pybind11;

PYBIND11_MODULE(kwallet, m) {
    m.doc() = "Wrapper module for KWallet";

    py::class_<WalletHandle>(m, "WalletHandle")
        .def(py::init<std::string>())
        .def("wallet_is_open", &WalletHandle::wallet_is_open)
        .def("wallet_name", &WalletHandle::wallet_name)
        .def("hasFolder", &WalletHandle::hasFolder)
        .def("setFolder", &WalletHandle::setFolder)
        .def("currentFolder", &WalletHandle::currentFolder)
        .def("password", &WalletHandle::password)
        .def("username", &WalletHandle::username)
        .def("hostname", &WalletHandle::hostname);

    py::class_<WalletIterator>(m, "WalletIterator")
        .def(py::init<WalletHandle &>())
        .def("__str__", &WalletIterator::__str__)
        .def(
            "__iter__", [](WalletIterator &s) { return py::make_iterator(s.begin(), s.end()); }, py::keep_alive<0, 1>()
            /* Essential: keep object alive while iterator exists */);
}
