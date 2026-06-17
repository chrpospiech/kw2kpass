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
#***********************************************************************
#
#        CMakeLists.txt for converter from Kwallet to KeePassXC
#
#        Created  on: May  09, 2026
#        Modified on: June 17, 2026
#        Author: Dr. Christoph Pospiech
#
#***********************************************************************

cmake_minimum_required(VERSION 3.20.0)

project(kw2kpass)

######################################
# Cmake modules/macros are in a      #
# subdirectory to keep this file     #
# cleaner.                           #
######################################

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

###########################################
# compiler tests                          #
###########################################

enable_language(CXX)
include(CheckCXXCompilerFlag)

######################################
# Find Python3 and pybind11          #
# include dirs and libs              #
######################################

# To escape from version hell ( [py]Qt 5.9.2 in
# python/3.8.11 versus  Qt > 5.12 as required
# by KDE KWallet) switch to Kubuntu Python3
# (version 3.8.10).

find_package (Python3  REQUIRED
	COMPONENTS Interpreter Development)
list(APPEND CMAKE_PREFIX_PATH
    "${CMAKE_SOURCE_DIR}/.venv/lib64/python3.12/site-packages/pybind11"
)
find_package (pybind11 REQUIRED)

# Include path for Python header files
include_directories(${Python3_INCLUDE_DIRS})

# Include path for pybind11
include_directories( ${pybind11_INCLUDE_DIRS} )

# This is where the pybind modules should go
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

add_subdirectory(readKwallet)

######################################
# enable testing functionality       #
######################################

include(CTest)

######################################
# Include the tests subdirectory     #
# to set up unit tests               #
######################################

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
