
set (TARGET_NAME sx_core)
set (MODULE_NAME ${TARGET_NAME}.ko)
set (TARGET_NAME_CLEAN ${TARGET_NAME}_clean)
file(RELATIVE_PATH SXD_MODULE_PATH ${SXD_KERNEL_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

set (MODULE_CMD ${MODULE_CMD_COMMON} EXTRA_SYMVERS=${MEMTRACK_EXTRA_SYMVERS_VAR} M=${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Kbuild
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/Makefile ${CMAKE_CURRENT_BINARY_DIR}/Kbuild
)

add_custom_target (
    ${TARGET_NAME} ALL
    COMMAND ${MODULE_CMD} modules
    COMMENT "Building ${MODULE_NAME}"
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Kbuild
    VERBATIM
)

# Register this module target for compile_commands generation (only if enabled)
if (KERNEL_COMPILE_COMMANDS)
	set_property(GLOBAL APPEND PROPERTY KERNEL_MODULE_TARGETS "${TARGET_NAME}")
endif()

if (MEMTRACK)
    add_dependencies(${TARGET_NAME} memtrack)
endif()

add_custom_target (${TARGET_NAME_CLEAN}
    COMMAND ${MODULE_CMD} clean
    COMMENT "Cleaning ${MODULE_NAME}"
)

install (
    PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}
    DESTINATION ${MODULES_PATH}/update/kernel/${SXD_MODULE_PATH}
)

add_subdirectory(debug)
