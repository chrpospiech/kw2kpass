#***********************************************************************
#
#        CMakeLists.txt for converter from Kwallet to KeePassXC
#
#        Created  on: May  09, 2026
#        Modified on:
#        Author: Dr. Christoph Pospiech
#
#***********************************************************************

cmake_minimum_required(VERSION 3.20.0)

project(kw2kpass)

# Cmake modules/macros are in a subdirectory to keep this file cleaner
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

enable_language(CXX)

###########################################
# compiler tests                          #
###########################################

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
add_subdirectory(writeKeePass)
