
set (TARGET_NAME memtrack)
set (MODULE_NAME ${TARGET_NAME}.ko)
set (TARGET_NAME_CLEAN ${TARGET_NAME}_clean)
file(RELATIVE_PATH SXD_MODULE_PATH ${SXD_KERNEL_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

set (MODULE_CMD ${MODULE_CMD_COMMON} M=${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target (
    ${TARGET_NAME} ALL
    COMMAND ${MODULE_CMD} modules
    COMMENT "Building ${MODULE_NAME}"
    VERBATIM
)

# Register this module target for compile_commands generation (only if enabled)
if (KERNEL_COMPILE_COMMANDS)
	set_property(GLOBAL APPEND PROPERTY KERNEL_MODULE_TARGETS "${TARGET_NAME}")
endif()

add_custom_target (${TARGET_NAME_CLEAN}
    COMMAND ${MODULE_CMD} clean
    COMMENT "Cleaning ${MODULE_NAME}"
)

install (
    PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}
    DESTINATION ${MODULES_PATH}/update/kernel/${SXD_MODULE_PATH}
)

