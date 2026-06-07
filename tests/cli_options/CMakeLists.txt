

add_test(
    NAME libs_exist
    COMMAND ${PYTHON_SCRIPT_DIRECTORY}/run_libs_exist.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_property(
    TEST libs_exist
    PROPERTY ENVIRONMENT
    LIB_LOCATION=${CMAKE_BINARY_DIR}/src
)
