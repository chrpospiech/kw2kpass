

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
