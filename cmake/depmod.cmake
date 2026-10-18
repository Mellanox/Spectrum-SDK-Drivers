message(STATUS "Running depmod")

set(ENV{SDK_CMAKE_INSTALL} "1")
set(ENV{SDK_CMAKE_INSTALL_DEPMOD} "1")

execute_process(
    COMMAND ${POSTINST_SCRIPT_PATH}
    RESULT_VARIABLE ret
)

if (ret AND NOT ret EQUAL 0)
    message(FATAL_ERROR "Failed to run depmod")
else()
    message(STATUS "Done (depmod)")
endif()
