# Build a pybind11 module from libkeepass sources.
# Assumes Python3 and pybind11 are already found in the parent CMake project.

######################################
# List of source files               #
######################################

file(GLOB KEEPASS_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/libkeepass/src/*.cc"
)

######################################
# Name of the library                #
######################################

pybind11_add_module(keepass MODULE
  ${KEEPASS_SOURCES}
)

######################################
# include directories                #
######################################

target_include_directories(keepass PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/libkeepass/src
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(keepass PRIVATE -Wno-deprecated-declarations)
endif()
