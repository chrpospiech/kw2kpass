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

add_test(
    NAME translate_entry
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_translate_entry.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_property(
    TEST translate_entry
    PROPERTY ENVIRONMENT
    PYTHONPATH=${CMAKE_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}:$ENV{PYTHONPATH}
)
