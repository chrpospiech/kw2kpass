# Build a pybind11 module from libkeepass sources.
# Assumes Python3 and pybind11 are already found in the parent CMake project.

######################################
# List of source files               #
######################################

file(GLOB KEEPASS_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/libkeepass/src/*.cc"
)

list(APPEND KEEPASS_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/wrap_pybind_kp.cpp"
)

######################################
# Name of the library                #
######################################

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

pybind11_add_module(keepass MODULE
  ${KEEPASS_SOURCES}
)

######################################
# include directories                #
######################################

target_include_directories(keepass PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/libkeepass/src
)

target_link_libraries(keepass PRIVATE OpenSSL::Crypto)

add_custom_command(TARGET keepass POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:keepass>
    "${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:keepass>"
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(keepass PRIVATE -Wno-deprecated-declarations)
endif()
