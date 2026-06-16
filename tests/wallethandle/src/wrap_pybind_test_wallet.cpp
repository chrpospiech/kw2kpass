/*
 * wrap_pybind_test_wallet.cpp
 *
 * pybind11 binding for the TestReadWallet fixture class.
 *
 * Exposes TestReadWallet.initTestCase() and
 * TestReadWallet.cleanupTestCase() so that Python's unittest framework
 * can call them from setUp() and tearDown() respectively.
 *
 * A QCoreApplication is created once at module import time when no
 * QCoreApplication instance already exists.  This is required because
 * QDBusConnection, QProcess, and QTest::qWait() all depend on the Qt
 * event-loop infrastructure.
 *
 * Typical Python usage:
 *
 *   import unittest
 *   import test_wallet
 *
 *   class MyWalletTest(unittest.TestCase):
 *       def setUp(self):
 *           self._fixture = test_wallet.TestReadWallet()
 *           self._fixture.initTestCase()
 *
 *       def tearDown(self):
 *           self._fixture.cleanupTestCase()
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

#include <pybind11/pybind11.h>

#include <QCoreApplication>

#include "test_read_wallet.hpp"

namespace py = pybind11;

// QCoreApplication instance owned by this module.
// Allocated once on first import; never deleted (process lifetime).
static QCoreApplication *s_app = nullptr;

static void ensure_qapp() {
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char name[] = "test_wallet_py";
        static char *argv[] = {name, nullptr};
        s_app = new QCoreApplication(argc, argv);
    }
}

PYBIND11_MODULE(test_wallet, m) {
    m.doc() = "Test fixture module: exposes TestReadWallet.initTestCase() "
              "and TestReadWallet.cleanupTestCase() for use as Python "
              "unittest setUp / tearDown.";

    ensure_qapp();

    // QObject subclasses are non-copyable; pybind11 uses unique_ptr by default
    // when the copy constructor is deleted (which Q_DISABLE_COPY enforces).
    py::class_<TestReadWallet>(m, "TestReadWallet")
        .def(py::init<>())
        .def("initTestCase", &TestReadWallet::initTestCase,
             "Open the kw2kpass_test wallet via kwalletd5 (setUp).\n\n"
             "Exits the process with code 222 when prerequisites are not met "
             "(kwalletd5 absent, wallet archive missing, etc.) so that Python "
             "unittest skips the test suite gracefully when called via CTest.")
        .def("cleanupTestCase", &TestReadWallet::cleanupTestCase,
             "Delete the test wallet from kwalletd5 and remove imported files "
             "(tearDown).");
}
