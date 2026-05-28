######################################
# Find KDE include dirs and libs     #
######################################

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH
	${ECM_MODULE_PATH}
	${ECM_KDE_MODULE_DIR} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Solid Wallet)
######################################
# Adding Qt5 and KDE include dirs    #
######################################

if (Qt5_FOUND)
  message(STATUS "Adding include files and libraries for Qt5.")
  include_directories(${Qt5Core_INCLUDE_DIRS},
                      ${Qt5Gui_INCLUDE_DIRS},
                      ${QtWidgets_INCLUDE_DIRS})
  if (Qt5_POSITION_INDEPENDENT_CODE)
    message(STATUS
      "Adding ${Qt5Core_EXECUTABLE_COMPILE_FLAGS} to CXXFLAGS.")
    SET( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}" )
    SET( CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}" )
  endif (Qt5_POSITION_INDEPENDENT_CODE)
else (Qt5_FOUND)
  message(FATAL_ERROR "Qt5 was not found.")
endif (Qt5_FOUND)

if (KF5_FOUND)
  message(STATUS "Adding include files and libraries for KF5.")
  include_directories(${KF5_INCLUDE_INSTALL_DIR}/KWallet,
                      ${KF5_INCLUDE_INSTALL_DIR}/KCoreAddons)
else (KF5_FOUND)
  message(FATAL_ERROR "KF5 was not found.")
endif (KF5_FOUND)

######################################
# pybind11 settings                  #
######################################

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)

  if (HAS_CPP14_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif (HAS_CPP11_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    message(FATAL_ERROR "Unsupported compiler -- at least C++11 support is needed!")
  endif()

  # Default symbol visibility
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

  # Adding the -fexceptions flag (again)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

  # Get rid of -Wundef warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef")
endif()


######################################
# List of source and include files   #
######################################

set(READ_KW_SOURCES
    src/wallethandle.cpp
    src/readKwallet.cpp)

set(KW2PW_SOURCES
    src/wallethandle.cpp
    src/wrap_pybind_kw.cpp)

include_directories(include)

######################################
# Names of executable and library    #
######################################

add_executable( readKwallet
  ${READ_KW_SOURCES} )

# See https://pybind11.readthedocs.io/en/stable/compiling.html
# for details on pybind11_add_module.

pybind11_add_module( kwallet MODULE
  ${KW2PW_SOURCES} )

######################################
# Adding Qt5, Python and KDE libs    #
######################################

# In addition, the signature "PRIVATE"
# has to be added to
# target_link_libraries (kwallet ...)
# below (e.g. ${PYTHON_LIBRARIES}.

target_link_libraries (readKwallet
  	${Qt5Core_LIBRARIES}
  	${Qt5Gui_LIBRARIES}
  	${Qt5Widgets_LIBRARIES}
    ${Python3_LIBRARIES}
    )

target_link_libraries (kwallet PRIVATE
  	${Qt5Core_LIBRARIES}
  	${Qt5Gui_LIBRARIES}
  	${Qt5Widgets_LIBRARIES}
    ${Python3_LIBRARIES}
    )

target_link_libraries (readKwallet
	KF5::CoreAddons
	KF5::Wallet)
target_link_libraries (kwallet PRIVATE
	KF5::CoreAddons
	KF5::Wallet)

add_custom_command(TARGET kwallet POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:kwallet>
    "${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:kwallet>"
)
