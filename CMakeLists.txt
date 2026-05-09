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

# This is a temporary setting.
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

add_subdirectory(readKwallet)
# temporarily disabled
# add_subdirectory(writePwsafe)
